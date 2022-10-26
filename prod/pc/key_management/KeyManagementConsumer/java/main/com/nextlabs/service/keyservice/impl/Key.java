/*
 * Created on Feb 4, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice.impl;

import java.util.Arrays;

import com.nextlabs.service.keyservice.IKey;
import com.nextlabs.service.keyservice.IKeyId;
import com.nextlabs.service.keyservice.InvalidKeyException;
import com.nextlabs.service.keyservice.KeyServiceException;


/**
 * @author  hchan
 * @version  $Id$
 */

public class Key implements IKey, IKeyId{
    private static final int CURRENT_VERSION = 1;
    
    private final int structureVersion;
    private final byte[] key;
    private final IKeyId keyId;
    
    public Key(IKeyId keyId, int structureVersion, byte[] key)
            throws InvalidKeyException {
        this.keyId = keyId;
        if (key == null) {
            throw new InvalidKeyException("Key can't be null.", KeyServiceException.VALUE_NOT_SET);
        }  
            
        if (key.length == 0) {
            throw new InvalidKeyException("Key can't be empty", KeyServiceException.VALUE_NOT_SET);
        }
        this.structureVersion = structureVersion;
        this.key = new byte[key.length];
        System.arraycopy(key, 0, this.key, 0, key.length);
    }
    
    public Key(IKeyId keyId, byte[] key) throws InvalidKeyException {
        this(keyId, CURRENT_VERSION, key);
    }

    @Override
    public String toString() {
        return "key" + keyId.toString().substring(5) + "\n" 
            + "  version: " + structureVersion + "\n"
            + "  key: " + Arrays.toString(key);
    }
    
    public int getStructureVersion() {
        return structureVersion;
    }
    
    public byte[] getEncoded(){
        return key.clone();
    }

    public IKeyId getKeyId() {
        return keyId;
    }
    
    public long getCreationTimeStamp() {
        return keyId.getCreationTimeStamp();
    }

    public byte[] getId() {
        return keyId.getId();
    }
    
    

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + Arrays.hashCode(key);
        result = prime * result + ((keyId == null) ? 0 : keyId.hashCode());
        result = prime * result + structureVersion;
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        
        if(obj instanceof IKeyId){
            return this.keyId.equals(obj);
        }
        
        if(!(obj instanceof IKey)){
            return false;
        }
        
        IKey other = (IKey) obj;
        if (!Arrays.equals(key, other.getEncoded()))
            return false;
        if (keyId == null) {
            if (other.getKeyId() != null)
                return false;
        } else if (!keyId.equals(other.getKeyId()))
            return false;
        if (structureVersion != other.getStructureVersion())
            return false;
        return true;
    }

    @Override
    protected void finalize() throws Throwable {
        if (key != null) {
            Arrays.fill(key, (byte) 0);
        }
        super.finalize();
    }
}
