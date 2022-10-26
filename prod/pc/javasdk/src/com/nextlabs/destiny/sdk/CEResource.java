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

/**
 * @author amorgan
 * @version: $Id: //depot/main/Destiny/cesdk.java
 */

public class CEResource  implements java.io.Serializable
{
	private static final long serialVersionUID = 1L;
	private String name;
    private String type;

    /**
     * Create a new resource of the specified name and type
     * @param name the resource name (e.g. "c:/foo.txt")
     * @param type the type (e.g. "fso")
     */
    public CEResource(String name, String type)
    {
        this.name = name;
        this.type = type;
    }

    /**
     * Get the name of the resource
     * @return the name
     */
    public String getName()
    {
        return name;
    }

    /**
     * Get the type of the resource
     * @return the type
     */
    public String getType()
    {
        return type;
    }

    /**
     * Convert to an array (used by JNI code)
     * @return an array of two strings, the name and the type
     */
    public String[] toArray()
    {
        String[] res = { name, type };
        return res;
    }
	/*
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object arg) {
		if (this == arg) return true;
		if ( ! (arg instanceof CEResource) ) return false;
		CEResource that = (CEResource) arg;
		return	(this.type.equals(that.type) &&
				 this.name.equals(that.name));
	}

	/* 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((type == null) ? 0 : type.hashCode());
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
