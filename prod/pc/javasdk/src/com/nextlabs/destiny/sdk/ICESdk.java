/*
 * Created on May 29, 2012
 *
 * All sources, binaries and HTML pages (C) copyright 2012 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.destiny.sdk;

import com.nextlabs.destiny.sdk.CESdkException;

/**
 * @author amorgan
 * @version: $Id: //depot/main/Destiny/icesdk.java
 */


public interface ICESdk
{
    /** Return values from sdk functions */
    public static final int CE_RESULT_SUCCESS = 0;
    public static final int CE_RESULT_GENERAL_FAILED = -1;
    public static final int CE_RESULT_CONN_FAILED = -2;
    public static final int CE_RESULT_INVALID_PARAMS = -3;
    public static final int CE_RESULT_VERSION_MISMATCH = -4;
    public static final int CE_RESULT_FILE_NOT_PROTECTED = -5;
    public static final int CE_RESULT_INVALID_PROCESS = -6;
    public static final int CE_RESULT_INVALID_COMBINATION = -7;
    public static final int CE_RESULT_PERMISSION_DENIED = -8;
    public static final int CE_RESULT_FILE_NOT_FOUND = -9;
    public static final int CE_RESULT_FUNCTION_NOT_AVAILBLE = -10;
    public static final int CE_RESULT_TIMEDOUT = -11;
    public static final int CE_RESULT_SHUTDOWN_FAILED = -12;
    public static final int CE_RESULT_INVALID_ACTION_ENUM = -13;
    public static final int CE_RESULT_EMPTY_SOURCE = -14;
    public static final int CE_RESULT_MISSING_MODIFIED_DATE = -15;
    public static final int CE_RESULT_NULL_CEHANDLE = -16;
    public static final int CE_RESULT_INVALID_EVAL_ACTION = -17;
    public static final int CE_RESULT_EMPTY_SOURCE_ATTR = -18;
    public static final int CE_RESULT_EMPTY_ATTR_KEY = -19;
    public static final int CE_RESULT_EMPTY_ATTR_VALUE = -20;
    public static final int CE_RESULT_EMPTY_PORTAL_USER = -21;
    public static final int CE_RESULT_EMPTY_PORTAL_USERID = -22;
    public static final int CE_RESULT_MISSING_TARGET = -23;
    public static final int CE_RESULT_PROTECTION_OBJECT_NOT_FOUND = -24;
    public static final int CE_RESULT_NOT_SUPPORTED = -25;

    /** Noise levels */
    public static final int CE_NOISE_LEVEL_MIN = 0;
    public static final int CE_NOISE_LEVEL_SYSTEM = 1;
    public static final int CE_NOISE_LEVEL_APPLICATION = 2;
    public static final int CE_NOISE_LEVEL_USER_ACTION = 3;
    public static final int CE_NOISE_LEVEL_MAX = 4;
    
    /**
     * Timeout Wait Forever
     */
    public static int CE_WAIT_FOREVER = -1;

    /**
     * @param app the application
     * @param user the user
     * @param hostName the name of the host machine.  Uses default port. 
     * @param timeout Unused.
     * @return handle Used in subsequent calls to the server
     * @deprecated
     */
    @Deprecated
    public long Initialize(CEApplication app,
                               CEUser user,
                               String hostName,
                               int timeout) throws CESdkException;
    
    
    /** Initialize a connection to the server
     * @param app the application
     * @param user the user
     * @param hostName the name of the host machine
     * @param portNumber the port to connect to.
     * @param timeout Unused.
     * @return handle Used in subsequent calls to the server.
     * @deprecated
     */
    @Deprecated
    public long Initialize(CEApplication app,
                               CEUser user,
                               String hostName,
                               int portNumber,
                               int timeout) throws CESdkException;
    
    /**
     * Close the connection created in Initialize call
     * @param handle The connection handle returned by Initialize
     * @param timeout Unused.
     * @throws CESdkException
     * @deprecated
     */
    @Deprecated
    public void Close(long handle, int timeout) throws CESdkException;

    /**
     * CheckResources
     * @param handle The connection handle returned by Initialize
     * @param action The action to check
     * @param source The source
     * @param sourceAttrs The source's attributes
     * @param dest The destination
     * @param destAttrs The destination's attributes
     * @param user The user Name
     * @param userAttrs The user's attributes. i.e. The SID of the user.
     * @param app The application Name.
     * @param appAttrs The application's attributes. i.e. The full path to the Executable.
     * @param recipients The recipients of the action.
     * @param ipAddress IPAddress of host. If 0, LocalHost is assumed.
     * @param performObligations should obligations be performed as a result of this query?
     * @param noiseLevel One of: CE_NOISE_LEVEL_MIN, CE_NOISE_LEVEL_SYSTEM, CE_NOISE_LEVEL_APPLICATION, CE_NOISE_LEVEL_USER_ACTION 
     * @param timeout In MilliSeconds. CE_WAIT_FOREVER (-1) will not timeout. Must be > 0, if it is not value of WAIT_FOREVER.
     * @return CEEnforcement
     * @throws CESdkException  The cause of the exception is set and can be checked for RemoteException in case the server is not working.
     * @deprecated
     */
    @Deprecated
    public CEEnforcement CheckResources(long handle,
                                        String action,
                                        CEResource source, CEAttributes sourceAttrs,
                                        CEResource dest, CEAttributes destAttrs,
                                        CEUser user, CEAttributes userAttrs,
                                        CEApplication app, CEAttributes appAttrs,
                                        String[] recipients,
                                        int ipAddress,
                                        boolean performObligations,
                                        int noiseLevel,
                                        int timeout) throws CESdkException;

    /**
     * CheckResources
     * @param action The action to check
     * @param source The source
     * @param sourceAttrs The source's attributes
     * @param dest The destination
     * @param destAttrs The destination's attributes
     * @param user The user Name
     * @param userAttrs The user's attributes. i.e. The SID of the user.
     * @param app The application Name.
     * @param appAttrs The application's attributes. i.e. The full path to the Executable.
     * @param additionalAttrs Any additional attribute sets
     * @param recipients The recipients of the action.
     * @param ipAddress IPAddress of host. If 0, LocalHost is assumed.
     * @param performObligations should obligations be performed as a result of this query?
     * @param noiseLevel One of: CE_NOISE_LEVEL_MIN, CE_NOISE_LEVEL_SYSTEM, CE_NOISE_LEVEL_APPLICATION, CE_NOISE_LEVEL_USER_ACTION 
     * @param timeout In MilliSeconds. CE_WAIT_FOREVER (-1) will not timeout. Must be > 0, if it is not value of WAIT_FOREVER.
     * @return CEEnforcement
     * @throws CESdkException  The cause of the exception is set and can be checked for RemoteException in case the server is not working.
     */
    public CEEnforcement checkResources(String action,
                                        CEResource source, CEAttributes sourceAttrs,
                                        CEResource dest, CEAttributes destAttrs,
                                        CEUser user, CEAttributes userAttrs,
                                        CEApplication app, CEAttributes appAttrs,
                                        CENamedAttributes[] additionalAttrs,
                                        String[] recipients,
                                        int ipAddress,
                                        boolean performObligations,
                                        int noiseLevel,
                                        int timeout) throws CESdkException;
    /**
     * LogObligationData
     * @param handle The connection handle returned by Initialize
     * @param logIdentifier the log id (usually returned in the obligation) of the policy activity log
     * @param obligationName  the name of the Policy Assistant (e.g. "Encryption Assistant")
     * @param attributes Assistant Options.  There are three required values.
     * Options, Description, and UserAction and can be created in the following way:
     * <pre>
     * {@code}String [] attributesArray = 
     *     { "options", "aOptions",
	 *       "desc", "aDescription",
	 *       "actions", "aUserActions" };
	 * CEAttributes attributes = 
	 *       new CEAttributes(attributesArray);
	 * </pre>
     * @throws CESdkException  The cause of the exception is set and can be checked for RemoteException in case the server is not working.
     * @deprecated
     */
    @Deprecated
    public void LogObligationData(long handle,
                                  String logIdentifier,
                                  String obligationName,
                                  CEAttributes attributes) throws CESdkException;
    
    /**
     * logObligationData
     * @param logIdentifier the log id (usually returned in the obligation) of the policy activity log
     * @param obligationName  the name of the Policy Assistant (e.g. "Encryption Assistant")
     * @param attributes Assistant Options.  There are three required values.
     * Options, Description, and UserAction and can be created in the following way:
     * <pre>
     * {@code}String [] attributesArray = 
     *     { "options", "aOptions",
	 *       "desc", "aDescription",
	 *       "actions", "aUserActions" };
	 * CEAttributes attributes = 
	 *       new CEAttributes(attributesArray);
	 * </pre>
     * @throws CESdkException  The cause of the exception is set and can be checked for RemoteException in case the server is not working.
     */
    public void logObligationData(String logIdentifier,
                                  String obligationName,
                                  CEAttributes attributes) throws CESdkException;
    /**
     * Convert an InetAddress to an integer representation required by CheckResources.
     * 
     * @param dottedNotation
     * @return Integer representation of IP Address
     * @throws CESdkException
     */
    public int IPAddressToInteger(String dottedNotation) throws CESdkException;
    
}
