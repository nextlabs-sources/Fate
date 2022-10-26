/*
 * Created on Mar 16, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;

import java.util.Arrays;

/**
 * @author hchan
 * @version $Id$
 */

public class ArrayHelper {
    public static byte[] trim(byte[] input, int length) {
        if (input.length > length) {
            byte[] firstXbytes = new byte[length];
            System.arraycopy(input, 0, firstXbytes, 0, length);
            return firstXbytes;
        }
        return input;
    }
    
    public static byte[] pad(byte[] input, int length) {
        return pad(input, length, (byte) 0);
    }
    
    public static byte[] pad(byte[] input, int length, byte filler) {
        if (input.length < length) {
            byte[] output = new byte[length];
            Arrays.fill(output, filler);
            System.arraycopy(input, 0, output, 0, input.length);
            return output;
        }
        return input;
    }
}
