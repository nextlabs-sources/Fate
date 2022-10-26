/*
 * Created on Sep 17, 2012
 *
 * All sources, binaries and HTML pages (C) copyright 2012 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.destiny.sdk;

/**
 * Exception for the CE SDK Timeout
 */

public class CESdkTimeoutException extends CESdkException {

	private static final long serialVersionUID = 1L;

	/**
	 * The default constructor
	 */
	public CESdkTimeoutException() {
		super();
	}

	/**
	 * Construct a <code>CESdkTimeoutException</code> with the given message
	 * 
	 * @param message
	 *            the message for the <code>CESdkTimeoutException</code>
	 */
	public CESdkTimeoutException(String message) {
		super(message);
	}

	/**
	 * Construct a <code>CESdkTimeoutException</code> with the given cause
	 * 
	 * @param cause
	 *            the cause for the <code>CESdkTimeoutException</code>
	 */
	public CESdkTimeoutException(Throwable cause) {
		super(cause);
	}

	/**
	 * Construct a <code>CESdkTimeoutException</code> with the given message and
	 * cause
	 * 
	 * @param message
	 *            the message for the <code>CESdkTimeoutException</code>
	 * @param cause
	 *            the cause for the <code>CESdkTimeoutException</code>
	 */
	public CESdkTimeoutException(String message, Throwable cause) {
		super(message, cause);
	}
}
