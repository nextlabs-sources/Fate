/*
 * Created on Feb 17, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice.impl;

import java.io.UnsupportedEncodingException;

/**
 * @author hchan
 * @version $Id$
 */

public class CodecHelper {
    private static final String DEFAULT_CHARSET = "UTF-8";
    
    public static byte[] toBytes(String s){
        if (s == null) {
            return null;
        }
        try {
            return s.getBytes(DEFAULT_CHARSET);
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }
    
    public static String toString(byte[] b){
        if (b == null) {
            return null;
        }
        try {
            return new String(b, DEFAULT_CHARSET);
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }
    
    
    
    
    
    private static final char[] ENCODE_TABLE = new char[]{
        '0', '1', '2', '3', '4',
        '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E',
        'F',
    };
    
    private static final byte[] DECODE_TABLE = new byte[]{
        0,  1,  2,  3,  4,
        5,  6,  7,  8,  9,
        0,  0,  0,  0,  0,
        0,  0,  10, 11, 12,
        13, 14, 15, 16,
    };
    
    public static String base16Encode(byte[] input){
        StringBuilder sb = new StringBuilder();
        
        for(byte b: input){
            // base 16 encoding
            sb.append(ENCODE_TABLE[0x0F & (b >> 4)])
              .append(ENCODE_TABLE[0x0F & b]);
        }
        
        return sb.toString().toLowerCase();
    }
    
    public static byte[] base16Decode(String input){
        char[] chars = input.toUpperCase().toCharArray();
        if (chars.length % 2 != 0) {
            throw new IllegalArgumentException("Length should be multiple of 2");
        }
        
        byte[] original = new byte[chars.length / 2];
        for (int i = 0; i < chars.length; i += 2) {
            // base 16 decoding
            byte b = (byte)( (DECODE_TABLE[chars[i] - '0'] << 4) |  (DECODE_TABLE[chars[i+1] - '0']));
            original[i/2] = b;
        }
        return original;
    }
}
