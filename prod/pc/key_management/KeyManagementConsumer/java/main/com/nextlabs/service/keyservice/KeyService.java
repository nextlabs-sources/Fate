/*
 * Created on Feb 18, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;

import java.io.File;
import java.io.FileFilter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Serializable;
import java.io.StringWriter;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.bluejungle.destiny.agent.controlmanager.ITrustedProcessManager;
import com.bluejungle.destiny.agent.controlmanager.TrustedProcessManager;
import com.bluejungle.framework.comp.ComponentManagerFactory;
import com.bluejungle.framework.utils.SerializationUtils;
import com.bluejungle.pf.destiny.lib.KeyResponseDTO;
import com.bluejungle.pf.destiny.lib.KeyRequestDTO;
import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProvider;
import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProviderResponse;
import com.bluejungle.pf.domain.destiny.serviceprovider.IHeartbeatServiceProvider;
import com.bluejungle.pf.domain.destiny.serviceprovider.INextlabsExternalSPResponseCode;
import com.nextlabs.service.keyservice.impl.CachedKeyRingManager;
import com.nextlabs.service.keyservice.impl.CodecHelper;
import com.nextlabs.service.keyservice.impl.Key;
import com.nextlabs.service.keyservice.impl.KeyId;
import com.nextlabs.service.keyservice.impl.KeyRingManagerFactory;
import com.nextlabs.service.keyservice.impl.Password;
import com.nextlabs.service.keyservice.impl.jceks.KeyRingManagerKeyStoreImpl;
import com.nextlabs.pf.destiny.lib.KeyManagerConstants;

/**
 * @author hchan
 * @version $Id$
 */

public class KeyService implements IHeartbeatServiceProvider, IExternalServiceProvider {
    private static final Log LOG = LogFactory.getLog(KeyService.class); 

    private static final String KEYRING_FILE_PREFIX = "KM_";
    private static final String KEYRING_FILE_SUFFIX = ".jceks";
    private static final String DEFAULT_MANAGER_NAME = KeyRingManagerKeyStoreImpl.NAME;
    
    //there is only one base parameter which is the operation
    //the pre parameter is id (operation id), nonce, encrypted nonce
    private static final int NUMBER_OF_PRE_PARAMETERS = 3;
    
    //the post parameter is process id
    private static final int NUMBER_OF_POST_PARAMETERS = 1;
    
    //in milli seconds
    private static final int AUTH_PROCESS_ALLOWED_TIMEOUT = 10000;
    
    private static Map<String, Operation> REVERSE_MAP = new HashMap<String, Operation>();
    
    private KeyRingUpdateInfo updateInfo = new KeyRingUpdateInfo();

    private static String rootDirectory = ".";

    private static IKeyServiceSDK sdk;

    enum Operation {
        
        CREATE_KEYRING("CREATERING", 
                String.class // keyRingName
        ){
            @Override
            IExternalServiceProviderResponse run(KeyService keyService, Object[] objs)
                    throws KeyServiceException {
                String keyRingName = (String) objs[NUMBER_OF_PRE_PARAMETERS + 0];
                keyService.createKeyRing(keyRingName);
                return keyService.responseFactory.ok();
            }
        },
        
        
        DELETE_KEYRING("DELETERING", 
                String.class // keyRingName
        ){
            @Override
            IExternalServiceProviderResponse run(KeyService keyService, Object[] objs)
                    throws KeyServiceException {
                String keyRingName = (String) objs[NUMBER_OF_PRE_PARAMETERS + 0];
                keyService.krManager.deleteKeyRing(keyRingName, keyService.convertToFile(keyRingName));
                return keyService.responseFactory.ok();
            }
        },          
        

        LIST_KEYRINGS_NAME("GETALLRINGS"
                            // no parameters
        ){
            @Override
            IExternalServiceProviderResponse run(KeyService keyService, Object[] objs) {
                List<String> keyRingNames = getAllKeyRingNames(keyService);
                return keyService.responseFactory.convertKeyRings(keyRingNames);
            }
        },            
        
        
        GENERATE_KEY("GENERATEKEY", 
                String.class    // keyRingName
              , Integer.class   // key length
        ){
            @Override
            IExternalServiceProviderResponse run(KeyService keyService, Object[] objs)
                    throws KeyServiceException {
                String keyRingName = (String)  objs[NUMBER_OF_PRE_PARAMETERS + 0];
                int lengthInByte   = (Integer) objs[NUMBER_OF_PRE_PARAMETERS + 1];

                IKeyRing keyRing = keyService.openKeyRing(keyRingName);
                byte[] keyValue = keyRing.generateKey(lengthInByte * 8);
                
                MessageDigest digest;
                try {
                    digest = MessageDigest.getInstance("SHA1");
                } catch (NoSuchAlgorithmException e) {
                    throw new RuntimeException(e);
                }
                
                // don't call get byte since it is depends on the default encoding
                byte[] hash = digest.digest(keyValue);
                assert hash.length == 20;
                //pad zero's
                hash = ArrayHelper.pad(hash, KeyServiceResponseFactory.HASH_LENGTH);
                IKeyId keyId = new KeyId(hash, System.currentTimeMillis() / 1000 * 1000);
                IKey key = new Key(keyId, keyValue);
                keyRing.setKey(key);
                return keyService.responseFactory.convert(keyId);
            }
        },
        
        
        GET_KEY("GETKEY", 
                String.class    // keyRingName
              , String.class    // hash base16
              , Integer.class   // timestamp
        ){
            @Override
            IExternalServiceProviderResponse run(KeyService keyService, Object[] objs)
                    throws KeyServiceException {
                String keyRingName  = (String)  objs[NUMBER_OF_PRE_PARAMETERS + 0];
                String hashBase16   = (String)  objs[NUMBER_OF_PRE_PARAMETERS + 1];
                int timestampInS    = (Integer) objs[NUMBER_OF_PRE_PARAMETERS + 2];
                
                IKeyId keyId = keyService.responseFactory.revertToKeyId(hashBase16, timestampInS);
                IKeyRing keyRing = keyService.openKeyRing(keyRingName);
                IKey key = keyRing.getKey(keyId);
                return key != null 
                        ? keyService.responseFactory.convert(key, encryptor) 
                        : keyService.responseFactory.fail(new Exception("The key is not found"));
            }
        }, 
        
        
        SET_KEY("SETKEY", 
                String.class    // keyRingName
              , Integer.class   // structure version
              , String.class    // hash base16
              , Integer.class   // timestamp
              , String.class    // key base16
              , Integer.class   // keyLength
        ){
            @Override
            IExternalServiceProviderResponse run(KeyService keyService, Object[] objs)
                    throws KeyServiceException {
                String keyRingName      = (String)  objs[NUMBER_OF_PRE_PARAMETERS + 0];
                int structureVersion    = (Integer) objs[NUMBER_OF_PRE_PARAMETERS + 1];
                String hashBase16       = (String)  objs[NUMBER_OF_PRE_PARAMETERS + 2];
                int timestampInS        = (Integer) objs[NUMBER_OF_PRE_PARAMETERS + 3];
                String keyValueBase16   = (String)  objs[NUMBER_OF_PRE_PARAMETERS + 4];
                byte[] keyValue         = CodecHelper.base16Decode(keyValueBase16);
                int keyLength           = (Integer) objs[NUMBER_OF_PRE_PARAMETERS + 5];
                if (keyValue.length < keyLength) {
                    throw new com.nextlabs.service.keyservice.InvalidKeyException(
                            "The keyLength (" + keyLength + ") doesn't match the length of the key (" 
                            + keyValue.length + ")."
                            , KeyServiceException.INVALID_FORMAT
                    );
                }
                keyValue = ArrayHelper.trim(keyValue, keyLength);
                
                IKeyId keyId = keyService.responseFactory.revertToKeyId(hashBase16, timestampInS);
                IKey key = new Key(keyId, structureVersion, keyValue);
                IKeyRing keyRing = keyService.openKeyRing(keyRingName);
                keyRing.setKey(key);
                return keyService.responseFactory.ok();
            }
        }, 
        
        
        DELETE_KEY("DELETEKEY", 
                String.class    // keyRingName, 
              , String.class    // hash base16
              , Integer.class   // timestamp 
        ){
            @Override
            public IExternalServiceProviderResponse run(KeyService keyService, Object[] objs)
                    throws KeyServiceException {
                String keyRingName  = (String)  objs[NUMBER_OF_PRE_PARAMETERS + 0];
                String hashBase16   = (String)  objs[NUMBER_OF_PRE_PARAMETERS + 1];
                int timestampInS    = (Integer) objs[NUMBER_OF_PRE_PARAMETERS + 2];
                
                IKeyId keyId = keyService.responseFactory.revertToKeyId(hashBase16, timestampInS);
                IKeyRing keyRing = keyService.openKeyRing(keyRingName);
                keyRing.deleteKey(keyId);
                return keyService.responseFactory.ok();
            }
        }, 
        
        
        LIST_KEYIDS("GETALLKEYS", 
                String.class    // keyRingName
        ){
            @Override
            public IExternalServiceProviderResponse run(KeyService keyService, Object[] objs)
                    throws KeyServiceException {
                String keyRingName = (String) objs[NUMBER_OF_PRE_PARAMETERS + 0];
                IKeyRing keyRing = keyService.openKeyRing(keyRingName);
                Collection<IKey> keys = keyRing.getKeys();
                List<IKey> keyList = new ArrayList<IKey>(keys);
                
                //sort by the timestamp
                Collections.sort(keyList, new Comparator<IKey>() {
                    public int compare(IKey o1, IKey o2) {
                        //latest time go first
                        return new Long(o2.getCreationTimeStamp()).compareTo(o1.getCreationTimeStamp());
                    }
                });
                return keyService.responseFactory.convertKeyIds(keyList);
            }
        },
        ;

        private final Class<?>[] inputClasses;
        private final String id;
        Operation(String id, Class<?>... inputClasses) {
            REVERSE_MAP.put(id, this);
            this.id = id;
            this.inputClasses = inputClasses;
        }
        
        String getId() {
            return id;
        }

        void check(KeyService keyService, Object[] objs) throws InvalidMethodSignature{
            int length = inputClasses.length;
            int expectedLength = NUMBER_OF_PRE_PARAMETERS + NUMBER_OF_POST_PARAMETERS+ length;
            if (objs.length != expectedLength) {
                throw InvalidMethodSignature.wrongLength(expectedLength, objs.length);
            }

            for (int i = 0; i < length; i++) {
                if (!(inputClasses[i].isInstance(objs[NUMBER_OF_PRE_PARAMETERS + i]))) {
                    //oops, wrong class

                    //get all class from input. It will be used in the exception
                    Class<?>[] actualClazzes = new Class<?>[objs.length];
                    for (int j = 0; j < length; j++) {
                        Object acutalObj = objs[NUMBER_OF_PRE_PARAMETERS + j];
                        actualClazzes[j] = acutalObj != null ? acutalObj.getClass() : null;
                    }
                    throw InvalidMethodSignature.wrongClass(inputClasses, actualClazzes);
                }
            }

            // The pid will always be the last argument
            int pid = (Integer)objs[objs.length-1];
            if (pid != -1 && !keyService.getTrustedProcessManager().isTrustedProcess(pid)) {
                throw InvalidMethodSignature.untrustedProcessId(pid);
            }
            
        }
        
        private IExternalServiceProviderResponse execute(KeyService keyService, Object[] objs)
                throws InvalidMethodSignature, KeyServiceException {
            check(keyService, objs);
            KeyService.check((Integer) objs[1], (String) objs[2]);
            return run(keyService, objs);
        }
        
        abstract IExternalServiceProviderResponse run(KeyService keyService, Object[] objs)
                throws KeyServiceException;
    }
    
    static class InvalidMethodSignature extends Exception {
        private int errorCode;

        InvalidMethodSignature(String message, int errorCode) {
            super(message);
            this.errorCode = errorCode;
        }
        
        InvalidMethodSignature(String message) {
            this(message, INextlabsExternalSPResponseCode.CE_RESULT_INVALID_PARAMS);
        }
        
        static InvalidMethodSignature emptyInput() {
            return new InvalidMethodSignature("The input objects can not be empty.");
        }
        
        static InvalidMethodSignature wrongLength(int expect, int actual) {
            return new InvalidMethodSignature(String.format(
                    "The number of expected arguments is %1$d but the actual is %2$d.", expect, actual));
        }
    
        static InvalidMethodSignature untrustedProcessId(int pid) {
            return new InvalidMethodSignature("The process id " + pid + " is untrusted",
                                              INextlabsExternalSPResponseCode.CE_RESULT_PERMISSION_DENIED);
        }

        static InvalidMethodSignature wrongClass(Class<?>[] expect, Class<?>[] actual) {
            assert expect.length == actual.length;
            
            int length = expect.length;
            
            StringBuilder expectSb = new StringBuilder();
            StringBuilder actualSb = new StringBuilder();
            for (int i = 0; i < length; i++) {
                if (i != 0) {
                    expectSb.append(", ");
                    actualSb.append(", ");
                }
                expectSb.append(expect[i].getName());
                actualSb.append(actual[i] == null ? "null" : actual[i].getName());
            }
            
            return new InvalidMethodSignature(String.format(
                    "The input classes '%2$s' don't match the expected classes '%1$s'", 
                    expectSb, actualSb)
            );
        }
        
        static InvalidMethodSignature unknownOperation(Object obj) {
            return new InvalidMethodSignature(String.format(
                    "The operation '%1$s' is unknown.", obj), 
                    INextlabsExternalSPResponseCode.CE_RESULT_FUNCTION_NOT_AVAILBLE);
        }
    }
    
   
    private IKeyRingManager krManager;
    private File keyRingFolder;
    private KeyServiceResponseFactory responseFactory;
    private IPassword password;
    private ITrustedProcessManager trustedProcessManager;

    ITrustedProcessManager getTrustedProcessManager() {
        return trustedProcessManager;
    }

    public void init() {
        setRootDirectory();

        trustedProcessManager = (ITrustedProcessManager)ComponentManagerFactory.getComponentManager().getComponent(TrustedProcessManager.COMP_INFO);

        krManager = KeyRingManagerFactory.getManager(DEFAULT_MANAGER_NAME);
        if (krManager == null) {
            throw new NullPointerException("KeyRingManager is not initialized.");
        }
        
        krManager = new CachedKeyRingManager(krManager);
        keyRingFolder = new File(rootDirectory + File.separator + "config", "security");
        responseFactory = new KeyServiceResponseFactory();
        
        try {
            this.password = readPassword();
        } catch (IOException e) {
            throw new RuntimeException("can't get the key password", e);
        }
        Operation.values();
        
        initDefaultKeyRings();

        sdk = KeyServiceSDK.createKeyServiceSDK(this);
    }
    
    private void initDefaultKeyRings() {
        //TODO this should comes from configuration file and can have more than one.
        final String defaultKeyRingName = "NL_LOCAL";
        IKeyRing keyRing;
        try {
            LOG.trace("creating default key ring " + defaultKeyRingName);
            keyRing = createKeyRing(defaultKeyRingName);
        } catch (KeyServiceException e) {
            if((e.getErrorCode() & (KeyServiceException.KEY_RING | KeyServiceException.ALREADY_EXIST)) == 0){
                // this is not duplicated exception
                LOG.error("can't create default key ring '" + defaultKeyRingName + "'", e);
                return;
            } else {
                LOG.trace("The default key ring '" + defaultKeyRingName + "' is already created.");
                try {
                    keyRing = openKeyRing(defaultKeyRingName);
                } catch (KeyServiceException e1) {
                    LOG.error("unable to open key ring '" + defaultKeyRingName + "'", e);
                    return;
                }
            }
        }
        
        try {
            LOG.trace("check if keyring '" + defaultKeyRingName + "' is empty?");
            Collection<IKey> keys = keyRing.getKeys();
            if(keys.isEmpty()){
                LOG.trace("keyring '" + defaultKeyRingName + "' is empty. creating a new key");
                byte[] keyValue = keyRing.generateKey(8 * 16); // 16bytes
                
                //TODO I am C&P
                MessageDigest digest;
                try {
                    digest = MessageDigest.getInstance("SHA1");
                } catch (NoSuchAlgorithmException e) {
                    throw new RuntimeException(e);
                }
                
                // don't call get byte since it is depends on the default encoding
                byte[] hash = digest.digest(keyValue);
                assert hash.length == 20;
                //pad zero's
                hash = ArrayHelper.pad(hash, KeyServiceResponseFactory.HASH_LENGTH);
                IKeyId keyId = new KeyId(hash, System.currentTimeMillis() / 1000 * 1000);
                IKey key = new Key(keyId, keyValue);
                keyRing.setKey(key);
            } else {
                String message = "keyring '" + defaultKeyRingName + "' contains " + keys.size() +". ";
                IKey firstKey = keys.iterator().next();
                message += "First key is " + Arrays.toString(firstKey.getId()) + " ts=" + firstKey.getCreationTimeStamp();
                LOG.trace(message);
            }
        } catch (KeyServiceException e) {
            LOG.error("unable to generate key in key ring '" + defaultKeyRingName + "'", e);
            return;
        }
    }
    
    /*
     *  start internal API
     */
    
    private static Cipher encryptor;
    private static Cipher decryptor;

    static {
        byte[] raw = new byte[]{7, -117, 34, -79, -74, 85, -10, -63, -99, -120, 103, 15, -48, -46, -8, -88};

        SecretKeySpec skeySpec = new SecretKeySpec(raw, "AES");
        
        try {
            encryptor = Cipher.getInstance("AES/ECB/PKCS5Padding");
            encryptor.init(Cipher.ENCRYPT_MODE, skeySpec);
        } catch (Exception e) {
            throw new ExceptionInInitializerError(e);
        }
        
        try {
            decryptor = Cipher.getInstance("AES/ECB/PKCS5Padding");
            decryptor.init(Cipher.DECRYPT_MODE, skeySpec);
        } catch (Exception e) {
            throw new ExceptionInInitializerError(e);
        }
    }
    
    private static List<String> getAllKeyRingNames(KeyService keyService) {
        File[] keyRingFiles = keyService.keyRingFolder.listFiles(new FileFilter() {
            public boolean accept(File pathname) {
                if (pathname.isFile()) {
                    String fileName = pathname.getName();
                    if (fileName.startsWith(KEYRING_FILE_PREFIX)
                        && fileName.endsWith(KEYRING_FILE_SUFFIX)) {
                        return true;
                    }
                }
                return false;
            }
        });
        
        List<String> keyRingNames = new ArrayList<String>(keyRingFiles.length);
        for(File keyRingFile : keyRingFiles){
            String fileName = keyRingFile.getName();
            fileName = fileName.substring(KEYRING_FILE_PREFIX.length(), 
                                          fileName.length() - KEYRING_FILE_SUFFIX.length());
            keyRingNames.add(fileName);
        }
        //case sensitive sort
        Collections.sort(keyRingNames);
        return keyRingNames;
    }

    private static void check(int timestamp, String encryptedTsBase16) throws InvalidMethodSignature{
        long currentTime = System.currentTimeMillis();
        if (currentTime > ((long)timestamp * 1000 + AUTH_PROCESS_ALLOWED_TIMEOUT)) {
            throw new InvalidMethodSignature("The authentication is already expired.", 
                    INextlabsExternalSPResponseCode.CE_RESULT_APPLICATION_AUTH_FAILED);
        }
       
        try {
            byte[] encryptedTs = CodecHelper.base16Decode(encryptedTsBase16);
            byte[] originalTs = decryptor.doFinal(encryptedTs);
            if (originalTs.length != 16) {
                throw new InvalidMethodSignature("The authentication value is invalid format.", 
                        INextlabsExternalSPResponseCode.CE_RESULT_APPLICATION_AUTH_FAILED); 
            }
            int decryptedTimestamp = 0;
            
            for(int i = 0; i < 16; i++){
                decryptedTimestamp *= 10;
                decryptedTimestamp += originalTs[i] - '0';
            }
            
            if(timestamp != decryptedTimestamp){
                throw new InvalidMethodSignature("The authentication is failed.", 
                        INextlabsExternalSPResponseCode.CE_RESULT_APPLICATION_AUTH_FAILED); 
            }
            
        } catch (Exception e) {
            throw new InvalidMethodSignature(e.getMessage(), 
                    INextlabsExternalSPResponseCode.CE_RESULT_APPLICATION_AUTH_FAILED);
        }
        
        //the timestamp is good
    }
    
    void setPassword(IPassword password) {
        this.password = password;
    }

    private IPassword readPassword() throws IOException{
        File passwordFile = new File(rootDirectory + File.separator + "config", "config2.dat");
        String password;
        if (passwordFile.exists()) {
            //read password file
            StringWriter writer;
            writer = new StringWriter(32);
            
            FileReader reader= new FileReader(passwordFile);
            try {
                int c;
                while ((c = reader.read()) != -1) {
                    writer.write(c);
                }
            } finally {
                reader.close();
            }
            // the string writer doesn't necessary need to close.
            writer.close();
            password = writer.toString();
        } else {
            FileWriter writer = new FileWriter(passwordFile);
            FileWriter backupwriter = 
                new FileWriter(new File(passwordFile.getParent(), passwordFile.getName() + ".bak"));
            char[] chars;
            try {
                Random r = new Random();
                int length = 32;
                chars = new char[length];
                for (int i = 0; i < length; i++) {
                    char c = (char) ('!' + r.nextInt('~' - '!'));
                    chars[i] = c;
                    writer.write(c);
                    backupwriter.write(c);
                }
            } finally {
                writer.close();
                backupwriter.close();
            }
            password = new String(chars);
        }
        
        return new Password(password);
    }
    

    /*
     *  end internal API
     */
    
    private File convertToFile(String keyRingName){
        return new File(keyRingFolder, KEYRING_FILE_PREFIX + keyRingName + KEYRING_FILE_SUFFIX);
    }

    private IKeyRing createKeyRing(String keyRingName) throws KeyServiceException {
        return openKeyRing(keyRingName, true);
    }

    IKeyRing openKeyRing(String keyRingName) throws KeyServiceException {
        return openKeyRing(keyRingName, false);
    }

    private IKeyRing openKeyRing(String keyRingName, boolean create) throws KeyServiceException {
        if (password == null) {
            throw new KeyServiceException("The keyRing password is not set."
                    , KeyServiceException.KEY_RING | KeyServiceException.VALUE_NOT_SET);
        }
        if(keyRingName.length() > KeyManagerConstants.MAX_KEY_RING_NAME_LIMIT){
            throw new KeyServiceException("The keyRingName can't more than " + KeyManagerConstants.MAX_KEY_RING_NAME_LIMIT + " characters."
                    , KeyServiceException.KEY_RING | KeyServiceException.VALUE_NOT_SET);
        }
        
        //TODO check invalid characters
        
        IKeyRing keyRing = krManager.openKeyRing(keyRingName, 
                convertToFile(keyRingName),
                password, 
                create);
        assert keyRing != null;
        return keyRing;
    }
    
    public IExternalServiceProviderResponse invoke(Object[] objs) {
        if (LOG.isTraceEnabled()) {
            LOG.trace("The request is " + Arrays.toString(objs));
        }
        IExternalServiceProviderResponse r;
        try {
            //quick check inputs
            if (objs == null || objs.length == 0) {
                throw InvalidMethodSignature.emptyInput();
            }
            
            if(objs[0] == null){
                throw InvalidMethodSignature.unknownOperation(objs[0]);
            }
            
            //get operation
            Operation operation = REVERSE_MAP.get(objs[0]);
            if (operation == null) {
                throw InvalidMethodSignature.unknownOperation(objs[0]);
            }
            
            //each operation set IExternalServiceProviderResponse or throw exception
            r = operation.execute(this, objs);
        } catch (KeyServiceException e) {
            LOG.debug("Exception in the request: " + Arrays.toString(objs), e);
            r = responseFactory.fail(e);
        } catch (InvalidMethodSignature e) {
            LOG.debug("Exception in the request: " + Arrays.toString(objs), e);
            r= responseFactory.create(e.errorCode, e.toString());
        } catch (Exception e){
            LOG.error("Exception in the request: " + Arrays.toString(objs), e);
            r = responseFactory.fail(e);
        }
        if (LOG.isTraceEnabled()) {
            LOG.trace("The response is " + r);
        }
        return r;
    }

    private class KeyRingUpdateInfo {
        private Map<String, Long> updateTimes = null;

        private void getKeyTimestamps(KeyService keyService) {
            if (updateTimes != null) {
                return;
            }

            updateTimes = new HashMap<String, Long>();

            // get all key ring information from key store
            List<String> keyRingNames = getAllKeyRingNames(keyService);

            for (String keyRingName : keyRingNames) {
                // Get timestamp
                try {
                    IKeyRing keyRing = keyService.openKeyRing(keyRingName);
                    long lastUpdate = keyRing.getLastServerUpdateTime();
                    
                    updateTimes.put(keyRingName, lastUpdate);
                } catch (KeyServiceException e) {
                    LOG.warn("Unable to open key " + keyRingName + e);
                }
            }
        }

        synchronized public long getTimestamp(KeyService serviceProvider, String name) {
            getKeyTimestamps(serviceProvider);

            Long ts = updateTimes.get(name);
            
            if (ts == null) {
                return IKeyRing.NEVER_UPDATED_BY_SERVER;
            } else {
                return ts;
            }
        }
        
        synchronized public void flush() {
            updateTimes = null;
        }
    }

    public Serializable prepareRequest(String reqName) {
        LOG.info("Preparing request: " + reqName);
        KeyRequestDTO request = null;

        if (KeyRequestDTO.SERVICE_NAME.equals(reqName)) {
            request = new KeyRequestDTO();

            List<String> keyRings = getAllKeyRingNames(this);
            
            for (String keyRing : keyRings) {
                request.addKeyRing(keyRing, updateInfo.getTimestamp(this, keyRing));
            }
        }

        return request;
    }

    public void processResponse(String reqName, String data) {
        processResponse(reqName, SerializationUtils.unwrapSerialized(data));
    }

    public void processResponse(String reqName, Serializable data) {
        LOG.info("Processing response: " + reqName);
        if (KeyRequestDTO.SERVICE_NAME.equals(reqName) && data instanceof KeyResponseDTO) {
            KeyResponseDTO response = (KeyResponseDTO)data;

            for (KeyResponseDTO.KeyRingDTO keyRingDTO : response.getKeyRings()) {
                try {
                    // Delete existing key ring
                    String keyRingName = keyRingDTO.getName();
                    LOG.info("Deleting key ring " + keyRingName);
                    krManager.deleteKeyRing(keyRingName, convertToFile(keyRingName));

                    // Create new one with the same name
                    LOG.info("Creating new key ring " + keyRingName);
                    IKeyRing newKeyRing = createKeyRing(keyRingName);

                    // Add in keys
                    for (KeyResponseDTO.KeyDTO keyDTO : keyRingDTO.getKeys()) {
                        LOG.info(".. adding key");
                        IKeyId keyId = new KeyId(keyDTO.getId(), keyDTO.getTimestamp());
                        IKey key = new Key(keyId,
                                           keyDTO.getVersion(),
                                           keyDTO.getKey());
                        newKeyRing.setKey(key);
                    }

                    // Set server update information
                    newKeyRing.setLastServerUpdateTime(keyRingDTO.getTimestamp());

                    // And mark this is as a key ring managed by the server
                    newKeyRing.setManagedByServer(true);

                } catch (KeyServiceException e) {
                }

                if (!response.getKeyRings().isEmpty()) {
                    updateInfo.flush();
                }
            }
        }
        return;
    }

    private void setRootDirectory() {
        rootDirectory = System.getProperty("dpc.install.home");
        if (rootDirectory == null || rootDirectory.trim().length() < 1) {
            rootDirectory = ".";
        }
        
        LOG.info("Setting root directory to " + rootDirectory);
    }        
}
