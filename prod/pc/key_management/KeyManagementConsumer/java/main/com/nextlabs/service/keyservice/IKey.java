/*
 * Created on Feb 10, 2010
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

public interface IKey extends IKeyId{
    public IKeyId getKeyId();
    
    public int getStructureVersion();
    
    public byte[] getEncoded();
}
