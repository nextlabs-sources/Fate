/*
 * Created on Feb 17, 2010
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

public interface IPassword {
    char[] getChars();
    
    boolean equals(IPassword password);
}
