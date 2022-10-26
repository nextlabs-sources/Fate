/*
 * Created on Feb 4, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice.impl.jceks;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.InvalidParameterException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableEntryException;
import java.security.KeyStore.PasswordProtection;
import java.security.cert.CertificateException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;

import javax.crypto.KeyGenerator;
import javax.crypto.spec.SecretKeySpec;
import javax.management.openmbean.InvalidKeyException;
import javax.security.auth.DestroyFailedException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.nextlabs.service.keyservice.IKey;
import com.nextlabs.service.keyservice.IKeyId;
import com.nextlabs.service.keyservice.IKeyRing;
import com.nextlabs.service.keyservice.IPassword;
import com.nextlabs.service.keyservice.KeyServiceException;
import com.nextlabs.service.keyservice.impl.CodecHelper;
import com.nextlabs.service.keyservice.impl.Key;
import com.nextlabs.service.keyservice.impl.KeyId;


/**
 * @author  hchan
 * @version  $Id$
 */

class KeyRingJCEKSImpl implements IKeyRing {
    private static final Log LOG = LogFactory.getLog(KeyRingJCEKSImpl.class);
    
    //have to pick the one that is not the encode table
    private static final char TOKEN = ' ';  
    
    //have to pick characters that is not from the encode table
    private static final String INTERNAL_PROPERTY_PREFIX = "X";
    private static final String NAME_PROPERTY = INTERNAL_PROPERTY_PREFIX + "name";
    private static final String LAST_SERVER_UPDATE_PROPERTY = INTERNAL_PROPERTY_PREFIX + "lastserverupdate";
    private static final String SERVER_MANAGED_PROPERTY = INTERNAL_PROPERTY_PREFIX + "servermanaged";
    
    private static final String CLOSED_MESSAGE = "The keyRing is already closed.";
    
    private final Map<Integer, KeyGenerator> keyGeneratorCache = new HashMap<Integer, KeyGenerator>(1);
    
    private final File file;
    private final File backupFile;
    
    /**
     * @uml.property  name="name"
     */
    private final String name;
    private long lastServerUpdate = IKeyRing.NEVER_UPDATED_BY_SERVER;
    private boolean serverManaged = false;
    private final KeyStore keyStore;
    private final PasswordProtection password;
    
    private boolean isClosed = false;
    
    KeyRingJCEKSImpl(String name, KeyStore keyStore, File file, IPassword password, boolean newRing) throws KeyServiceException {
        super();
        this.name = name;
        this.keyStore = keyStore;
        this.file = file;
        this.backupFile = new File(file.getParentFile(), file.getName() + ".bak");
        this.password = new PasswordProtection(password.getChars());
        
        if(newRing){
            try {
                KeyStore.Entry entry = new KeyStore.SecretKeyEntry(
                        new SecretKeySpec(CodecHelper.toBytes(name), "RAW"));
                keyStore.setEntry(NAME_PROPERTY, entry, this.password);
            } catch (KeyStoreException e) {
                throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.OTHER);
            }
            flush();
        } else {
            byte[] encodedName = getKey(NAME_PROPERTY);
            String storedName = CodecHelper.toString(encodedName);
            if (storedName == null || !name.equals(storedName)) {
                throw new KeyServiceException(
                        "The keyring's name doesn't match. The request name is '" + name
                                + "'. The actual name is '" + storedName + "'."
                        , KeyServiceException.KEY_RING | KeyServiceException.OTHER
                );
            }

            byte[] encodedServerUpdate = getKey(LAST_SERVER_UPDATE_PROPERTY);
            String storedUpdateTime = CodecHelper.toString(encodedServerUpdate);
            try {
                lastServerUpdate = Long.parseLong(storedUpdateTime);
            } catch (NumberFormatException ex) {
            }

            byte[] encodedServerManaged = getKey(SERVER_MANAGED_PROPERTY);
            serverManaged = Boolean.valueOf(CodecHelper.toString(encodedServerManaged));
        }
    }
    
    protected String convertToAlias(byte[] id, long timestamp){
        return CodecHelper.base16Encode(id) + TOKEN + Long.toHexString(timestamp);
    }
    
    protected String convertToAlias(IKeyId keyId){
        return convertToAlias(keyId.getId(), keyId.getCreationTimeStamp());
    }
    
    protected KeyId convertToKeyId(String alias) throws InvalidKeyException {
        alias = alias.toUpperCase();
        int sepIndex = alias.lastIndexOf(TOKEN);
        if(sepIndex == -1){
            throw new InvalidKeyException("invalid alias format, missing token. " + alias);
        }
        
        
        String name = alias.substring(0, sepIndex);
        
        byte[] original;
        try {
            original = CodecHelper.base16Decode(name);
        } catch (IllegalArgumentException e) {
            throw new InvalidKeyException("invalid alias format, '" + alias + "'. " + e.getMessage());
        }
        
        long date = Long.parseLong(alias.substring(sepIndex + 1), 16);
        return new KeyId(original, date);
    }
    
    private void checkClosed() throws KeyServiceException {
        if (isClosed) {
            throw new KeyServiceException(CLOSED_MESSAGE,
                    KeyServiceException.KEY_RING | KeyServiceException.INVALID_STATE);
        }
    }

    public void deleteKey(IKeyId keyId) throws KeyServiceException {
        checkClosed();
        
        String alias = convertToAlias(keyId);
        try {
            keyStore.deleteEntry(alias);
        } catch (KeyStoreException e) {
            throw new KeyServiceException(e,
                    KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        }
        
        flush();
    }
    
    protected byte[] getKey(String alias) throws KeyServiceException{
        checkClosed();
        LOG.debug(">getting key: " + alias);
        KeyStore.SecretKeyEntry entry;
        try {
            entry = (KeyStore.SecretKeyEntry)keyStore.getEntry(alias,  password);
        } catch (NoSuchAlgorithmException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        } catch (UnrecoverableEntryException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        } catch (KeyStoreException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        }
        if(entry != null){
            return entry.getSecretKey().getEncoded();
        }
        
        return null;
    }
    
    protected IKey getKey(String alias, IKeyId keyId) throws KeyServiceException {
        byte[] value = getKey(alias);
        if(value != null){
            return new Key(keyId, value);
        }
        return null;
    }

    public IKey getKey(IKeyId keyId) throws KeyServiceException {
        String alias = convertToAlias(keyId);
        return getKey(alias, keyId);
    }

    
    public Collection<IKey> getKeys() throws KeyServiceException {
        checkClosed();
        
        Collection<IKey> keys = new LinkedList<IKey>();
        try {
            Enumeration<String> aliases = keyStore.aliases();
            while (aliases.hasMoreElements()) {
                String alias = aliases.nextElement();
                if (alias.toUpperCase().startsWith(INTERNAL_PROPERTY_PREFIX)) {
                    continue;
                }
                keys.add(getKey(alias, convertToKeyId(alias)));
            }
        } catch (KeyStoreException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        }
        return keys;
    }

    public String getName() {
        return name;
    }
    
    public void setKey(final IKey key) throws KeyServiceException {
        if(key.getCreationTimeStamp() == IKeyId.NO_CREATION_TIMESTAMP){
            throw new KeyServiceException("CreationTimeStamp is not set.",
                    KeyServiceException.KEY | KeyServiceException.VALUE_NOT_SET);
        }
        setKey(key, true);
    }
    
    protected javax.crypto.SecretKey convertToSecretKey(IKey key){
        return new SecretKeySpec(key.getEncoded(), "Nextlabs" + key.getStructureVersion());
    }
    
    protected void setKey(final IKey key, boolean flush) throws KeyServiceException {
        checkClosed();
        
        String alias = convertToAlias(key);
        
        LOG.debug("<setting key: " + alias);
        
        javax.crypto.SecretKey mySecretKey = convertToSecretKey(key);

        try {
            keyStore.setEntry(alias, new KeyStore.SecretKeyEntry(mySecretKey), password);
        } catch (KeyStoreException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        }
        
        if(flush){
            flush();
        }
    }
    
    protected synchronized void flush() throws KeyServiceException{
        checkClosed();
        
        if (file.exists() && !file.renameTo(backupFile)) {
            throw new KeyServiceException("Can't make a backup of the file '" + file.getAbsolutePath() + "'",
                    KeyServiceException.KEY_RING | KeyServiceException.OTHER);
        }
        
        LOG.debug("flush");

        FileOutputStream fos;
        try {
            fos = new FileOutputStream(file);
        } catch (FileNotFoundException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.DOES_NOT_EXIST);
        }
        
        boolean isSuccess = false;
        boolean rollback = true;
        try {
            keyStore.store(fos, password.getPassword());
            rollback = false;
        } catch (KeyStoreException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        } catch (NoSuchAlgorithmException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        } catch (CertificateException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        } catch (IOException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
        } finally {
            try {
                fos.close();
            } catch (IOException e) {
                if (isSuccess) {
                    throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
                }
            }
            if (rollback) {
                file.delete();
                backupFile.renameTo(file);
            } else {
                backupFile.delete();
            }
        }
    }
    
    /**
     * return AES key and generate by using SHA1PRNG
     */
    public synchronized byte[] generateKey(int length) throws KeyServiceException {
        checkClosed();
        
        LOG.debug("generateKey with " + length + " bits");
        KeyGenerator keyGen = keyGeneratorCache.get(length);

        if (keyGen == null) {
            try {
                keyGen = KeyGenerator.getInstance("AES");

                //This is an algorithm-specific metric, specified in number of bits. 
                keyGen.init(length);

                keyGeneratorCache.put(length, keyGen);
            } catch (NoSuchAlgorithmException e) {
                throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
            } catch (InvalidParameterException e) {
                throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
            }
        }
        return keyGen.generateKey().getEncoded();
    }
    
    public void close() throws KeyServiceException {
        isClosed = true;
        try {
            password.destroy();
        } catch (DestroyFailedException e) {
            LOG.warn("Fail to destory password", e);
        }
    }

    public boolean isClosed() {
        return isClosed;
    }

    public boolean checkPassword(IPassword password) throws KeyServiceException {
        return Arrays.equals(this.password.getPassword(), password.getChars());
    }

    public long getLastServerUpdateTime() {
        return lastServerUpdate;
    }

    public void setLastServerUpdateTime(long updated) throws KeyServiceException {
        try {
            KeyStore.Entry entry = new KeyStore.SecretKeyEntry(
                new SecretKeySpec(CodecHelper.toBytes(((Long)updated).toString()), "RAW"));
            keyStore.setEntry(LAST_SERVER_UPDATE_PROPERTY, entry, this.password);

            lastServerUpdate = updated;
        } catch (KeyStoreException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.OTHER);
        }
        
        flush();
    }

    public boolean managedByServer() {
        return serverManaged;
    }

    public void setManagedByServer(boolean flag) throws KeyServiceException {
        try {
            KeyStore.Entry entry = new KeyStore.SecretKeyEntry(
                new SecretKeySpec(CodecHelper.toBytes(((Boolean)flag).toString()), "RAW"));
            keyStore.setEntry(SERVER_MANAGED_PROPERTY, entry, this.password);
        } catch (KeyStoreException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.OTHER);
        }
        
        flush();
    }
}
