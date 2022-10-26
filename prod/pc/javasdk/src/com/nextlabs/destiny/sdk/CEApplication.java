/*
 * Created on May 29, 2012
 *
 * All sources, binaries and HTML pages (C) copyright 2012 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.destiny.sdk;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;

/**
 * @author amorgan
 * @version: $Id: //depot/main/Destiny/ceapplication.java
 */

public class CEApplication implements Serializable
{
	private static final long serialVersionUID = 1L;
	private String name;
    private String path;
    private String url;

    /**
     * Create a new application of the specified name and location
     * @param name the application name (e.g. "iexplore.exe")
     * @param path the path (e.g. "c:/windows/system32/iexplore.exe")
     */
    public CEApplication(String name, String path)
    {
        this.name = name;
        this.path = path;
        this.url = "";
    }

    /**
     * Create a new application of the specified name and location, referencing
     * a particular URL (e.g. this is IE looking at a webpage)
     * @param name the application name (e.g. "iexplore.exe")
     * @param path the path (e.g. "c:/windows/system32/iexplore.exe")
     * @param url the url (e.g. "http://www.cnn.com")
     */
    public CEApplication(String name, String path, String url)
    {
        this.name = name;
        this.path = path;
        this.url = url;
    }

    /**
     * Get the name of the application
     * @return the name
     */
    public String getName()
    {
        return name;
    }

    /**
     * Get the path of the application
     * @return the path
     */
    public String getPath()
    {
        return path;
    }

    /**
     * Get the url of the application
     * @return the url
     */
    public String getUrl()
    {
        return url;
    }

    /**
     * Convert to an array (used by JNI code)
     * @return an array of three strings, the name, the path, and the url
     */
    public String[] toArray()
    {
        String[] res = { name, path, url};
        return res;
    }
    
	/* Test if two Objects are the same CEUser object.
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object arg) {
		if (this == arg) return true;
		if ( ! (arg instanceof CEApplication) ) return false;
		CEApplication that = (CEApplication) arg;
		return	(this.name.equals(that.name) &&
				 this.path.equals(that.path) &&
				 this.url.equals (that.url ));
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((name == null) ? 0 : name.hashCode());	
		result = prime * result + ((path == null) ? 0 : path.hashCode());
		result = prime * result + ((url == null) ? 0 : url.hashCode());
		return result;
	}
	
	private  void writeObject(ObjectOutputStream stream) 
			throws IOException 
	{	
		stream.defaultWriteObject();
	} 
	
	private  void readObject(ObjectInputStream stream) 
			throws IOException, ClassNotFoundException 
	{	
		stream.defaultReadObject();
	}    
}
