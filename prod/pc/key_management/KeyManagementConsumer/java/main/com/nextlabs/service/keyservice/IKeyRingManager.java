/*
 * Created on Feb 5, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;

import java.io.File;
import java.util.Collection;

/**
 * @author hchan
 * @version $Id$
 */

public interface IKeyRingManager {

    /**
     * 
     * @param file
     * @param password
     * @param create true will create the storage if not found
     * @return an opened good keyStore.
     * @throws KeyServiceException if KeyStoreException, NoSuchAlgorithmException, CertificateException, IOException
     */
    IKeyRing openKeyRing(String name, File file, IPassword password, boolean create)
            throws KeyServiceException;

    /**
     * if the key ring doesn't exist, do nothing
     * @param name
     * @param file
     * @throws KeyServiceException
     */
    void deleteKeyRing(String name, File file) throws KeyServiceException;

    /**
     * only return opened keyrings
     * @return
     * @throws KeyServiceException
     */
    Collection<IKeyRing> getKeyRings() throws KeyServiceException;
    
    String getName();
    
    void close() throws KeyServiceException;
    
    boolean isClosed();
}