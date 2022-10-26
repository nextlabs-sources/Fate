/*
 * Created on Feb 17, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice.impl;

import java.util.Collection;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import com.nextlabs.service.keyservice.IKey;
import com.nextlabs.service.keyservice.IKeyId;
import com.nextlabs.service.keyservice.IKeyRing;
import com.nextlabs.service.keyservice.IPassword;
import com.nextlabs.service.keyservice.KeyServiceException;

/**
 * @author  hchan
 * @version  $Id$
 */

public class CacheLatestKeyRing implements IKeyRing{
    private final IKeyRing parent;
    
    private final Lock r;
    private final Lock w;

    // there is only one latest key id
    private IKeyId latestKeyId;

    
    public CacheLatestKeyRing(IKeyRing parent) throws KeyServiceException {
        this.parent = parent;
        
        ReentrantReadWriteLock lock = new ReentrantReadWriteLock();
        r = lock.readLock();
        w = lock.writeLock();
        
        calcuateLatestMap();
    }
    
    private void calcuateLatestMap() throws KeyServiceException{
        w.lock();
        try{
            latestKeyId = null;
            
            Collection<IKey> keys = parent.getKeys();
            for (IKey key : keys) {
                if (latestKeyId == null || 
                        latestKeyId.getCreationTimeStamp() < key.getCreationTimeStamp()) {
                    latestKeyId = new KeyId(key.getId(), key.getCreationTimeStamp());
                }
            }
        } finally{
            w.unlock();
        }
    }
    
    public void deleteKey(IKeyId keyId) throws KeyServiceException {
        parent.deleteKey(keyId);
        
        if (keyId.equals(latestKeyId)) {
            calcuateLatestMap();
        }
    }
    
    public byte[] generateKey(int length) throws KeyServiceException {
        return parent.generateKey(length);
    }

    public IKey getKey(IKeyId keyId) throws KeyServiceException {
        if (keyId.getCreationTimeStamp() == IKeyId.NO_CREATION_TIMESTAMP) {
            byte[] id = keyId.getId();
            boolean isAllZero = true;
            for (byte b : id) {
                if (b != 0) {
                    isAllZero = false;
                    break;
                }
            }
            if(isAllZero){
                r.lock();
                try {
                    if (latestKeyId != null) {
                        keyId = latestKeyId;
                    }
                } finally {
                    r.unlock();
                }
            }
        }
        
        IKey key = parent.getKey(keyId);
        return key;
    }

    public Collection<IKey> getKeys() throws KeyServiceException {
        //don't return the cached version since it may not cached all keys
        return parent.getKeys();
    }

    public String getName() {
        return parent.getName();
    }

    public void setKey(IKey key) throws KeyServiceException {
        parent.setKey(key);
        w.lock();
        try {
            if (latestKeyId == null || 
                    key.getCreationTimeStamp() > latestKeyId.getCreationTimeStamp()) {
                latestKeyId = new KeyId(key.getId(), key.getCreationTimeStamp());
            }
        } finally {
            w.unlock();
        }
    }

    public void close() throws KeyServiceException {
        parent.close();
    }
    
    public boolean isClosed() {
        return parent.isClosed();
    }

    public boolean checkPassword(IPassword password) throws KeyServiceException {
        return parent.checkPassword(password);
    }

    public long getLastServerUpdateTime() {
        return parent.getLastServerUpdateTime();
    }

    public void setLastServerUpdateTime(long updated) throws KeyServiceException {
        parent.setLastServerUpdateTime(updated);
    }

    public boolean managedByServer() {
        return parent.managedByServer();
    }

    public void setManagedByServer(boolean flag) throws KeyServiceException {
        parent.setManagedByServer(flag);
    }
}
