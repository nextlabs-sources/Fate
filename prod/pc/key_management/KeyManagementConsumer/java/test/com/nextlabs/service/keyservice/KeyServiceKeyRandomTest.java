/*
 * Created on Feb 24, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Random;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProviderResponse;
import com.nextlabs.service.keyservice.impl.Password;

/**
 * @author hchan
 * @version $Id$
 */
@RunWith(value = Parameterized.class)
public class KeyServiceKeyRandomTest extends KeyServiceTestBase {
    
    @Parameters
    public static Collection<Object> data() {
        Collection<Object> data = new ArrayList<Object>();
        data.add(new Object[] { 1, 1, 10, 500 });
        data.add(new Object[] { 1, 3, 100, 500 });
        data.add(new Object[] { 3, 3, 500, 500 });
        return data;
    }
    
    private final int numberOfKeyRing;
    private final int numberOfKeyId;
    private final int numberOfKey;
    private final int numberOfActions;
    
    private final Random R;

    public KeyServiceKeyRandomTest(int numberOfKeyRing, int numberOfKeyId, int numberOfKey, int numberOfActions) {
        this.numberOfKeyRing = numberOfKeyRing;
        this.numberOfKeyId = numberOfKeyId;
        this.numberOfKey = numberOfKey;
        this.numberOfActions = numberOfActions;
        
        R = new Random();
    }
    
    private enum Action{
        ADD,
        UPDATE,
        DELETE,
    }

    
    private class KeyRingT{
        String keyRingName;
        List<KeyEntrys> keyEntryArray;
        
        KeyRingT(String keyRingName) {
            this.keyRingName = keyRingName;
            keyEntryArray = new ArrayList<KeyEntrys>();
        }

        void createKeyEntrys(byte[] hash) {
            keyEntryArray.add(new KeyEntrys(hash));
        }

        KeyEntrys getRandomKeyEntrys(){
            return keyEntryArray.get(R.nextInt(keyEntryArray.size()));
        }
    }


    @Test
    public void testGetSetKey() {
        
        keyService.setPassword(new Password("keyRingPassword"));
        
        KeyRingT[] keyEntryss = new KeyRingT[numberOfKeyRing];
        for (int i = 0; i < numberOfKeyRing; i++) {
            keyEntryss[i] = new KeyRingT("keyRingName" + i);
            for (int j = 0; j < numberOfKeyId; j++) {
                keyEntryss[i].createKeyEntrys(String.format("hash-%015d", j).getBytes());
            }
        }
        
        for (KeyRingT keyEntrys : keyEntryss) {
            IExternalServiceProviderResponse response = createKeyRing(keyEntrys.keyRingName);
            assertOk(response);
        }
        
        final Random r = new Random();
        final Action[] actions = Action.values();
        int round = 1;
        
        while (round <= numberOfActions) {
            final KeyRingT luckyKeyRingT = keyEntryss[r.nextInt(keyEntryss.length)];
            final KeyEntrys luckyKeyEntrys = luckyKeyRingT.getRandomKeyEntrys();
            final Action action = actions[r.nextInt(actions.length)];
            
            Integer timestamp = null;
            
            switch (action) {
            case ADD:{
                if (luckyKeyEntrys.size() >= numberOfKey) {
                    continue;
                }

                final byte[] keyValue = String.format("key-%016d", round).getBytes();
                timestamp = round;
                //use the round as the timestamp
                IExternalServiceProviderResponse response =
                        setKey(luckyKeyRingT.keyRingName, luckyKeyEntrys.hash, round, keyValue);
                assertOk(response);
                luckyKeyEntrys.add(new KeyEntry(round, keyValue));
            }
                break;
            case UPDATE:{
                if (luckyKeyEntrys.isEmpty()) {
                    continue;
                }
                KeyEntry luckyKey = luckyKeyEntrys.data.get(R.nextInt(luckyKeyEntrys.data.size()));
                timestamp = luckyKey.timestamp;
                
                final byte[] keyValue = String.format("key-%016d", round).getBytes();
                
                //use the round as the timestamp
                IExternalServiceProviderResponse response =
                        setKey(luckyKeyRingT.keyRingName, luckyKeyEntrys.hash, luckyKey.timestamp, keyValue);
                assertOk(response);
                luckyKey.keyValue = keyValue;
            }
                break;
            case DELETE: {
                if (luckyKeyEntrys.isEmpty()) {
                    continue;
                }
                KeyEntry[] keys = luckyKeyEntrys.toArray(new KeyEntry[luckyKeyEntrys.size()]);
                
                KeyEntry luckyKey = keys[R.nextInt(keys.length)];
                timestamp = luckyKey.timestamp;
                
                IExternalServiceProviderResponse response =
                    deleteKey(luckyKeyRingT.keyRingName,
                            luckyKeyEntrys.hash, luckyKey.timestamp);
                assertOk(response);
                luckyKeyEntrys.remove(luckyKey);
            }
                break;
            default:
                break;
            }
            System.out.println(round 
                    + ": (" + luckyKeyRingT.keyRingName + "," 
                    + new String(luckyKeyEntrys.hash) + ","
                    + timestamp + ") "
                    + action);
            
            for (KeyRingT keyEntrys : keyEntryss) {
                assertKeysTotally(keyEntrys.keyEntryArray, keyEntrys.keyRingName);
            }
           
            round++;
        }
    }
    
}
