/*
 * Created on Feb 17, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice.impl;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.nextlabs.service.keyservice.IKeyRingManager;
import com.nextlabs.service.keyservice.KeyServiceException;
import com.nextlabs.service.keyservice.impl.jceks.KeyRingManagerKeyStoreImpl;

/**
 * @author hchan
 * @version $Id$
 */

public class KeyRingManagerFactory {
    private static final Log LOG = LogFactory.getLog(KeyRingManagerFactory.class);

    
    private static class CacheEntry{
        final Constructor<? extends IKeyRingManager> constructor;
        
        IKeyRingManager instance;
        
        CacheEntry(Constructor<? extends IKeyRingManager> constructor) {
            this.constructor = constructor;
        }
        
        IKeyRingManager getInstance(){
            if(instance == null || instance.isClosed()){
                try {
                    instance = constructor.newInstance();
                    
                    //should not throw exceptions since it is already tested during registration.
                } catch (InstantiationException e) {
                    throw new RuntimeException(e);
                } catch (IllegalAccessException e) {
                    throw new RuntimeException(e);
                } catch (IllegalArgumentException e) {
                    throw new RuntimeException(e);
                } catch (InvocationTargetException e) {
                    throw new RuntimeException(e);
                }
            }
            return instance;
        }
    }
    
    private static Map<String, CacheEntry> MANAGER_MAP =
            new HashMap<String, CacheEntry>();

    /**
     * 
     * @param name case-insensitive
     * @return
     * @throws KeyServiceException 
     */
    public static IKeyRingManager getManager(String name) {
        name = name.toUpperCase();
        CacheEntry e = MANAGER_MAP.get(name);
        if (e == null) {
            return null;
        }
        return e.getInstance();
    }

    static void register(String name, Class<? extends IKeyRingManager> clazz) throws KeyServiceException {
        name = name.toUpperCase();
        if (MANAGER_MAP.containsKey(name)) {
            throw new KeyServiceException("Manager is already defined with name '" + name + "'."
                    , KeyServiceException.KEY_RING_MANGER | KeyServiceException.ALREADY_EXIST);
        }

        Constructor<? extends IKeyRingManager> constructor;
        try {
            constructor = clazz.getDeclaredConstructor();
            constructor.setAccessible(true);
        } catch (SecurityException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING_MANGER | KeyServiceException.OTHER);
        } catch (NoSuchMethodException e) {
            throw new KeyServiceException(e, KeyServiceException.KEY_RING_MANGER | KeyServiceException.OTHER);
        }
        MANAGER_MAP.put(name, new CacheEntry(constructor));
    }
    
    static{
        try {
            register(KeyRingManagerKeyStoreImpl.NAME, KeyRingManagerKeyStoreImpl.class);
        } catch (KeyServiceException e) {
            LOG.error("Can't register KeyRingManager with name '" + KeyRingManagerKeyStoreImpl.class + "'.", e);
        }
    }
}
