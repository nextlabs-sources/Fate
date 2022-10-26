/*
 * Created on Feb 22, 2010
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

import org.junit.Test;

import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProviderResponse;
import com.bluejungle.pf.domain.destiny.serviceprovider.INextlabsExternalSPResponseCode;
import com.nextlabs.service.keyservice.KeyService;
import com.nextlabs.service.keyservice.KeyService.InvalidMethodSignature;
import com.nextlabs.service.keyservice.impl.Password;
/**
 * @author hchan
 * @version $Id$
 */

public class KeyServiceTest extends KeyServiceTestBase implements INextlabsExternalSPResponseCode {
    @Test
    public void testNullArguments(){
        IExternalServiceProviderResponse response = keyService.invoke(null);
        assertInvalidMethodSignature(response, InvalidMethodSignature.emptyInput());
    }
    
    @Test
    public void testEmptyArguments(){
        IExternalServiceProviderResponse response = keyService.invoke(new Object[]{});
        assertInvalidMethodSignature(response, InvalidMethodSignature.emptyInput());
    }
    
    @Test
    public void testInvalidArguments(){
        IExternalServiceProviderResponse response;
        Object[] input;
        
        input = new Object[10];
        response = keyService.invoke(input);
        assertInvalidMethodSignature(response, InvalidMethodSignature.unknownOperation(input[0]));
        
        
        input = new Object[1];
        input[0] = new Integer(-972762075);
        response = keyService.invoke(input);
        assertInvalidMethodSignature(response, InvalidMethodSignature.unknownOperation(input[0]));
    }
    

    @Test
    public void testCREATE_KEYRING(){
        IExternalServiceProviderResponse response;
        
        keyService.setPassword(null); 
        
        //create key ring needs profile password
        final String keyRingName1 = "keyRingName1";
        response = createKeyRing(keyRingName1);
        assertKeyServiceException(response);
        
        keyService.setPassword(new Password("keyRingPassword"));
        
        response = createKeyRing(keyRingName1);
        assertOk(response);

        final String keyRingName2 = "keyRingName2";
        response = createKeyRing(keyRingName2);
        assertOk(response);
        
        //create again?!?
        response = createKeyRing(keyRingName2);
        assertFail(response);
    }
    
    @Test
    public void testCREATE_KEYRINGInvalidParameters(){
        IExternalServiceProviderResponse response;
        
        keyService.setPassword(new Password("keyRingPassword"));
        
        //null
        response = createKeyRing(null);
        assertInvalidMethodSignature(response, InvalidMethodSignature.wrongClass(
                new Class[]{String.class}, new Class[]{null}));
        
        //empty string
        response = createKeyRing("");
        assertKeyServiceException(response);
        
        //one space
        response = createKeyRing(" ");
        assertKeyServiceException(response);
        
        //too many arguments
        response = invoke(KeyService.Operation.CREATE_KEYRING.getId(), "abc", "def", PROCESS_ID);
        assertInvalidMethodSignature(response, InvalidMethodSignature.wrongLength(5, 4));
    }
    
    @Test
    public void testDELETE_KEYRING(){
        IExternalServiceProviderResponse response;
        
        //delete doesn't need password
        //delete doesn't exist keystore doesn't do anything
        final String keyRingName1 = "keyRingName1";
        response = deleteKeyRing(keyRingName1);
        assertOk(response);
        
        keyService.setPassword(new Password("keyRingPassword"));
        
        response = createKeyRing(keyRingName1);
        assertOk(response);
        
        response = deleteKeyRing(keyRingName1);
        assertOk(response);
    }
    
    @Test
    public void testLIST_KEYRINGS(){
        IExternalServiceProviderResponse response;
        Object[] data;
        int i;
        
        keyService.setPassword(new Password("keyRingPassword"));
        
        
        //no keyrings yet
        
        response = listKeyRingNames();
        assertResponse(response);
        data = response.getData();
        assertEquals("ii", response.getFormatString());
        assertArrayEquals(new Object[]{CE_RESULT_SUCCESS, 0}, data);
        
        
        //add one keyrig
        
        final String keyRingName1 = "keyRingName one";
        response = createKeyRing(keyRingName1);
        assertOk(response);
        
        
        // one keyring now
        
        response = listKeyRingNames();
        assertResponse(response);
        data = response.getData();
        assertEquals("iis", response.getFormatString());
        assertArrayEquals(new Object[] { CE_RESULT_SUCCESS, 1, keyRingName1}, data);
        
        
        // add the other keyring
        
        final String keyRingName2 = "keyRingName two";
        response = createKeyRing(keyRingName2);
        assertOk(response);
     
        
        // two keyrings now
        
        response = listKeyRingNames();
        assertResponse(response);
        data = response.getData();
        assertEquals("iiss", response.getFormatString());
        //the keyring is not sorted
        i = 0;
        assertEquals(CE_RESULT_SUCCESS, data[i++]);
        assertEquals(2, data[i++]);
        assertTrue(data[i].equals(keyRingName1) || data[i].equals(keyRingName2));
        i++;
        assertTrue(data[i].equals(keyRingName1) || data[i].equals(keyRingName2));
        i++;
        
        // delete one keyring
        
        response = deleteKeyRing(keyRingName1);
        assertOk(response);
        
        
        // only keyring2 exists
        
        response = listKeyRingNames();
        assertResponse(response);
        data = response.getData();
        assertEquals("iis", response.getFormatString());
        assertArrayEquals(new Object[] {CE_RESULT_SUCCESS, 1, keyRingName2}, data);
        
        
        // delete the other one
        
        response = deleteKeyRing(keyRingName2);
        assertOk(response);
        
        
        // no keyring left
        
        response = listKeyRingNames();
        assertResponse(response);
        data = response.getData();
        assertEquals("ii", response.getFormatString());
        assertArrayEquals(new Object[]{CE_RESULT_SUCCESS, 0}, data);
    }
    
    @Test
    public void testGENERATE_KEY(){
        IExternalServiceProviderResponse response;
        Object[] data;
        
        keyService.setPassword(new Password("keyRingPassword"));
        
        final String keyRingName1 = "keyRingName1";
        response = createKeyRing(keyRingName1);
        assertOk(response);
        
        //128 bits, 16 bytes
        response = generateKey(keyRingName1, 16);
        data = response.getData();
        assertEquals("isi", response.getFormatString());
        assertEquals(CE_RESULT_SUCCESS, data[0]);
        assertNotNull(data[1]);
        
        //the key length must be 16 (128 bit), 24 (192 bit) or 32 (256 bits)
        response = generateKey(keyRingName1, 30);
        assertKeyServiceException(response);
        
        //missing key length
        response = invoke(KeyService.Operation.GENERATE_KEY.getId(), keyRingName1, PROCESS_ID);
        assertInvalidMethodSignature(response, InvalidMethodSignature.wrongLength(6, 3));
        
        //missing key ring name
        response = invoke(KeyService.Operation.GENERATE_KEY.getId(), 30, PROCESS_ID);
        assertInvalidMethodSignature(response, InvalidMethodSignature.wrongLength(6, 3));
        
        //wrong order
        int now = (int)(System.currentTimeMillis() / 1000);
        response = invoke(KeyService.Operation.GENERATE_KEY.getId(), now, encryptTimestamp(now),30, keyRingName1, PROCESS_ID);
        assertInvalidMethodSignature(response, InvalidMethodSignature.wrongClass(
                new Class[] { String.class, Integer.class, }, 
                new Class[] { Integer.class, String.class, }));
    }
    
    
    @Test
    public void create1000keys() {
        IExternalServiceProviderResponse response;
        Object[] data;
        
        keyService.setPassword(new Password("keyRingPassword"));
        
        final String keyRingName1 = "keyRingName1";
        response = createKeyRing(keyRingName1);
        assertOk(response);
        
        for(int i=0; i<1000; i++){
            //128 bits, 16 bytes
            response = generateKey(keyRingName1, 16);
            data = response.getData();
            assertEquals("isi", response.getFormatString());
            assertEquals(CE_RESULT_SUCCESS, data[0]);
            assertNotNull(data[1]);
        }
        
        response = invoke(KeyService.Operation.LIST_KEYIDS.getId(), keyRingName1, PROCESS_ID);
//        System.out.println(response);
    }
    
    @Test
    public void testGetSetKey(){
        final byte[] hash1 = "hashhash01hashhash01".getBytes();
        
        KeyEntrys records1 = new KeyEntrys(hash1);
        
        IExternalServiceProviderResponse response;
        
        keyService.setPassword(new Password("keyRingPassword"));
        
        final String keyRingName1 = "keyRingName1";
        response = createKeyRing(keyRingName1);
        assertOk(response);
        
        final Integer timestamp1 = 1111;
        
        // <-oldest    newest->
        response = getKey(keyRingName1, hash1, timestamp1 * 1000);
        assertResponse(response);
        assertEquals("is", response.getFormatString());
        assertEquals(INextlabsExternalSPResponseCode.CE_RESULT_GENERAL_FAILED, response.getData()[0]);
        assertKeysTotally(records1, keyRingName1);
        
        
        // <-oldest  k1  newest->
        final byte[] keyValue1 = "key01key01key01key01".getBytes();
        response = setKey(keyRingName1, hash1, timestamp1, keyValue1);
        assertOk(response);
        records1.add(new KeyEntry(timestamp1, keyValue1));
        assertKeysTotally(records1, keyRingName1);
        
        
        // <-oldest  k1 k1b  newest->
        final byte[] keyValue1b = "key1bkey1bkey1bkey1b".getBytes();
        final Integer timestamp1b = 2222;
        response = setKey(keyRingName1, hash1, timestamp1b, keyValue1b);
        assertOk(response);
        records1.add(new KeyEntry(timestamp1b, keyValue1b));
        assertKeysTotally(records1, keyRingName1);
        
        
        // <-oldest  k1 k1b k1d  newest->
        final byte[] keyValue1d = "key1dkey1dkey1dkey1d".getBytes();
        final Integer timestamp1d = 4444;
        response = setKey(keyRingName1, hash1, timestamp1d, keyValue1d);
        assertOk(response);
        records1.add(new KeyEntry(timestamp1d, keyValue1d));
        assertKeysTotally(records1, keyRingName1);
        
        
        // <-oldest  k1 k1b k1c k1d  newest->
        final byte[] keyValue1c = "key1ckey1ckey1ckey1c".getBytes();
        final Integer timestamp1c = 3333;
        response = setKey(keyRingName1, hash1, timestamp1c, keyValue1c);
        assertOk(response);
        records1.add(new KeyEntry(timestamp1c, keyValue1c));
        assertKeysTotally(records1, keyRingName1);
    }
    
    
//    
//    @Test
//    public void testDELETE_KEY(){
//        IExternalServiceProviderResponse response;
//        Object[] input;
//
//    }
//    
//    @Test
//    public void testLIST_KEYS(){
//        IExternalServiceProviderResponse response;
//        Object[] input;
//
//    }
//    
//    @Test
//    public void testSET_PASSWORD(){
//        IExternalServiceProviderResponse response;
//        Object[] input;
//
//    }
    
}
