/*
 * Created on Feb 10, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;

import java.io.Serializable;

/**
 * @author hchan
 * @version $Id$
 */

public interface IKeyId extends Serializable{
    long NO_CREATION_TIMESTAMP = 0;
    
    byte[] getId();
    
    long getCreationTimeStamp();
}
