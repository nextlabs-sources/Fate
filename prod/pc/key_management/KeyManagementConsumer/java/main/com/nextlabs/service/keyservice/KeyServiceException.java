/*
 * Created on Feb 4, 2010
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

public class KeyServiceException extends Exception {
    public static int AREA_MASK         = 0x0F000000;
    public static int KEY_RING          = 0x01000000;
    public static int KEY               = 0x02000000;
    public static int KEY_RING_MANGER   = 0x03000000;
    public static int KEY_SERVICE       = 0x04000000;
    public static int JAVA_SDK          = 0x05000000;
    
    public static int CACHE_MASK        = 0x10000000;
    public static int CACHE             = 0x10000000;
    
    public static int INTERNAL_MASK     = 0x10000000;
    public static int INTERNAL          = 0x20000000;
    
    public static int VALUE_MASK        = 0x00FFFFFF;
    public static int ALREADY_EXIST     = 0x00000001;
    public static int DOES_NOT_EXIST    = 0x00000002;
    public static int VALUE_NOT_SET     = 0x00000003;
    public static int INVALID_FORMAT    = 0x00000004;
    public static int INVALID_STATE     = 0x00000005;
    public static int INVALID_VALUE     = 0x00000006;
    public static int UNKNOWN           = 0x00FFFFFF;
    public static int OTHER             = 0x007FFFFF;
    
    private final int errorCode;

    public KeyServiceException(String message, Throwable cause, int errorCode) {
        super(message, cause);
        this.errorCode = errorCode;
    }

    public KeyServiceException(String message, int errorCode) {
        super(message);
        this.errorCode = errorCode;
    }

    public KeyServiceException(Throwable cause, int errorCode) {
        super(cause);
        this.errorCode = errorCode;
    }
    
    /**
     * 
     * @return errorCode or null if no errorCode
     */
    public int getErrorCode() {
        return errorCode;
    }
}
