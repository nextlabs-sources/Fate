/*
 * Created on Feb 18, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice.impl;

import java.io.File;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import net.sf.ehcache.CacheException;

import com.nextlabs.service.keyservice.IKeyRing;
import com.nextlabs.service.keyservice.IKeyRingManager;
import com.nextlabs.service.keyservice.IPassword;
import com.nextlabs.service.keyservice.KeyServiceException;

/**
 * @author hchan
 * @version $Id$
 */

public class CachedKeyRingManager implements IKeyRingManager {
    private final IKeyRingManager parent;
    private final boolean usingCachedKeyRing;
    private final Map<String, IKeyRing> map;
    
    public CachedKeyRingManager(IKeyRingManager parent, boolean usingCachedKeyRing) {
        this.parent = parent;
        this.usingCachedKeyRing = usingCachedKeyRing;
        
        map = new HashMap<String, IKeyRing>(1);
        
        //TODO use ehcache when we upgrade to the latest version
//      String name = CachedKeyRingManager.class.getName() + " " + parent.getName();
//      if (cacheManager.cacheExists(name)) {
//          cache = cacheManager.getCache(name);
//      } else {
//          cache = new Cache(name, 
//              1000,   //maximumSize,
//              false,  //overflowToDisk, 
//              false,  //eternal, 
//              0, 
//              0);
//      }
//      cacheManager.addCache(cache);
    }
    
    /**
     * @param parent
     * @throws CacheException
     */
    public CachedKeyRingManager(IKeyRingManager parent) {
        this(parent, true);
    }

    public IKeyRing openKeyRing(String name, File file, IPassword password, boolean create)
            throws KeyServiceException {
        if (parent.isClosed()) {
            //this will throw exception
            parent.openKeyRing(name, file, password, create);
        }

        //TODO use ehcache when we upgrade to the latest version
//      Element exisiting = null;
//      try {
//          exisiting = cache.get(name);
//      } catch (IllegalStateException e1) {
//      } catch (CacheException e1) {
//      }
//
//      if (exisiting != null) {
//          return (IKeyRing) exisiting.getValue();
//      }
        
        IKeyRing keyRing;
        //only look up the cache if not "create"
        if(!create){
            keyRing = map.get(name);
            //the keyRing is found, active and correct password
            if (keyRing != null && !keyRing.isClosed() && keyRing.checkPassword(password)) {
                return keyRing;
            }
        }
        
        keyRing = parent.openKeyRing(name, file, password, create);
        if (usingCachedKeyRing) {
            keyRing = new CacheLatestKeyRing(keyRing);
        }

        //TODO upgrade to the latest ehcache to allow caching non-Serializable object;  
        //      cache.put(new Element(name, keyRing));

        map.put(name, keyRing);

        return keyRing;
    }
    
    public Collection<IKeyRing> getKeyRings() throws KeyServiceException {
        //don't return the cached version since it may not cached all keys
        return parent.getKeyRings();
    }
    
    public void deleteKeyRing(String name, File file) throws KeyServiceException {
        parent.deleteKeyRing(name, file);
        map.remove(name);
    }

    public String getName() {
        return parent.getName();
    }

    public void close() throws KeyServiceException {
        parent.close();
    }

    public boolean isClosed() {
        return parent.isClosed();
    }
}
