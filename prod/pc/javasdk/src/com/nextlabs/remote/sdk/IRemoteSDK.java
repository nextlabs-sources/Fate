/*
 * Created on May 29, 2012
 *
 * All sources, binaries and HTML pages (C) copyright 2012 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.remote.sdk;

import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.List;

import com.nextlabs.destiny.sdk.CEApplication;
import com.nextlabs.destiny.sdk.CEAttributes;
import com.nextlabs.destiny.sdk.CEEnforcement;
import com.nextlabs.destiny.sdk.CENamedAttributes;
import com.nextlabs.destiny.sdk.CEResource;
import com.nextlabs.destiny.sdk.CERequest;
import com.nextlabs.destiny.sdk.CESdkException;
import com.nextlabs.destiny.sdk.CESdkTimeoutException;
import com.nextlabs.destiny.sdk.CEUser;

public interface IRemoteSDK extends Remote {
    public static final String JAVA_SDK = "java sdk";
    
    @Deprecated
    public CEEnforcement CheckResources(long handle, String action,
                                        CEResource source, CEAttributes sourceAttrs, CEResource dest,
                                        CEAttributes destAttrs, CEUser user, CEAttributes userAttrs,
                                        CEApplication app, CEAttributes appAttrs, String[] recipients,
                                        int ipAddress, boolean performObligations, int noiseLevel,
                                        int timeout) throws RemoteException, CESdkTimeoutException, CESdkException;
    
    public CEEnforcement checkResources(String action, CEResource source, CEAttributes sourceAttrs, CEResource dest,
                                        CEAttributes destAttrs, CEUser user, CEAttributes userAttrs,
                                        CEApplication app, CEAttributes appAttrs, CENamedAttributes[] additionalAttrs, String[] recipients,
                                        int ipAddress, boolean performObligations, int noiseLevel,
                                        int timeout) throws RemoteException, CESdkTimeoutException, CESdkException;
    
    public List<CEEnforcement> checkResources(List<CERequest> requests,
                                              String additionalPql, boolean ignoreBuiltinPolicies,
                                              int ipAddress,
                                              int timeout) throws RemoteException, CESdkTimeoutException, CESdkException;

    @Deprecated
    void LogObligationData(long handle, String logIdentifier, String obligationName, CEAttributes attributes)
        throws RemoteException, CESdkTimeoutException, CESdkException;
    
    void logObligationData(String logIdentifier, String obligationName, CEAttributes attributes)
        throws RemoteException, CESdkTimeoutException, CESdkException;

}
