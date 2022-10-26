/*
 * Created on May 29, 2012
 *
 * All sources, binaries and HTML pages (C) copyright 2012 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.remote.sdk;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.rmi.AccessException;
import java.rmi.AlreadyBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Properties;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.bluejungle.destiny.agent.pdpapi.IPDPApplication;
import com.bluejungle.destiny.agent.pdpapi.IPDPEnforcement;
import com.bluejungle.destiny.agent.pdpapi.IPDPHost;
import com.bluejungle.destiny.agent.pdpapi.IPDPNamedAttributes;
import com.bluejungle.destiny.agent.pdpapi.IPDPResource;
import com.bluejungle.destiny.agent.pdpapi.IPDPSDKCallback;
import com.bluejungle.destiny.agent.pdpapi.IPDPUser;
import com.bluejungle.destiny.agent.pdpapi.PDPApplication;
import com.bluejungle.destiny.agent.pdpapi.PDPException;
import com.bluejungle.destiny.agent.pdpapi.PDPHost;
import com.bluejungle.destiny.agent.pdpapi.PDPNamedAttributes;
import com.bluejungle.destiny.agent.pdpapi.PDPResource;
import com.bluejungle.destiny.agent.pdpapi.PDPSDK;
import com.bluejungle.destiny.agent.pdpapi.PDPTimeout;
import com.bluejungle.destiny.agent.pdpapi.PDPUser;
import com.bluejungle.pf.domain.destiny.serviceprovider.IServiceProvider;
import com.nextlabs.destiny.sdk.CEApplication;
import com.nextlabs.destiny.sdk.CEAttributes;
import com.nextlabs.destiny.sdk.CEAttributes.CEAttribute;
import com.nextlabs.destiny.sdk.CEEnforcement;
import com.nextlabs.destiny.sdk.CENamedAttributes;
import com.nextlabs.destiny.sdk.CEResource;
import com.nextlabs.destiny.sdk.CERequest;
import com.nextlabs.destiny.sdk.CESdkException;
import com.nextlabs.destiny.sdk.CESdkTimeoutException;
import com.nextlabs.destiny.sdk.CEUser;

/**
 * @author dwashburn
 * @version: $Id:
 */

public class RemoteSDKServer implements IRemoteSDK, IServiceProvider {
    private final Log log = LogFactory.getLog(RemoteSDKServer.class);

    private static final IPDPHost LOCALHOST = new PDPHost(0x7F000001); // 127.0.0.1

    private Properties serviceProperties = null;

    public RemoteSDKServer() {
    }

    public static final void main(String[] args) {
        RemoteSDKServer sdkImpl = new RemoteSDKServer();
        sdkImpl.init();
    }

    /*
     * Service Provider initialization.
     * 
     * @see
     * com.bluejungle.pf.domain.destiny.serviceprovider.IServiceProvider#init()
     */
    public void init() {
        Registry registry = null;
        IRemoteSDK stub = null;
        try {

            // Read the RMI Port configuration from properties file.
            String rmiRegistryPort = getProperty("rmi_registry_port", "1099");
            int rmiPortNumber = 1099;
            try {
                rmiPortNumber = Integer.parseInt(rmiRegistryPort);
            } catch (NumberFormatException e) { 
                log.warn("Unable to parse rmi_registry_port property value=" + 
                         rmiRegistryPort + ".");
            };
            log.info("Service Property rmiRegistryPort: " + rmiRegistryPort);

            stub = (IRemoteSDK) UnicastRemoteObject.exportObject(this, rmiPortNumber);
            log.info("Locating registry");
            registry = LocateRegistry.getRegistry(rmiPortNumber);

            // Check if registry is already exist, else create new.
            try {
                // Don't worry about .list() result. Call this method to know
                // registry's state.
                registry.list();
            } catch (RemoteException re) {
                log.info("Creating registry");
                try {
                    registry = LocateRegistry.createRegistry(Integer
                                                             .parseInt(rmiRegistryPort));
                } catch (RemoteException re1) {
                    log.error("Java SDK RemoteException. " + re1);
                    throw new RuntimeException(
                        "JavaSDK - Registry creation failed. ", re1);
                }
            }

            log.info("Binding");
            registry.bind(JAVA_SDK, stub);
        } catch (AlreadyBoundException abe) {
            log.warn("JavaSDK - Object already bound." + abe);
            // Attempt to rebind it.
            try {
                log.info("Re-Binding");
                registry.rebind(JAVA_SDK, stub);
            } catch (AccessException av) {
                throw new RuntimeException(
                    "JavaSDK - AccessException while re-binding. ", av);
            } catch (RemoteException re1) {
                throw new RuntimeException(
                    "JavaSDK - RemoteException while re-binding. ", re1);
            }
        } catch (RemoteException re) {
            log.error("JavaSDK RemoteException. " + re);
            throw new RuntimeException("JavaSDK - RemoteException. ", re);
        }

        log.info("Remote Java SDK ready");
    }

    /*
     * (non-Javadoc)
     * 
     * @see com.nextlabs.remote.sdk.IRemoteSDK#CheckResources(long,
     * java.lang.String, com.nextlabs.destiny.sdk.CEResource,
     * com.nextlabs.destiny.sdk.CEAttributes,
     * com.nextlabs.destiny.sdk.CEResource,
     * com.nextlabs.destiny.sdk.CEAttributes, com.nextlabs.destiny.sdk.CEUser,
     * com.nextlabs.destiny.sdk.CEAttributes,
     * com.nextlabs.destiny.sdk.CEApplication,
     * com.nextlabs.destiny.sdk.CEAttributes, java.lang.String[], int, boolean,
     * int, int)
     * @deprecated
     */
    @Deprecated
    public CEEnforcement CheckResources(long handle, String action,
                                        CEResource source, CEAttributes sourceAttrs, CEResource dest,
                                        CEAttributes destAttrs, CEUser user, CEAttributes userAttrs,
                                        CEApplication app, CEAttributes appAttrs, String[] recipients,
                                        int ipAddress, boolean performObligations, int noiseLevel,
                                        int timeout) throws RemoteException, CESdkTimeoutException,
                                                            CESdkException {
        return checkResources(action,
                              source, sourceAttrs,
                              dest, destAttrs,
                              user, userAttrs,
                              app, appAttrs,
                              null,
                              recipients,
                              ipAddress, performObligations, noiseLevel, timeout);
    }

    private IPDPNamedAttributes[] buildAdditionalData(String[] recipients, CENamedAttributes[] additionalAttrs) {
        int additionalDataLength = 0;

        if (recipients != null && recipients.length > 0) {
            additionalDataLength++;
        }

        if (additionalAttrs != null) {
            additionalDataLength += additionalAttrs.length;
        }

        IPDPNamedAttributes[] additionalData = null;

        if (additionalDataLength > 0) {
            additionalData = new IPDPNamedAttributes[additionalDataLength];

            int index = 0;

            if (additionalAttrs != null) {
                for (CENamedAttributes namedAttrs : additionalAttrs) {
                    additionalData[index] = new PDPNamedAttributes(namedAttrs.getName());
                    for (CEAttribute attr : namedAttrs.getAttributes()) {
                        additionalData[index].setAttribute(attr.getKey(), attr.getValue());
                    }
                    index++;
                }
            }
            if (recipients != null && recipients.length > 0) {
                additionalData[index] = new PDPNamedAttributes("sendto");
                for (String recipient : recipients) {
                    additionalData[index].setAttribute("email", recipient);
                }
            }
        }

        return additionalData;
    }

    public CEEnforcement checkResources(String action,
                                        CEResource source, CEAttributes sourceAttrs,
                                        CEResource dest, CEAttributes destAttrs,
                                        CEUser user, CEAttributes userAttrs,
                                        CEApplication app, CEAttributes appAttrs,
                                        CENamedAttributes[] additionalAttrs,
                                        String[] recipients,
                                        int ipAddress, boolean performObligations, int noiseLevel,
                                        int timeout) throws RemoteException, CESdkTimeoutException, CESdkException {

        return checkResources(action,
                              source, sourceAttrs,
                              dest, destAttrs,
                              user, userAttrs,app, appAttrs,
                              additionalAttrs,
                              recipients,
                              ipAddress, performObligations, noiseLevel,
                              timeout,
                              IPDPSDKCallback.NONE);
    }

    /**
     * This class allows us to wait on the result of each individual callback.
     * It would seem like Future<T> would do this, but the current implementations
     * create a thread to run the action. We put a work unit on a queue to be handled
     * by a thread pool, with the result returned via callback.
     */
    private static class CheckableCallback implements IPDPSDKCallback {
        ArrayBlockingQueue<CEEnforcement> q = new ArrayBlockingQueue<CEEnforcement>(1);

        public void callback(IPDPEnforcement result) {
            q.add(new CEEnforcement(result.getResult(),
                                    new CEAttributes(result.getObligations())));
        }

        public boolean hasResult() {
            return q.peek() != null;
        }

        public CEEnforcement getResult() throws InterruptedException {
            return q.take();
        }

        public CEEnforcement getResult(long timeout, TimeUnit unit) throws InterruptedException {
            return q.poll(timeout, unit);
        }
    }

    public List<CEEnforcement> checkResources(List<CERequest> requests,
                                              String additionalPQL,
                                              boolean ignoreBuiltinPolicies,
                                              int ipAddress,
                                              int timeout) throws RemoteException, CESdkTimeoutException, CESdkException {
        LinkedList<CheckableCallback> callbacks = new LinkedList<CheckableCallback>();

        for (CERequest request : requests) {
            // Join additional PQL into the already existing additional attributes in request
            CENamedAttributes[] additionalAttrs = request.getAdditionalAttributes();
            
            int newLength = (request.getAdditionalAttributes() == null ? 0 : request.getAdditionalAttributes().length) + (additionalPQL != null ? 1 : 0);

            if (newLength != 0) {
                CENamedAttributes origAttrs[] = request.getAdditionalAttributes();
                additionalAttrs = new CENamedAttributes[newLength];

                if (origAttrs != null) {
                    System.arraycopy(origAttrs, 0, additionalAttrs, 0, origAttrs.length);
                }

                if (additionalPQL != null) {
                    CENamedAttributes policies = new CENamedAttributes("policies");
                    policies.add("pql", additionalPQL);
                    policies.add("ignoredefault", ignoreBuiltinPolicies ? "yes" : "no");

                    additionalAttrs[additionalAttrs.length-1] = policies;
                }

            }

            CheckableCallback cb = new CheckableCallback();
            callbacks.add(cb);

            checkResources(request.getAction(),
                           request.getSource(), request.getSourceAttributes(),
                           request.getDest(), request.getDestAttributes(),
                           request.getUser(), request.getUserAttributes(),
                           request.getApplication(), request.getApplicationAttributes(),
                           additionalAttrs,
                           request.getRecipients(),
                           ipAddress,
                           request.getPerformObligations(),
                           request.getNoiseLevel(),
                           timeout, cb);
        }

        /* There's no need to collect the results in the order they finish. We need all of them
         * before the timeout happens, so why not just start at the beginning?
         *
         * If we don't get all the response we give up. We do not return partial results.
         */
        ArrayList<CEEnforcement> results = new ArrayList<CEEnforcement>(requests.size());
        long waitTime = timeout;

        for (CheckableCallback cb : callbacks) {
            log.debug("Wait time is " + waitTime + " milliseconds");

            if (waitTime <= 0) {
                throw new CESdkTimeoutException();
            }

            /* This time is not particularly precise, despite the name. On Windows it is usually some
             * multiple of 16ms (with 0 as an option). It doesn't matter. If you have a 5s timeout then
             * we don't guarantee that it will be *exactly* 5 seconds.
             *
             * More accurate calls are available, but they are slower.
             */
            long start = System.currentTimeMillis();
            try {
                CEEnforcement enf = cb.getResult(waitTime, TimeUnit.MILLISECONDS);

                if (enf == null) {
                    throw new CESdkTimeoutException();
                }

                results.add(enf);
            } catch (InterruptedException e) {
                throw new CESdkTimeoutException(e);
            }
            waitTime -= System.currentTimeMillis() - start;
        }

        return results;
    }

    private CEEnforcement checkResources(String action,
                                         CEResource source, CEAttributes sourceAttrs,
                                         CEResource dest, CEAttributes destAttrs,
                                         CEUser user, CEAttributes userAttrs,
                                         CEApplication app, CEAttributes appAttrs,
                                         CENamedAttributes[] additionalAttrs,
                                         String[] recipients,
                                         int ipAddress, boolean performObligations, int noiseLevel,
                                         int timeout,
                                         IPDPSDKCallback cb) throws RemoteException, CESdkTimeoutException, CESdkException {

        log.info("IRemoteSDK.CheckResources Server Impl called.");
        if (source == null)
            throw new RemoteException("CheckResources source can not be null");

        int resource_count = 1;

        if ((dest != null) && (!(dest.getName().length() == 0))) {
            resource_count++; // We have a target attribute
        }

        IPDPResource[] resources = new IPDPResource[resource_count];

        resources[0] = buildResource("from", source, sourceAttrs);

        if ((dest != null) && (!(dest.getName().length() == 0))) {
            resources[1] = buildResource("to", dest, destAttrs);
        }

        IPDPUser theUser = buildUser(user, userAttrs);
  
        IPDPNamedAttributes[] additionalData = buildAdditionalData(recipients, additionalAttrs);

        IPDPApplication application = buildApplication(app, appAttrs);

        IPDPHost host = buildHost(ipAddress);

        IPDPEnforcement ret = null;
        try {
            ret = PDPSDK.PDPQueryDecisionEngine(action, resources, theUser,
                                                application, host, performObligations,
                                                additionalData, noiseLevel, timeout,
                                                cb);
        } catch (IllegalArgumentException e) {
            throw new RemoteException("PDP query has illegal arguments", e);
        } catch (PDPTimeout e) {
            throw new CESdkTimeoutException("PDP query timedout");
        } catch (PDPException e) {
            throw new CESdkException("PDP query exception");
        }

        // The return value will be null if we have a callback, otherwise it won't
        if (ret != null) {
            return new CEEnforcement(ret.getResult(), new CEAttributes(ret.getObligations()));
        }

        return null;
    }


    /*
     * (non-Javadoc)
     * 
     * @see com.nextlabs.remote.sdk.IRemoteSDK#LogObligationData(long,
     * java.lang.String, java.lang.String,
     * com.nextlabs.destiny.sdk.CEAttributes)
     * @deprecated
     */
    @Deprecated
    public void LogObligationData(long handle, String logIdentifier,
                                  String assistantName, CEAttributes attributes)
        throws RemoteException, CESdkTimeoutException, CESdkException {
    }

    public void logObligationData(String logIdentifier, String assistantName, CEAttributes attributes)
        throws RemoteException, CESdkTimeoutException, CESdkException {
        log.info("IRemoteSDK.LogObligationData Server Impl called.");
        String assistantOptions = null;
        String assistantDescription = null;
        String assistantUserActions = null;

        if (attributes.size() >= 1) {
            assistantOptions = attributes.getAttributes().get(0).getValue();
            if (attributes.size() >= 2) {
                assistantDescription = attributes.getAttributes().get(1)
                                       .getValue();
                if (attributes.size() >= 3)
                    assistantUserActions = attributes.getAttributes().get(2)
                                           .getValue();
            }
        }

        PDPSDK.PDPLogObligationData(logIdentifier, assistantName,
                                    assistantOptions, assistantDescription, assistantUserActions);

    }

    /*
     * Reads the given property from JavaSDKService.properties file. If property
     * not found, returns the given default value.
     */
    private String getProperty(String name, String defaultValue) {
        // In DPC environment, this SDK plugin need to resolve paths based on DPC Install Home directory.
        String dpcInstallHome = System.getProperty("dpc.install.home");
        if(dpcInstallHome == null || dpcInstallHome.trim().length() < 1){
            dpcInstallHome = ".";
        }
        log.debug("DPC Install Home :" + dpcInstallHome);
  
        File file = new File(dpcInstallHome + "/jservice/config/JavaSDKService.properties");
        log.debug("Properties file path " + file.getAbsolutePath());
        if (serviceProperties == null) {
            serviceProperties = new Properties();
            try {
                serviceProperties.load(new FileInputStream(file));
            } catch (FileNotFoundException e) {
                log.warn("JavaSDK - Service properties file not found. "
                         + file.getAbsolutePath());
            } catch (IOException e) {
                log.warn("JavaSDK - Service properties cannot be opened. "
                         + file.getAbsolutePath());
            }
        }
        return serviceProperties.getProperty(name, defaultValue);
    }

    private IPDPResource buildResource(final String dimensionName, final CEResource resource, final CEAttributes resourceAttrs) {
        IPDPResource theResource = new PDPResource(dimensionName, resource.getName(), resource.getType());

        if (resourceAttrs != null) {
            for (CEAttribute attr : resourceAttrs.getAttributes()) {
                theResource.setAttribute(attr.getKey().toLowerCase(), attr.getValue());
            }
        }
        return theResource;
    }

    private IPDPUser buildUser(final CEUser user, final CEAttributes userAttrs) {
        IPDPUser theUser = new PDPUser(user.getId(), user.getName());

        if (userAttrs != null) {
            for (CEAttribute attr : userAttrs.getAttributes()) {
                theUser.setAttribute(attr.getKey(), attr.getValue());
            }
        }

        return theUser;
    }

    private IPDPApplication buildApplication(final CEApplication app, final CEAttributes appAttrs) {
        IPDPApplication application = PDPApplication.NONE;

        if (!(app.getName().length() == 0)) {
            // TODO: do we need an actual PID here?
            application = new PDPApplication(app.getName(), 0L /* PID */);
            if (appAttrs != null) {
                for (CEAttribute attr : appAttrs.getAttributes()) {
                    application.setAttribute(attr.getKey(), attr.getValue());
                }
            }
        }

        return application;
    }

    private IPDPHost buildHost(final int ipAddress) {
        IPDPHost host = LOCALHOST;

        if (ipAddress != 0) {
            host = new PDPHost(ipAddress);
        }

        return host;
    }
}
