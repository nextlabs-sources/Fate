/*
 * Created on Feb 4, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice.impl;

import java.util.Arrays;

import com.nextlabs.service.keyservice.IKeyId;

/**
 * @author  hchan
 * @version  $Id$
 */

public class KeyId implements IKeyId{
    private final byte[] id;
    
    private final long creationTimeStamp;
    
    public KeyId(byte[] hash, long createiontTimeStamp) {
        this.id = hash;
        this.creationTimeStamp = createiontTimeStamp;
    }
    
    public KeyId(byte[] hash) {
        this(hash, NO_CREATION_TIMESTAMP);
    }
    
//    public KeyId(String hash, long creationTimeStamp) {
//        char[] chars = hash.toCharArray();
//        id = new byte[chars.length * 2];
//        
//        for (int i = 0; i < chars.length; i++) {
//            id[i *2] = (byte)(chars[i] >> 8);
//            id[i *2 + 1] = (byte)(chars[i]);      
//        }
//        this.creationTimeStamp = creationTimeStamp;
//    }
    
    public byte[] getId() {
        return id;
    }
    
    public long getCreationTimeStamp(){
        return creationTimeStamp;
    }
    
    @Override
    public String toString() {
        return "KeyId" + "\n" 
            + "  id: " + Arrays.toString(id) + "\n" 
            + "  time: " + creationTimeStamp;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + (int) (creationTimeStamp ^ (creationTimeStamp >>> 32));
        result = prime * result + Arrays.hashCode(id);
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (!(obj instanceof IKeyId)) {
            return false;
        }
        IKeyId other = (IKeyId) obj;
        if (creationTimeStamp != other.getCreationTimeStamp())
            return false;
        if (!Arrays.equals(id, other.getId()))
            return false;
        return true;
    }
}
