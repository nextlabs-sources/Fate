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

import java.rmi.Remote;
import java.rmi.RemoteException;

/**
 * Defines the Java SDK for the Key Service
 */
public interface IKeyServiceSDK extends Remote {
    public static final String KEY_MANAGEMENT_SDK = "nextlabs key management";
    
    public static int DEFAULT_PORT = 1099;

    public IKey getKey(String keyRingName, IKeyId keyId) throws RemoteException, KeyServiceException;
}
