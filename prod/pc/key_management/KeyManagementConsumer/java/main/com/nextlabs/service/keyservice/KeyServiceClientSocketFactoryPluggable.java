/*
 * Created on Apr 02, 2014
 *
 * All sources, binaries and HTML pages (C) copyright 2014 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 *
 * @author amorgan
 * @version $Id$:
 */
package com.nextlabs.service.keyservice;
    
import java.io.IOException;
import java.net.Socket;
import javax.rmi.ssl.SslRMIClientSocketFactory;

public class KeyServiceClientSocketFactoryPluggable extends SslRMIClientSocketFactory {

    // Idea taken from http://blogger.ziesemer.com/2010/02/jmx-avoid-java-system-properties-rmi.html

    // The client needs to specify a keystore/truststore, but we *don't* want this done through properties
    // because the client might be on an appserver and we don't want to grant permission to everyone.
    // We can do this by providing our own socket factory with keystore/truststore information, but the
    // client socket factory is created here, on the server, and there is no obvious way to access the object
    // when it actually gets to the client. So... how can we set the values?
    //
    // The best (?) answer appears to be through a thread-local static. Static, so that we don't need to access the
    // *object*, and thread-local, so that everyone doesn't get our permissions.
    //
    // That this insane solution is actually a perfectly reasonable and sensible one makes me cry...

    public static final ThreadLocal<SslRMIClientSocketFactory> SOCKET_FACTORY = new InheritableThreadLocal<SslRMIClientSocketFactory>(){
        @Override
        protected SslRMIClientSocketFactory initialValue(){
            return new SslRMIClientSocketFactory();
        }
    };
  
    @Override
    public Socket createSocket(String host, int port) throws IOException{
        return SOCKET_FACTORY.get().createSocket(host, port);
    }
}
