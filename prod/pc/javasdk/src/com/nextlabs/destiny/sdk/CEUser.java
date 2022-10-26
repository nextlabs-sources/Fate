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
 * @version: $Id: //depot/main/Destiny/ceuser.java
 */

public class CEUser implements Serializable
{
	private static final long serialVersionUID = 1L;
	private String name;
    private String id;
    
    /**
     * Create a new user with the specified name and id
     * @param name the user name (e.g. "c:/foo.txt")
     * @param type the user id (e.g. "fso")
     */
     public CEUser(String name, String id)
    {
    	//We assume this is email address. It is very heuristic though.
    	//Email address has to be converted to lower case
    	// We do this for name and id for users.
        this.name = (name.contains("@"))?name.toLowerCase():name;
        this.id = (id.contains("@"))?id.toLowerCase():id;
    }

    /**
     * Get the name of the user
     * @return the name
     */
    public String getName()
    {
        return name;
    }

    /**
     * Get the id of the user
     * @return the type
     */
    public String getId()
    {
        return id;
    }

    /**
     * Convert to an array (used by JNI code)
     * @return an array of two strings, the name and the id
     */
    public String[] toArray()
    {
        String[] res = { name, id };
        return res;
    }

	/* Test if two Objects are the same CEUser object.
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object arg) {
		if (this == arg) return true;
		if ( ! (arg instanceof CEUser) ) return false;
		CEUser that = (CEUser) arg;
		return	(this.id.equals(that.id) &&
				 this.name.equals(that.name));
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((id == null) ? 0 : id.hashCode());
		result = prime * result + ((name == null) ? 0 : name.hashCode());	
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
