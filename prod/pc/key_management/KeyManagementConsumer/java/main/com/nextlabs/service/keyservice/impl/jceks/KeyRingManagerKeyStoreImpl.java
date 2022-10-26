/*
 * Created on Feb 5, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice.impl.jceks;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import com.nextlabs.service.keyservice.IKeyRing;
import com.nextlabs.service.keyservice.IKeyRingManager;
import com.nextlabs.service.keyservice.IPassword;
import com.nextlabs.service.keyservice.KeyServiceException;

/**
 * @author hchan
 * @version $Id$
 */

public class KeyRingManagerKeyStoreImpl implements IKeyRingManager {
    public static final String NAME = "JCEKS_KR";
    
    private static final String KEY_STRORE_TYPE = "JCEKS";
    
    private class CacheEntry{
        final IKeyRing keyRing;
        final IPassword password;
        
        CacheEntry(IKeyRing keyRing, IPassword password) {
            this.keyRing = keyRing;
            this.password = password;
        }
    }
    
    private final Map<String, CacheEntry> map;
    private final Lock r;
    private final Lock w;
    
    private boolean isClosed;
    
    private KeyRingManagerKeyStoreImpl() {
        map = new HashMap<String, CacheEntry>();
        isClosed = false;
        
        ReentrantReadWriteLock lock = new ReentrantReadWriteLock();
        r = lock.readLock();
        w = lock.writeLock();
    }

    public String getName() {
        return NAME;
    }

    public IKeyRing openKeyRing(String name, File file, IPassword password, boolean create)
            throws KeyServiceException {
        if (isClosed) {
            throw new KeyServiceException("KeyRingManger is already closed"
                    , KeyServiceException.KEY_RING | KeyServiceException.INVALID_STATE );
        }
        
        if(name == null || name.trim().length() == 0){
            throw new KeyServiceException("keyRing's name can't be emtpy."
                    , KeyServiceException.KEY_RING | KeyServiceException.VALUE_NOT_SET );
        }
        
        //only lookup the cache is not "create"
        if (!create) {
            r.lock();
            try {
                CacheEntry ce = map.get(name);
                if (ce != null && ce.password.equals(password)) {
                    return ce.keyRing;
                }
            } finally {
                r.unlock();
            }
        }
        
        w.lock();
        try {
            boolean newKeyStore;
            KeyStore keyStore;
            try {
                if (file.exists()) {
                    if (create) {
                        throw new KeyServiceException("The KeyRing, " + name+  ", is already exist."
                                , KeyServiceException.KEY_RING | KeyServiceException.ALREADY_EXIST);
                    }
                    keyStore = readKeyStore(file, password);
                    newKeyStore = false;
                } else {
                    if(!create){
                        throw new KeyServiceException("The KeyRing, " + name+  ", does not exist."
                                , KeyServiceException.KEY_RING | KeyServiceException.DOES_NOT_EXIST);
                    }
                    keyStore = createKeyStore(file, password);
                    newKeyStore = true;
                }
            } catch (IOException e) {
                throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
            } catch (KeyStoreException e) {
                throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
            } catch (NoSuchAlgorithmException e) {
                throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
            } catch (CertificateException e) {
                throw new KeyServiceException(e, KeyServiceException.KEY_RING | KeyServiceException.UNKNOWN);
            }
            IKeyRing keyRing = new KeyRingJCEKSImpl(name, keyStore, file, password, newKeyStore);
            map.put(name, new CacheEntry(keyRing, password));
            return keyRing;
        } finally {
            w.unlock();
        }
    }
    
    protected KeyStore getKeyStore() throws KeyStoreException{
        return KeyStore.getInstance(KEY_STRORE_TYPE);
    }
    
    protected KeyStore readKeyStore(File file, IPassword password) throws KeyStoreException,
            NoSuchAlgorithmException, CertificateException, IOException {
        FileInputStream fis = new FileInputStream(file);
        boolean isSuccess = false;
        try {
            KeyStore keyStore = readKeyStore(fis, password);
            isSuccess = true;
            return keyStore;
        } finally {
            try {
                fis.close();
            } catch (IOException e) {
                if (isSuccess) {
                    throw e;
                }
                //TODO log and ignore
            }
        }
    }
    
    protected KeyStore readKeyStore(InputStream is, IPassword password) throws KeyStoreException,
            NoSuchAlgorithmException, CertificateException, IOException {
        KeyStore keyStore = getKeyStore();
        keyStore.load(is, password.getChars());
        return keyStore;
    }

    protected KeyStore createKeyStore(File file, IPassword password) throws KeyStoreException,
            NoSuchAlgorithmException, CertificateException, IOException {
        FileOutputStream fos = new FileOutputStream(file);
        boolean isSuccess = false;
        try {
            KeyStore keyStore = createKeyStore(fos, password);
            isSuccess = true;
            return keyStore;
        } finally {
            try {
                fos.close();
            } catch (IOException e) {
                if (isSuccess) {
                    throw e;
                }
                //TODO log and ignore
            }
        }
    }

    protected KeyStore createKeyStore(OutputStream os, IPassword password) throws KeyStoreException,
            NoSuchAlgorithmException, CertificateException, IOException {
        KeyStore keyStore = getKeyStore();
        keyStore.load(null);
        keyStore.store(os, password.getChars());
        return keyStore;
    }

    public void deleteKeyRing(String name, File file) throws KeyServiceException {
        if (isClosed) {
            throw new KeyServiceException("KeyRingManger is already closed"
                    , KeyServiceException.KEY_RING | KeyServiceException.INVALID_STATE);
        }
        w.lock();
        try {
            CacheEntry ce = map.get(name);
            if (ce != null) {
                ce.keyRing.close();
                map.remove(name);

            }
        } finally {
            w.unlock();
        }
        if (file.exists() && !file.delete()) {
            throw new KeyServiceException("The keyRing is closed but the data can't not be deleted."
                    , KeyServiceException.KEY_RING | KeyServiceException.OTHER);
        }
    }

    public void close() throws KeyServiceException {
        isClosed = true;

        KeyServiceException lastException = null;
        r.lock();
        try {
            for (CacheEntry ce : map.values()) {
                try {
                    ce.keyRing.close();
                } catch (KeyServiceException e) {
                    lastException = e;
                }
            }
        } finally {
            r.unlock();
        }
        if (lastException != null) {
            throw lastException;
        }
    }

    public Collection<IKeyRing> getKeyRings() throws KeyServiceException {
        r.lock();
        try {
            Collection<IKeyRing> rings = new ArrayList<IKeyRing>(map.size());
            for (CacheEntry ce : map.values()) {
                rings.add(ce.keyRing);
            }
            return Collections.unmodifiableCollection(rings);
        } finally {
            r.unlock();
        }
    }

    public boolean isClosed() {
        return isClosed;
    }
}
