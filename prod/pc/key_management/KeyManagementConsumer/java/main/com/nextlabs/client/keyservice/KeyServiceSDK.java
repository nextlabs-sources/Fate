/*
 * Created on Oct 24, 2013
 *
 * All sources, binaries and HTML pages (C) copyright 2013 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 *
 * @author amorgan
 * @version $Id$:
 */

package com.nextlabs.client.keyservice;

import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import javax.rmi.ssl.SslRMIClientSocketFactory;
import com.nextlabs.service.keyservice.IKey;
import com.nextlabs.service.keyservice.IKeyServiceSDK;
import com.nextlabs.service.keyservice.KeyServiceException;
import com.nextlabs.service.keyservice.impl.KeyId;
import com.nextlabs.service.keyservice.KeyServiceClientSocketFactoryPluggable;

public class KeyServiceSDK {
    final static byte[] LATEST_KEY_HASH = { 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0 };
    final IKeyServiceSDK sdk;


    public KeyServiceSDK(String hostname, String keyStoreName, String keyStorePassword, String trustStoreName, String trustStorePassword) throws KeyServiceSDKException {
        this(hostname, keyStoreName, keyStorePassword, trustStoreName, trustStorePassword, IKeyServiceSDK.DEFAULT_PORT);
    }

    public KeyServiceSDK(String hostname, String keyStoreName, String keyStorePassword, String trustStoreName, String trustStorePassword, int portNumber) throws KeyServiceSDKException {
        try {
            Registry registry = LocateRegistry.getRegistry(hostname, portNumber);

            KeyServiceClientSocketFactoryPluggable.SOCKET_FACTORY.set(new KeyServiceClientSocketFactory(keyStoreName, keyStorePassword, trustStoreName, trustStorePassword));
            sdk = (IKeyServiceSDK)registry.lookup(IKeyServiceSDK.KEY_MANAGEMENT_SDK);
        } catch (NotBoundException nbe) {
            throw new KeyServiceSDKException(nbe);
        } catch (RemoteException re) {
            throw new KeyServiceSDKException(re);
        }

        
    }

    public IKey getKey(byte[] password, String keyRingName) throws KeyServiceSDKException {
        return getKey(password, keyRingName, LATEST_KEY_HASH, 0);
    }

    public IKey getKey(byte[] password, String keyRingName, byte[] hash, long timestamp) throws KeyServiceSDKException {
        try {
            return sdk.getKey(keyRingName, new KeyId(hash, timestamp));
        } catch (KeyServiceException kse) {
            throw new KeyServiceSDKException(kse);
        } catch (RemoteException re) {
            throw new KeyServiceSDKException(re);
        }
    }

    /*
    public static void main(String[] args) {
        final byte[] password = new byte[] {7, -117, 34, -79, -74, 85, -10, -63, -99, -120, 103, 15, -48, -46, -8, -88};

        try {
            KeyServiceSDK sdk = new KeyServiceSDK("127.0.0.1",
                                                  "C:/Program Files/NextLabs/Policy Controller/jservice/KeyManagement/jar/agent-keystore.jks",
                                                  "password",
                                                  "C:/Program Files/NextLabs/Policy Controller/jservice/KeyManagement/jar/agent-truststore.jks",
                                                  "password");
            IKey key = sdk.getKey(password, "NL_LOCAL");
            System.out.println("Successfully got key from NL_LOCAL");
        } catch (KeyServiceSDKException e) {
            System.out.println("Unable to get key from NL_LOCAL");
            System.out.println("Exception: " + e);
        }
    }
    */
}
