/*
 * Created on Oct 30, 2013
 *
 * All sources, binaries and HTML pages (C) copyright 2013 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 *
 * @author amorgan
 * @version $Id$:
 */

package com.nextlabs.client.keyservice;

public class KeyServiceSDKException extends Exception {
    public KeyServiceSDKException(String message, Throwable cause) {
        super(message, cause);
    }

    public KeyServiceSDKException(String message) {
        super(message);
    }

    public KeyServiceSDKException(Throwable cause) {
        super(cause);
    }
}
