/*
 * Created on Mar 26, 2014
 *
 * All sources, binaries and HTML pages (C) copyright 2014 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 *
 * @author amorgan
 * @version $Id$:
 */
package com.nextlabs.service.keyservice;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.ServerSocket;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLServerSocketFactory;
import javax.rmi.ssl.SslRMIServerSocketFactory;

class KeyServiceServerSocketFactory extends SslRMIServerSocketFactory {
    private String keyStoreFileName = null;
    private char[] keyStorePassword = null;
    private String trustStoreFileName = null;
    private char[] trustStorePassword = null;
    private SSLServerSocketFactory serverSocketFactory = null;
    private int preferredResponsePort = 0;

    public KeyServiceServerSocketFactory() {
        super();
    }

    public KeyServiceServerSocketFactory(String keyStoreFileName, String keyStorePassword, String trustStoreFileName, String trustStorePassword, int preferredResponsePort) {
        super(null, null, true);
        this.keyStoreFileName = keyStoreFileName;
        this.keyStorePassword = keyStorePassword == null ? null : keyStorePassword.toCharArray();
        this.trustStoreFileName = trustStoreFileName;
        this.trustStorePassword = trustStorePassword == null ? null : trustStorePassword.toCharArray();
        this.preferredResponsePort =  preferredResponsePort;
    }

    synchronized private SSLServerSocketFactory getServerSocketFactory() throws KeyServiceException {
        if (serverSocketFactory == null) {
            try {
                KeyManager[] keyManagers = null;

                if (keyStoreFileName != null) {
                    // init keystore
                    KeyStore keyStore = KeyStore.getInstance(KeyStore.getDefaultType());
                    FileInputStream keyStoreFile = new FileInputStream(keyStoreFileName);
                    keyStore.load(keyStoreFile, keyStorePassword);
                    keyStoreFile.close();

                    // Key Manager
                    KeyManagerFactory keyManagerFactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
                    keyManagerFactory.init(keyStore, keyStorePassword);
                    keyManagers = keyManagerFactory.getKeyManagers();
                }

                TrustManager[] trustManagers = null;

                if (trustStoreFileName != null) {
                    // init truststore
                    KeyStore trustStore = KeyStore.getInstance(KeyStore.getDefaultType());
                    FileInputStream trustStoreFile = new FileInputStream(trustStoreFileName);
                    trustStore.load(trustStoreFile, trustStorePassword);
                    trustStoreFile.close();
                    
                    // Trust Manager
                    TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
                    trustManagerFactory.init(trustStore);
                    trustManagers = trustManagerFactory.getTrustManagers();
                }

                SSLContext sslContext = SSLContext.getInstance("SSL");
                sslContext.init(keyManagers, trustManagers, new SecureRandom());

                serverSocketFactory = sslContext.getServerSocketFactory();
            } catch (FileNotFoundException e) {
                throw new KeyServiceException(e, KeyServiceException.JAVA_SDK);
            } catch (IOException e) {
                throw new KeyServiceException(e, KeyServiceException.JAVA_SDK);
            } catch (KeyStoreException e) {
                throw new KeyServiceException(e, KeyServiceException.JAVA_SDK);
            } catch (KeyManagementException e) {
                throw new KeyServiceException(e, KeyServiceException.JAVA_SDK);
            } catch (NoSuchAlgorithmException e) {
                throw new KeyServiceException(e, KeyServiceException.JAVA_SDK);
            } catch (UnrecoverableKeyException e) {
                throw new KeyServiceException(e, KeyServiceException.JAVA_SDK);
            } catch (CertificateException e) {
                throw new KeyServiceException(e, KeyServiceException.JAVA_SDK);
            }
        }

        return serverSocketFactory;
    }

    @Override
    public ServerSocket createServerSocket(int port) throws IOException {
        try {
            return getServerSocketFactory().createServerSocket(port == 0 ? preferredResponsePort : port);
        } catch (KeyServiceException e) {
            throw new IOException(e);
        }
    }
}
