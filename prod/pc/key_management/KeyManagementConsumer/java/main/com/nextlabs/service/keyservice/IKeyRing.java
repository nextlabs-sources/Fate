/*
 * Created on Feb 4, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;

import java.util.Collection;

/**
 * @author hchan
 * @version $Id$
 */

public interface IKeyRing {
    public static final long NEVER_UPDATED_BY_SERVER = -1;

    String getName();

    long getLastServerUpdateTime();

    void setLastServerUpdateTime(long update) throws KeyServiceException;

    boolean managedByServer();
    
    void setManagedByServer(boolean flag) throws KeyServiceException;

    IKey getKey(IKeyId keyId) throws KeyServiceException;

    Collection<IKey> getKeys() throws KeyServiceException;

    void setKey(IKey key) throws KeyServiceException;

    void deleteKey(IKeyId keyId) throws KeyServiceException;
    
    /**
     * 
     * @param length in bits
     * @return
     * @throws KeyServiceException
     */
    byte[] generateKey(int length) throws KeyServiceException;
    
    void close() throws KeyServiceException;
    
    boolean isClosed();
    
    /**
     * return true if the password is same
     * @param password
     * @return
     */
    boolean checkPassword(IPassword password) throws KeyServiceException;
}
