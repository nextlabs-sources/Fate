/*
 * Created on Feb 10, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;
/**
 * @author hchan
 * @version $Id$
 */

public class InvalidKeyException extends KeyServiceException {
    
    public InvalidKeyException(String message, int errorCode) {
        super(message, KeyServiceException.KEY | errorCode);
    }

    public InvalidKeyException(Throwable cause, int errorCode) {
        super(cause, KeyServiceException.KEY | errorCode);
    }

    public InvalidKeyException(String message, Throwable cause, int errorCode) {
        super(message, cause, KeyServiceException.KEY | errorCode);
    }

}
