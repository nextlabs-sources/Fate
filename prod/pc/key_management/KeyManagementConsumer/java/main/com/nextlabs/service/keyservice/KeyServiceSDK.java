/*
 * Created on Oct 23, 2013
 *
 * All sources, binaries and HTML pages (C) copyright 2013 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 *
 * @author amorgan
 * @version $Id$:
 */
package com.nextlabs.service.keyservice;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.rmi.AccessException;
import java.rmi.AlreadyBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.RMIClientSocketFactory;
import java.rmi.server.RMIServerSocketFactory;
import java.rmi.server.RMISocketFactory;
import java.rmi.server.UnicastRemoteObject;
import java.util.Properties;

import javax.rmi.ssl.SslRMIClientSocketFactory;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.bluejungle.framework.crypt.ReversibleEncryptor;

public class KeyServiceSDK extends UnicastRemoteObject implements IKeyServiceSDK {
    private static Properties serviceProperties = null;
    private static final Log log = LogFactory.getLog(KeyServiceSDK.class);
    private KeyService keyService;

    private KeyServiceSDK(KeyService keyService,
                          String keyStoreFileName, String keyStorePassword,
                          String trustStoreFileName, String trustStorePassword, int preferredResponsePort) throws RemoteException {
        super(0, new KeyServiceClientSocketFactoryPluggable(),
              new KeyServiceServerSocketFactory(keyStoreFileName, keyStorePassword, trustStoreFileName, trustStorePassword, preferredResponsePort));
        this.keyService = keyService;

        init();
    }

    public static IKeyServiceSDK createKeyServiceSDK(KeyService keyService) {
        ReversibleEncryptor decryptor = new ReversibleEncryptor();

        String keyStoreFileName = getProperty("keystore", null);
        String keyStorePassword = decryptor.decrypt(getProperty("keypass", null));
        String trustStoreFileName = getProperty("truststore", null);
        String trustStorePassword = decryptor.decrypt(getProperty("trustpass", null));

        if (keyStoreFileName == null || keyStorePassword == null || trustStoreFileName == null || trustStorePassword == null) {
            log.info("One or more of keystore, keypass, truststore, trustpass not defined in properties file.\nJava Key API disabled");
            return null;
        }
        
        int preferredResponsePort = parseIntWithDefault(getProperty("preferred_response_port", "0"), 0);

        try {
            return new KeyServiceSDK(keyService, keyStoreFileName, keyStorePassword, trustStoreFileName, trustStorePassword, preferredResponsePort);
        } catch (RemoteException e) {
            log.error("Error when creating key service Java SDK\nJava Key API disabled\n" + e);
        }
        return null;
    }

    private void init() {
        // Find and connect to RMI server. This code is taken from
        // RemoteSDKServer.java. Ideally we should share it.
        
        Registry registry = null;
        
        try {
            // Read the RMI Port configuration from properties file.
            int rmiPortNumber = parseIntWithDefault(getProperty("rmi_registry_port", "" + IKeyServiceSDK.DEFAULT_PORT), IKeyServiceSDK.DEFAULT_PORT);

            log.debug("Locating registry at port " + rmiPortNumber);
            registry = LocateRegistry.getRegistry(rmiPortNumber);
            
            // Check if registry is already exist, else create new.
            try {
                // Don't worry about .list() result. Call this method to know
                // registry's state.
                registry.list();
            } catch (RemoteException re) {
                log.info("Creating registry");
                try {
                    registry = LocateRegistry.createRegistry(rmiPortNumber);
                } catch (RemoteException re1) {
                    log.error("Key Management SDK RemoteException. " + re1);
                    throw new RuntimeException("Key Management SDK - Registry creation failed. ", re1);
                }
            }
            
            log.info("Binding");
            registry.bind(KEY_MANAGEMENT_SDK, this);
        } catch (AlreadyBoundException abe) {
            log.warn("KeyServiceSDK object already bound", abe);
            try {
                log.info("Re-binding");
                registry.rebind(KEY_MANAGEMENT_SDK, this);
            }  catch (AccessException e) {
                throw new RuntimeException("Access exception while re-binding", abe);
            }  catch (RemoteException e) {
                throw new RuntimeException("Remote exception while re-binding", abe);
            }
        } catch (RemoteException re) {
            log.error("KeyServiceSDK - Remote Exception", re);
            throw new RuntimeException(re);
        }
    }
                          
    public IKey getKey(String keyRingName, IKeyId keyId) throws KeyServiceException {
        return keyService.openKeyRing(keyRingName).getKey(keyId);
    }

    /*
     * Reads the given property from JavaSDKService.properties file. If property
     * not found, returns the given default value.
     *
     * Taken from JavaSDK code. This is all pretty bogus. We should pass in properties by default
     * to all plug-ins that want them.
     */
    private static String getProperty(String name, String defaultValue) {
        if (serviceProperties == null) {
            // In DPC environment, this SDK plugin need to resolve paths based on DPC Install Home directory.
            String dpcInstallHome = System.getProperty("dpc.install.home");
            if(dpcInstallHome == null || dpcInstallHome.trim().length() < 1){
                dpcInstallHome = ".";
            }
            
            File file = new File(dpcInstallHome + "/jservice/config/KeyManagementService.properties");
            serviceProperties = new Properties();
            try {
                serviceProperties.load(new FileInputStream(file));
            } catch (FileNotFoundException e) {
                log.warn("KeyServiceSDK - Service properties file not found. " + file.getAbsolutePath());
            } catch (IOException e) {
                log.warn("KeyServiceSDK - Service properties cannot be opened. " + file.getAbsolutePath());
            }
        }
        return serviceProperties.getProperty(name, defaultValue);
    }

    private static int parseIntWithDefault(String i, int defaultValue) {
        try {
            return Integer.parseInt(i);
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }

}
