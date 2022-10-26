/*
 * Created on Feb 17, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice.impl;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;

import com.nextlabs.service.keyservice.ArrayHelper;
import com.nextlabs.service.keyservice.IPassword;

import sun.misc.BASE64Encoder;

/**
 * @author  hchan
 * @version  $Id$
 */

public class Password implements IPassword {
    private static final int LIMIT_LENGTH = 16;
    
    private final char[] chars;
    
    public Password(String plaintext){
        MessageDigest digest;
        try {
            digest = MessageDigest.getInstance("SHA-256");
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
        
        // don't call get byte since it is depends on the default encoding
        byte[] hash = digest.digest( CodecHelper.toBytes(plaintext + HardcodedSalt.SALT[0]));
        
        hash = ArrayHelper.trim(hash, LIMIT_LENGTH);
        
        chars = convert(hash);
    }
    
    char[] convert(byte[] bytes){
        return new BASE64Encoder().encode(bytes).toCharArray();
    }
    
    public char[] getChars(){
        return chars;
    }

    public boolean equals(IPassword password) {
        if (password == null) {
            return false;
        }
        return Arrays.equals(chars, password.getChars());
    }
}
