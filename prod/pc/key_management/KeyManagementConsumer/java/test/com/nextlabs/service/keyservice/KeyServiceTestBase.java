/*
 * Created on Feb 24, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.FileFilter;
import java.lang.reflect.Field;
import java.security.GeneralSecurityException;
import java.util.AbstractSet;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import javax.crypto.Cipher;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;

import com.bluejungle.destiny.agent.controlmanager.ITrustedProcessManager;
import com.bluejungle.destiny.agent.controlmanager.TrustedProcessManager;
import com.bluejungle.framework.comp.ComponentInfo;
import com.bluejungle.framework.comp.ComponentManagerFactory;
import com.bluejungle.framework.comp.IComponentManager;
import com.bluejungle.framework.comp.LifestyleType;
import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProviderResponse;
import com.bluejungle.pf.domain.destiny.serviceprovider.INextlabsExternalSPResponseCode;
import com.nextlabs.service.keyservice.IKeyRingManager;
import com.nextlabs.service.keyservice.KeyService;
import com.nextlabs.service.keyservice.KeyServiceException;
import com.nextlabs.service.keyservice.KeyService.InvalidMethodSignature;
import com.nextlabs.service.keyservice.impl.CodecHelper;
import com.nextlabs.service.keyservice.impl.KeyId;
import com.nextlabs.service.keyservice.impl.KeyRingManagerFactory;

/**
 * @author hchan
 * @version $Id$
 */

public class KeyServiceTestBase implements INextlabsExternalSPResponseCode{
    static final int PROCESS_ID = -238502073;
    
    static String keyRingFilePrefix;
    static String keyRingFileSuffix;
    static String defaultManagerName;
    static Field keyRingFolderField;
    
    KeyService keyService;
    KeyServiceResponseFactory rf;
    
    static Cipher encryptor;
    static Cipher decryptor;
    
    @BeforeClass
    public static void initGlobal() throws Exception {
        
        Field keyRingFilePrefixField = KeyService.class.getDeclaredField("KEYRING_FILE_PREFIX");
        keyRingFilePrefixField.setAccessible(true);
        keyRingFilePrefix = (String)keyRingFilePrefixField.get(null);
        
        Field keyRingFileSuffixField = KeyService.class.getDeclaredField("KEYRING_FILE_SUFFIX");
        keyRingFileSuffixField.setAccessible(true);
        keyRingFileSuffix = (String)keyRingFileSuffixField.get(null);
        
        Field defaultManagerNameField = KeyService.class.getDeclaredField("DEFAULT_MANAGER_NAME");
        defaultManagerNameField.setAccessible(true);
        defaultManagerName = (String)defaultManagerNameField.get(null);
        
        keyRingFolderField = KeyService.class.getDeclaredField("keyRingFolder");
        keyRingFolderField.setAccessible(true);
        
        
        IComponentManager cm = ComponentManagerFactory.getComponentManager();
        
        ComponentInfo<ITrustedProcessManager> info = 
            new ComponentInfo<ITrustedProcessManager>(
		    TrustedProcessManager.class.getName(),
                    AlwaysTrusted.class, 
                    LifestyleType.SINGLETON_TYPE
        );
        
        cm.registerComponent(info, true);
        
        Field encryptorField = KeyService.class.getDeclaredField("encryptor");
        encryptorField.setAccessible(true);
        encryptor = (Cipher)encryptorField.get(null);
        assertNotNull(encryptor);
        
        Field decryptorField = KeyService.class.getDeclaredField("decryptor");
        decryptorField.setAccessible(true);
        decryptor = (Cipher)decryptorField.get(null);
        assertNotNull(decryptorField);
    }
    
    public static class AlwaysTrusted implements ITrustedProcessManager{

        @Override
        public void addTrustedProcess(int processID) {
        }

        @Override
        public boolean isTrustedProcess(int processID) {
            return true;
        }

        @Override
	public void addPermanentTrustedProcess(int processID) {
	}
    }
    
    @Before
    public void init() throws Exception {
        new File("config").mkdirs();
        new File("config", "config2.dat").createNewFile();
        
        keyService = new KeyService();
        keyService.init();
        
        File keyRingFolder = (File)keyRingFolderField.get(keyService);;

        if (keyRingFolder.exists()) {
            File[] keyRingFiles = keyRingFolder.listFiles(new FileFilter() {
                public boolean accept(File pathname) {
                    if( pathname.isFile()){
                       String filename = pathname.getName();
                        if (filename.startsWith(keyRingFilePrefix)
                            && (filename.endsWith(keyRingFileSuffix) 
                                || filename.endsWith(keyRingFileSuffix + ".bak")
                                )
                        ) {
                            return true;
                        }
                    }
                    return false;
                }
            });
            
            for(File keyRingFile : keyRingFiles){
                assertTrue(keyRingFile.delete());
            }
        }else{
            keyRingFolder.mkdirs();
        }
        rf = new KeyServiceResponseFactory();
    }
    
    @After
    public void cleanup() throws Exception{
        //the only way to clear the cache
        IKeyRingManager manager = KeyRingManagerFactory.getManager(defaultManagerName);
        manager.close();
    }
    
    protected IExternalServiceProviderResponse invoke(Object... objs){
        return keyService.invoke(objs);
    }
    
    static void assertResponse(IExternalServiceProviderResponse response){
        assertNotNull(response);
        Object[] data = response.getData();
        
        assertTrue(response.toString(), response.getFormatString().startsWith("i"));
        assertNotNull(data);
    }
    
    static void assertInvalidMethodSignature(IExternalServiceProviderResponse response, InvalidMethodSignature exception) {
        assertFail(response);
        assertEquals(exception.toString(), response.getData()[1]);
    }
    
    static void assertKeyServiceException(IExternalServiceProviderResponse response) {
        assertFail(response);
        String message = (String)response.getData()[1];
        assertTrue(message, message.startsWith(KeyServiceException.class.getName()));
    }
    
    static void assertFail(IExternalServiceProviderResponse response) {
        assertResponse(response);
        Object[] data = response.getData();
        
        assertEquals(2, data.length);
        assertTrue(!data[0].equals(CE_RESULT_SUCCESS));
    }
    
    static void assertOk(IExternalServiceProviderResponse response) {
        assertResponse(response);
        Object[] data = response.getData();
        
        assertEquals(response.toString(), 1, data.length);
        assertEquals(CE_RESULT_SUCCESS, data[0]);
    }
    
    
    static void assertKeyEquals(Integer structure, byte[] hash, Integer timestamp, byte[] keyValue,
            IExternalServiceProviderResponse response) {
        assertKeyEquals(structure, hash, timestamp, keyValue, response, 1);
        assertEquals(6, response.getData().length);
    }
    
    static void assertKeyEquals(Integer structure, byte[] hash, Integer timestamp, byte[] keyValue, 
            IExternalServiceProviderResponse response, int offset){
        Object[] data = response.getData();
        assertTrue(response.toString(), data.length >= (3 + offset));
        
        assertEquals(structure, (Integer)data[offset + 0]);
        assertHashEqual(hash,   CodecHelper.base16Decode((String) data[offset + 1]));
        assertEquals(timestamp, (Integer)data[offset + 2]);
        byte[] actualKeyValue = CodecHelper.base16Decode((String) data[offset + 3]);
        try {
            assertArrayEquals(keyValue, ArrayHelper.trim(decryptor.doFinal(actualKeyValue), 20));
        } catch (GeneralSecurityException e) {
            throw new RuntimeException(e);
        }
        assertEquals((Integer)keyValue.length, (Integer)data[offset + 4]);
    }
    
    static void assertHashEqual(byte[] expected, byte[] actual){
        assertTrue(actual.length >= expected.length);
        int i;
        for (i =0 ; i < expected.length; i++) {
            assertEquals(expected[i], actual[i]);
        }
        for (; i < actual.length; i++) {
            assertEquals(0, actual[i]);
        }
    }
    
    class KeyEntry implements Comparable<KeyEntry>{
        Integer structure;
        Integer timestamp;
        byte[] keyValue;
        
        KeyEntry(Integer timestamp, byte[] keyValue) {
            this.structure = 1;
            this.timestamp = timestamp;
            this.keyValue = keyValue;
        }

        public int compareTo(KeyEntry o) {
            return this.timestamp.compareTo(o.timestamp);
        }
    }
    
    class KeyEntrys extends AbstractSet<KeyEntry>{
        byte[] hash;
        
        ArrayList<KeyEntry> data;

        KeyEntrys(byte[] hash) {
            super();
            this.hash = hash;
            this.data = new ArrayList<KeyEntry>();
        }

        @Override
        public Iterator<KeyEntry> iterator() {
            return data.iterator();
        }

        @Override
        public int size() {
            return data.size();
        }

        @Override
        public boolean add(KeyEntry o) {
            if (data.contains(o)) {
                throw new RuntimeException();
            }
            data.add(o);
            Collections.sort(data);
            return true;
        }
        
        @Override
        public boolean remove(Object o) {
            return data.remove(o);
        }

        public KeyEntry last() {
            return data.get(data.size() - 1);
        }
    }
    
    void assertKeysTotally(KeyEntrys expected, String keyRingName){
        assertKeysTotally(Collections.singleton(expected), keyRingName);
    }
    
    void assertKeysTotally(Collection<KeyEntrys> expected, String keyRingName){
        IExternalServiceProviderResponse response;
        
        int totalSize = 0;
        
        KeyEntry latest = null;
        byte[] latestHash = null;
        
        // hash of the keyId.hash -> timestamp in second -> KeyEntry 
        Map<Integer, Map<Integer, KeyEntry>> map = new HashMap<Integer, Map<Integer, KeyEntry>>(expected.size());
        for (KeyEntrys ee : expected) {
            totalSize += ee.size();
            Map<Integer, KeyEntry> map2 = new HashMap<Integer, KeyEntry>(ee.size());
            for (KeyEntry e : ee) {
                map2.put(e.timestamp, e);
            }
            map.put(Arrays.hashCode(ee.hash), map2);
            if (!ee.isEmpty()) {
                if (latest != null) {
                    KeyEntry thisLatest = ee.last();
                    if (thisLatest.timestamp > latest.timestamp) {
                        latest = thisLatest;
                        latestHash = ee.hash;
                    }
                } else {
                    latest = ee.last();
                    latestHash = ee.hash;
                }
            }
        }
        
        //test list keys
        response = listKeyIds(keyRingName);
        
        assertResponse(response);
        
        //response code, size
        final int headerSize = 2;
        //hash, timestamp
        final int entrySize = 2;
        
        Object[] data = response.getData();
        assertEquals(response.toString(), totalSize, data[1]);
        
        //times two before each keyid is "si", two elements
        assertEquals(headerSize + (totalSize * entrySize), data.length);

        //each key is 4 items in the array
        //the header is 2 items, one is the return code, one is the size
        for (int i = headerSize; i < data.length; i += entrySize) {
            String hashBase16   = (String) response.getData()[i];
            int timestamp       = (Integer) response.getData()[i + 1];
            IKeyId keyId;
            try {
                keyId = rf.revertToKeyId(hashBase16, timestamp);
            } catch (KeyServiceException e1) {
                throw new RuntimeException(e1);
            }
            byte[] hash = ArrayHelper.trim(keyId.getId(), 20);
            KeyEntry e = map.get(Arrays.hashCode(hash)).get((int)(keyId.getCreationTimeStamp()/1000));
            assertNotNull(e);
        }
        
        for (KeyEntrys ee : expected) {
            for (KeyEntry e : ee) {
                response = getKey(keyRingName, ee.hash, e.timestamp);
                assertKeyEquals(e.structure, ee.hash, e.timestamp, e.keyValue, response);
            }
        }
        
        //check the latest key
        response = getKey(keyRingName, new byte[]{}, 0);
        
        if (latest == null) {
            assertEquals("is", response.getFormatString());
            assertEquals(INextlabsExternalSPResponseCode.CE_RESULT_GENERAL_FAILED, response.getData()[0]);
        } else {
            assertKeyEquals(latest.structure, latestHash, latest.timestamp, latest.keyValue, response);
        }
    }
    
    static String encryptTimestamp(int timestamp){
        byte[] message = new byte[16];
        
        for(int i=15; i>=0 ;i--){
            if(timestamp == 0){
                break;
            }
            message[i] = (byte)((timestamp % 10) + '0');
            timestamp /= 10;
        }
        
        try {
            message = encryptor.doFinal(message);
        } catch (GeneralSecurityException e) {
            throw new RuntimeException(e);
        }
        return CodecHelper.base16Encode(message);
    }
    
    IExternalServiceProviderResponse createKeyRing(
            String keyRingName){
        int now = (int)(System.currentTimeMillis() / 1000);
        return invoke(KeyService.Operation.CREATE_KEYRING.getId(),
                now,
                encryptTimestamp(now),
                keyRingName,
                PROCESS_ID
        );
    }
    
    IExternalServiceProviderResponse deleteKeyRing(
            String keyRingName) {
        int now = (int)(System.currentTimeMillis() / 1000);
        return invoke(KeyService.Operation.DELETE_KEYRING.getId(),
                now,
                encryptTimestamp(now),
                keyRingName,
                PROCESS_ID
        );
    }
    
    IExternalServiceProviderResponse listKeyRingNames() {
        int now = (int)(System.currentTimeMillis() / 1000);
        return invoke(KeyService.Operation.LIST_KEYRINGS_NAME.getId(),
                now,
                encryptTimestamp(now),
                PROCESS_ID
        );
    }
    
    IExternalServiceProviderResponse generateKey(
            String keyRingName,
            int keyLength) {
        int now = (int)(System.currentTimeMillis() / 1000);
        return invoke(KeyService.Operation.GENERATE_KEY.getId(),
                now,
                encryptTimestamp(now),
                keyRingName,
                keyLength,
                PROCESS_ID
        );
    }
    
    IExternalServiceProviderResponse getKey(
            String keyRingName, 
            byte[] hash,
            int timestamp) {
        IKeyId keyId = new KeyId(hash, timestamp * 1000);
        Object[] objs = rf.convertKeyId(keyId);
        int i=0;
        Object[] request = new Object[5 + objs.length];
        request[i++] = KeyService.Operation.GET_KEY.getId();
        int now = (int)(System.currentTimeMillis() / 1000);
        request[i++] = now;
        request[i++] = encryptTimestamp(now);
        request[i++] = keyRingName;
        for (Object obj : objs) {
            request[i++] = obj;
        }
        request[i++] = PROCESS_ID;
        return invoke(request);
    }
    
    IExternalServiceProviderResponse setKey(
            String keyRingName, 
            byte[] hash,
            int timestamp, 
            byte[] keyValue) {
        IKeyId keyId = new KeyId(hash, timestamp * 1000);
        
        Object[] objs = rf.convertKeyId(keyId);
        int i=0;
        Object[] request = new Object[8 + objs.length];
        request[i++] = KeyService.Operation.SET_KEY.getId();
        int now = (int)(System.currentTimeMillis() / 1000);
        request[i++] = now;
        request[i++] = encryptTimestamp(now);
        request[i++] = keyRingName;
        request[i++] = 1;
        for (Object obj : objs) {
            request[i++] = obj;
        }
        request[i++] = CodecHelper.base16Encode(keyValue);
        request[i++] = keyValue.length;
        request[i++] = PROCESS_ID;
        return invoke(request);
    }
    
    IExternalServiceProviderResponse deleteKey(
            String keyRingName, 
            byte[] hash,
            int timestamp) {
        IKeyId keyId = new KeyId(hash, timestamp * 1000);
        Object[] objs = rf.convertKeyId(keyId);
        int i=0;
        Object[] request = new Object[5 + objs.length];
        request[i++] = KeyService.Operation.DELETE_KEY.getId();
        int now = (int)(System.currentTimeMillis() / 1000);
        request[i++] = now;
        request[i++] = encryptTimestamp(now);
        request[i++] = keyRingName;
        for (Object obj : objs) {
            request[i++] = obj;
        }
        request[i++] = PROCESS_ID;
        return invoke(request);
    }
    
    IExternalServiceProviderResponse listKeyIds(
            String keyRingName) {
        int now = (int)(System.currentTimeMillis() / 1000);
        return invoke(KeyService.Operation.LIST_KEYIDS.getId(),
                now,
                encryptTimestamp(now),
                keyRingName,
                PROCESS_ID
        );
    }
    
}
