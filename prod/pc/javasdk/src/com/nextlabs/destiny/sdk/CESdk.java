/*
 * Created on May 29, 2012
 *
 * All sources, binaries and HTML pages (C) copyright 2012 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.destiny.sdk;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.nextlabs.remote.sdk.IRemoteSDK;

/**
 * @author dwashburn
 * @version: $Id: 
 */


/**
 * This class combines an old, deprecated, API and a new one, which leads to a lot of similarly
 * named functions. The old API returned a handle from an Initialize call rather than making the
 * object be a connection. The deprecated methods are:
 *
 *   Initialize (x2)
 *   CheckResources
 *   LogObligationData
 *   Close
 *
 * The new methods replacing them are
 *
 *   the constructors
 *   checkResources
 *   logObligationData
 * 
 * The capitalization has also been changed to the preferred Java standard.
 */
public class CESdk implements ICESdk
{
    private static final int CE_SUCCESS = 0;
    private static long nextMapKey = 1;
    private final IRemoteSDK remoteSDK;
    private static final int DEFAULT_RMI_PORT = 1099;

    private static Map<Long, IRemoteSDK> sdkInstanceMap = new HashMap<Long, IRemoteSDK>();

    @Deprecated
    public CESdk() {
        remoteSDK = null;
    }

    public CESdk(String hostName) throws CESdkException {
        this(hostName, DEFAULT_RMI_PORT);
    }

    public CESdk(String hostName, int portNumber) throws CESdkException {
        remoteSDK = initializeRemoteSDK(hostName, portNumber);
    }

    @Deprecated
    public long Initialize(CEApplication app,
                           CEUser user,
                           String hostName,
                           int timeout) throws CESdkException
    {
        return Initialize(app, user, hostName, 0, timeout);
    }
    
    @Deprecated
    public long Initialize(CEApplication app,
                           CEUser user,
                           String hostName,
                           int portNumber,
                           int timeout) throws CESdkException
    {
        long ret = CE_SUCCESS;

        synchronized (this) {
            IRemoteSDK sdk = initializeRemoteSDK(hostName, portNumber);
        
            long handle = nextMapKey++;

            sdkInstanceMap.put(handle, sdk);

            return handle;
        }
    }

    private IRemoteSDK initializeRemoteSDK(String hostName, int portNumber) throws CESdkException {
        // Setup the SDK instance.
        try {
            synchronized (this) {
                Registry registry = LocateRegistry.getRegistry(hostName, portNumber);
                IRemoteSDK sdk = (IRemoteSDK)registry.lookup(IRemoteSDK.JAVA_SDK);

                return sdk;
            }
        } catch (NotBoundException nbe) {
            throw new CESdkException(nbe);
        } catch (RemoteException re) {
            throw new CESdkException(re);
        }
    }

    public CEEnforcement checkResources(String action,
                                        CEResource    source, CEAttributes sourceAttrs,
                                        CEResource    dest,   CEAttributes destAttrs,
                                        CEUser        user,   CEAttributes userAttrs,
                                        CEApplication app,    CEAttributes appAttrs,
                                        CENamedAttributes[] additionalAttrs,
                                        String[] recipients,
                                        int ipAddress,
                                        boolean performObligations,
                                        int noiseLevel,
                                        int timeout) throws CESdkException
    {
        if (remoteSDK == null) {
            throw new CESdkException("Calling checkResources on an uninitialized object");
        }

        try {
            CEEnforcement enforcement = remoteSDK.checkResources(action,
                                                                 source, sourceAttrs,
                                                                 dest, destAttrs,
                                                                 user, userAttrs,
                                                                 app, appAttrs,
                                                                 additionalAttrs,
                                                                 recipients,
                                                                 ipAddress,
                                                                 performObligations,
                                                                 noiseLevel,
                                                                 timeout);

            if (enforcement != null) {
                return enforcement;
            }
        } catch (RemoteException e) {
            throw new CESdkException("Error: " + CE_RESULT_CONN_FAILED + " Remote Exception.", e);
        }

        throw new CESdkException("Error -1, return from checkResources was null");
    }

    public List<CEEnforcement> checkResources(List<CERequest> requests,
                                              String additionalPQL,
                                              boolean ignoreBuiltinPolicies,
                                              int ipAddress,
                                              int timeout) throws CESdkException {
        if (remoteSDK == null) {
            throw new CESdkException("Calling checkResources on an uninitialized object");
        }

        try {
            List<CEEnforcement> results = remoteSDK.checkResources(requests,
                                                                   additionalPQL,
                                                                   ignoreBuiltinPolicies,
                                                                   ipAddress,
                                                                   timeout);

            if (results != null) {
                return results;
            }
        } catch (RemoteException e) {
            throw new CESdkException("Error: " + CE_RESULT_CONN_FAILED + " Remote Exception.", e);
        }

        throw new CESdkException("Error -1, return from checkResources was null");
    }

    @Deprecated
    public CEEnforcement CheckResources(long handle,
                                        String action,
                                        CEResource    source, CEAttributes sourceAttrs,
                                        CEResource    dest,   CEAttributes destAttrs,
                                        CEUser        user,   CEAttributes userAttrs,
                                        CEApplication app,    CEAttributes appAttrs,
                                        String[] recipients,
                                        int ipAddress,
                                        boolean performObligations,
                                        int noiseLevel,
                                        int timeout) throws CESdkException
    {
      
        IRemoteSDK sdk = null;
        synchronized (this) {
            sdk = sdkInstanceMap.get(handle);
        }
    
        if (sdk == null)
            throw new CESdkException("Error " + CE_RESULT_CONN_FAILED);

        CEEnforcement enforcement = null;
        try {
            enforcement = sdk.CheckResources(handle, action, 
                                             source, sourceAttrs, 
                                             dest, destAttrs, 
                                             user, userAttrs, 
                                             app, appAttrs, 
                                             recipients, 
                                             ipAddress, 
                                             performObligations, 
                                             noiseLevel, 
                                             timeout);
        } catch (RemoteException e) {
            throw new CESdkException("Error: " + CE_RESULT_CONN_FAILED + " Remote Exception.", e);
        }
       
        if (enforcement == null) {
            throw new CESdkException("Error -1, return from CheckResources was null");
        }

        return enforcement;


    }
 
    @Deprecated   
    public void LogObligationData(long handle,
                                  String logIdentifier,
                                  String obligationName,
                                  CEAttributes attributes) throws CESdkException
    {
        IRemoteSDK sdk = null;
        synchronized (this) {
            sdk = sdkInstanceMap.get(handle);
        }
    
        if (sdk == null)
            throw new CESdkException("Error " + CE_RESULT_INVALID_PARAMS);

        try {
            sdk.LogObligationData(handle, logIdentifier, obligationName, attributes);
        } catch (RemoteException e) {
            throw new CESdkException("Error " + CE_RESULT_CONN_FAILED + " Remote Exception.", e);
        }   
    }

    public void logObligationData(String logIdentifier,
                                  String obligationName,
                                  CEAttributes attributes) throws CESdkException
    {
        if (remoteSDK == null) {
            throw new CESdkException("Calling CheckResources on an uninitialized object");
        }

        try {
            remoteSDK.logObligationData(logIdentifier, obligationName, attributes);
        } catch (RemoteException e) {
            throw new CESdkException("Error " + CE_RESULT_CONN_FAILED + " Remote Exception.", e);
        }   
    }

    @Deprecated
    public void Close(long handle, int timeout) throws CESdkException
    {
        IRemoteSDK sdk = null;
        synchronized (this) {
            sdk = sdkInstanceMap.remove(handle);
        }
    
        if (sdk == null)
            throw new CESdkException("Error " + CE_RESULT_INVALID_PARAMS);  
    }

    @Deprecated
    public int IPAddressToInteger(String dottedNotation) throws CESdkException {
        return ipAddressToInteger(dottedNotation);
    }

    public static int ipAddressToInteger(String dottedNotation) throws CESdkException {
        String[] octets = dottedNotation.split("\\.");

        if (octets.length != 4) {
            throw new CESdkException("Invalid IP address: "+ dottedNotation);
        }

        int res = 0;
        for (String octet : octets) {
            try {
                int o = Integer.parseInt(octet);
                if (o < 0 || o > 255) {
                    throw new CESdkException("Invalid IP address: "+ dottedNotation);
                }

                res = (res << 8) + o;
            }
            catch (NumberFormatException nfe) {
                throw new CESdkException("Invalid IP address: " + dottedNotation);
            }
        }

        return res;
    }
}
