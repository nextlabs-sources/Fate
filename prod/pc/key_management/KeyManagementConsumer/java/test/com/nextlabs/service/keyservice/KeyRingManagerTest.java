/*
 * Created on Feb 4, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;

import java.io.File;
import java.util.Arrays;
import java.util.Collection;
import java.util.Random;

import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;

import com.nextlabs.service.keyservice.IKey;
import com.nextlabs.service.keyservice.IKeyId;
import com.nextlabs.service.keyservice.IKeyRing;
import com.nextlabs.service.keyservice.IKeyRingManager;
import com.nextlabs.service.keyservice.InvalidKeyException;
import com.nextlabs.service.keyservice.KeyServiceException;
import com.nextlabs.service.keyservice.impl.CacheLatestKeyRing;
import com.nextlabs.service.keyservice.impl.Key;
import com.nextlabs.service.keyservice.impl.KeyId;
import com.nextlabs.service.keyservice.impl.KeyRingManagerFactory;
import com.nextlabs.service.keyservice.impl.Password;
import com.nextlabs.service.keyservice.impl.jceks.KeyRingManagerKeyStoreImpl;

import static org.junit.Assert.*;

/**
 * @author hchan
 * @version $Id$
 */

public class KeyRingManagerTest {
    File keyStoreFile;
    IKeyRingManager manager;
    
    @Before
    public void init() {
        keyStoreFile = new File(System.getProperty("java.io.tmpdir"), "testKeyStore.bin");
        if (keyStoreFile.exists()) {
            assertTrue(keyStoreFile.delete());
        }
        File backup = new File(System.getProperty("java.io.tmpdir"), "testKeyStore.bin.bak");
        if (backup.exists()) {
            assertTrue(backup.delete());
        }
        manager = KeyRingManagerFactory.getManager(KeyRingManagerKeyStoreImpl.NAME);
        assertNotNull(manager);
    }
    
    @Test
    public void testKeyStore() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        Password wrongPassword = new Password("abcdefg");
        
        assertFalse(keyStoreFile.exists());
        
        IKeyRing keyring;
        try {
            keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, false);
            fail();
        } catch (KeyServiceException e) {
            assertEquals(KeyServiceException.KEY_RING | KeyServiceException.DOES_NOT_EXIST, e.getErrorCode());
        }
        
        keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);
        assertNotNull("File doesn't exist and keyring is created", keyring);
        
        
        try {
            keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);
            fail();
        } catch (KeyServiceException e) {
            assertEquals(KeyServiceException.KEY_RING | KeyServiceException.ALREADY_EXIST, e.getErrorCode());
        }
        assertNotNull("File exisits and can open again", keyring);
        
        try {
            keyring = manager.openKeyRing(keyRingName, keyStoreFile, wrongPassword, false);
            fail();
        } catch (KeyServiceException e) {
            assertNotNull("can't open a keyring with wrong password", e);
        }
        
        try {
            keyring = manager.openKeyRing(keyRingName+ "!", keyStoreFile, password, false);
            fail();
        } catch (KeyServiceException e) {
            assertNotNull("keyring can't open with wrong name", e);
        }
    }
    
    @Test
    public void testGenerateKey() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);
        
        byte[] key = keyring.generateKey(128);
        System.out.println(Arrays.toString(key));
        
        try {
            keyring.generateKey(71);
            fail();
        } catch (KeyServiceException e) {
            assertNotNull("should not be able to handle such odd bit at this moment", e);
        }
        
        try {
            keyring.generateKey(0);
            fail();
        } catch (KeyServiceException e) {
            assertNotNull("should not be able to handle such odd bit at this moment", e);
        }
    }
    
    @Test (timeout = 15000)
    @Ignore
    public void generateKeyPerformance128() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        final IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);
        
        long startTime = System.currentTimeMillis();
        long count = 0;
        long delta = 0;
        while (delta <= 10000) {
            keyring.generateKey(128);
            count++;
            delta = System.currentTimeMillis() - startTime;
        }
        fail((float)(count / (double)delta) + " keys/ms");
        //247.22186 keys/ms
    }
    
    @Test (timeout = 15000)
    @Ignore
    public void generateKeyPerformance256() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        final IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);
        
        long startTime = System.currentTimeMillis();
        long count = 0;
        long delta = 0;
        while (delta <= 10000) {
            keyring.generateKey(256);
            count++;
            delta = System.currentTimeMillis() - startTime;
        }
        fail((float)(count / (double)delta) + " keys/ms");
        //161.77104 keys/ms
    }
    
        
    @Test
    public void testKey() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        final IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);

        KeyId keyId1 = new KeyId("key1".getBytes(), System.currentTimeMillis());

        IKey returnKey1 = keyring.getKey(keyId1);
        assertNull("the unsaved key doesn't exist", returnKey1);
        
        //save key
        Key key1 = new Key(keyId1, "secret1".getBytes());
        keyring.setKey(key1);

        returnKey1 = keyring.getKey(keyId1);
        assertNotNull("the saved key should exist", returnKey1);
        assertEquals("same as the key I try to save", key1, returnKey1);

        //there is only one key
        Collection<IKey> keys = keyring.getKeys();
        assertNotNull(keys);
        assertEquals(1, keys.size());
        
        //save the other key
        KeyId keyId2 = new KeyId("key2".getBytes(), System.currentTimeMillis());
        Key key2 = new Key(keyId2, "secret2".getBytes());
        keyring.setKey(key2);
        
        //there are two keys now
        keys = keyring.getKeys();
        assertNotNull(keys);
        assertEquals(2, keys.size());
        for (IKey key : keys) {
            assertTrue(key1.equals(key) || key2.equals(key));
        }
        
        keyring.deleteKey(keyId2);
        keys = keyring.getKeys();
        assertEquals(1, keys.size());
        for (IKey key : keys) {
            assertTrue(key1.equals(key));
        }
        
        keyring.deleteKey(keyId1);
        keys = keyring.getKeys();
        assertTrue(keys.isEmpty());
    }
    
    @Test
    public void testKeyError() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        final IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);

        KeyId keyId1 = new KeyId("key1".getBytes(), IKeyId.NO_CREATION_TIMESTAMP);

        Key key1 = new Key(keyId1, "secret1".getBytes());
        try {
            keyring.setKey(key1);
            fail();
        } catch (KeyServiceException e) {
            assertNotNull(e);
        }
    }
    

    private int writeDate(IKeyRing keyring, long timeout) throws InvalidKeyException,
            KeyServiceException {
        int count = 0;
        long delta = 0;
        long startTime = System.currentTimeMillis();
        while (delta <= timeout) {
            byte[] bytes = Integer.toString(count).getBytes();
            keyring.setKey(new Key(new KeyId(bytes, startTime), bytes));
            count++;
            delta = System.currentTimeMillis() - startTime;
        }
        return count;
    }
    
    @Test (timeout = 15000)
    @Ignore
    public void addKeyPerformance() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        final IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);
        
        long startTime = System.currentTimeMillis();
        int count = writeDate(keyring, 10000);
        long delta = System.currentTimeMillis() - startTime;
        fail((float)((double)count / delta) + " keys/ms");
        //0.022330774 keys/ms
    }
    
    private void getKeyPerformance(IKeyRing keyring) throws InvalidKeyException, KeyServiceException{
        long timeout = 10000;
        int writeCount = writeDate(keyring, timeout);
        assertTrue("less than 0.01 keys/ms is too slow", writeCount > timeout / 100);

        
        long readCount;
        long delta;
        long startTime;
        
        
        //read same
        readCount = 0;
        delta = 0;
        KeyId staticKeyId = new KeyId(Integer.toString(1).getBytes(), System.currentTimeMillis());
        startTime = System.currentTimeMillis();
        while (delta <= timeout) {
            keyring.getKey(staticKeyId);
            readCount++;
            delta = System.currentTimeMillis() - startTime;
        }
        float sameRead = (float)((double)readCount / delta);
        
        //read sequence
        readCount = 0;
        delta = 0;
        startTime = System.currentTimeMillis();
        int index = 0;
        while (delta <= timeout) {
            byte[] bytes = Integer.toString(index++).getBytes();
            keyring.getKey(new KeyId(bytes, startTime));
            readCount++;
            if (index >= writeCount) {
                index = 0;
            }
            delta = System.currentTimeMillis() - startTime;
        }
        float readSeq = (float)((double)readCount / delta);
        
        
        //random read
        Random r = new Random(); 
        readCount = 0;
        delta = 0;
        startTime = System.currentTimeMillis();
        while (delta <= timeout) {
            byte[] bytes = Integer.toString(r.nextInt(writeCount)).getBytes();
            keyring.getKey(new KeyId(bytes, startTime));
            readCount++;
            delta = System.currentTimeMillis() - startTime;
        }
        float randomReadHit = (float)((double)readCount / delta);
        
        
        readCount = 0;
        delta = 0;
        startTime = System.currentTimeMillis();
        while (delta <= timeout) {
            byte[] bytes = Integer.toString(r.nextInt(writeCount * 2)).getBytes();
            keyring.getKey(new KeyId(bytes, startTime));
            readCount++;
            delta = System.currentTimeMillis() - startTime;
        }
        float randomRead = (float)((double)readCount / delta);
        
        fail("sameRead = "      + sameRead      + "\n"
           + "readSeq = "       + readSeq       + "\n"
           + "randomReadHit = " + randomReadHit + "\n"
           + "randomRead = "    + randomRead    + "\n"
        );
    }
    
    @Test (timeout = 60000)
    @Ignore
    public void getKeyPerformance() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        final IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);

        getKeyPerformance(keyring);
//        sameRead = 540.6159
//        readSeq = 351.17932
//        randomReadHit = 327.65723
//        randomRead = 326.11813
    }
    
    
    @Test (timeout = 15000)
    @Ignore
    public void addKeyPerformanceCached() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);
        keyring = new CacheLatestKeyRing(keyring); 
        
        long startTime = System.currentTimeMillis();
        int count = writeDate(keyring, 10000);
        long delta = System.currentTimeMillis() - startTime;
        fail((float)((double)count / delta) + " keys/ms");
        //0.022394745 keys/ms
    }
    
    @Test (timeout = 60000)
    @Ignore
    public void getKeyPerformanceCached() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);

        keyring = new CacheLatestKeyRing(keyring); 
        
        getKeyPerformance(keyring);
//            hit = 30068696
//            not_found = 905
//            expired = 0
//            memory = 1000
        
//        sameRead = 1289.315
//        readSeq = 617.01306
//        randomReadHit = 564.47833
//        randomRead = 560.9132
    }
    
    @Test
    public void testDeleteKey() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);

        keyring = new CacheLatestKeyRing(keyring); 
        
        byte[] id = "key".getBytes();
        KeyId keyId1 = new KeyId(id, 1000);
        //nothing happen
        keyring.deleteKey(keyId1);
        
        //add key
        Key key1 = new Key(keyId1, "secret1".getBytes());
        keyring.setKey(key1);
        
        //key is delete
        keyring.deleteKey(keyId1);
        
        //delete again but nothing happen
        keyring.deleteKey(keyId1);
        
        assertTrue(keyring.getKeys().isEmpty());
    }
    
    @Test
    public void testLatestTime() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);

        keyring = new CacheLatestKeyRing(keyring); 
        
        byte[] id = "key".getBytes();
        KeyId keyId1 = new KeyId(id, 1000);
        KeyId keyId2 = new KeyId(id, 2000);
        KeyId keyId3 = new KeyId(id, 3000);
        KeyId keyId4 = new KeyId(id, 4000);
        KeyId latestKeyId = new KeyId(new byte[]{}, 0);
        
        IKey latestKey;
        //save key
        Key key1 = new Key(keyId1, "secret1".getBytes());
        keyring.setKey(key1);
        //<--oldest  key1  newest--> 
        latestKey = keyring.getKey(latestKeyId);
        assertEquals(key1, latestKey);
        
        Key key2 = new Key(keyId2, "secret2".getBytes());
        keyring.setKey(key2);
        //<--oldest  key1 key2  newest-->
        latestKey = keyring.getKey(latestKeyId);
        assertEquals(key2, latestKey);
        
        Key key4 = new Key(keyId4, "secret4".getBytes());
        keyring.setKey(key4);
        //<--oldest  key1 key2 key4  newest-->
        latestKey = keyring.getKey(latestKeyId);
        assertEquals(key4, latestKey);
        
        //insert something in the pass
        Key key3 = new Key(keyId3, "secret3".getBytes());
        keyring.setKey(key3);
        //<--oldest  key1 key2 key3 key4  newest-->
        latestKey = keyring.getKey(latestKeyId);
        assertEquals(key4, latestKey);
        
        //delete latest key
        keyring.deleteKey(key4);
        //<--oldest  key1 key2 key3  newest-->
        latestKey = keyring.getKey(latestKeyId);
        assertEquals(key3, latestKey);
        
        //delete old key
        keyring.deleteKey(keyId2);
        //<--oldest  key1 key3  newest-->
        latestKey = keyring.getKey(latestKeyId);
        assertEquals(key3, latestKey);
    }
    
    @Test
    @Ignore
    public void testMultiThread() throws KeyServiceException {
        String keyRingName = "firstKeyRing";
        Password password = new Password("123456");
        
        IKeyRing keyring = manager.openKeyRing(keyRingName, keyStoreFile, password, true);

        keyring = new CacheLatestKeyRing(keyring); 
        
        byte[] id = "key".getBytes();
        
        final int numberOfThread = 16;
        
        Thread[] threads = new Thread[numberOfThread];
        for (int i = 0; i < numberOfThread; i++) {
            threads[i] = new Thread(new ThreadTest(keyring, id, (i + 1) * 1000));
        }

        for (int i = 0; i < numberOfThread; i++) {
            threads[i].start();
        }
        
        try {
            Thread.sleep(1000*60*5);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
    
    private class ThreadTest implements Runnable{
        final IKeyRing keyring;
        final KeyId keyId;
        final Key key;
        
        
        ThreadTest(IKeyRing keyring, byte[] id, long timestamp) throws InvalidKeyException{
            this.keyring = keyring;
            keyId = new KeyId(id, timestamp);
            key = new Key(keyId, ("secret" + timestamp).getBytes());
        }
        
        public void run() {
            Random random = new Random();
            try {
                while (true) {
                    try {
                        Thread.sleep(random.nextInt(5));
                    } catch (InterruptedException e) {}
                   
                    keyring.setKey(key);
                    IKey returnKey = keyring.getKey(keyId);
                    assertNotNull(returnKey);
                    assertEquals(key, returnKey);
                    keyring.deleteKey(keyId);
                    returnKey = keyring.getKey(keyId);
                    assertNull(returnKey);
                }
            } catch (KeyServiceException e) {
                e.printStackTrace();
            }
        }
        
    }
}
