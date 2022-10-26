/*
 * Created on May 14, 2014
 *
 * All sources, binaries and HTML pages (C) copyright 2014 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 *
 * @author amorgan
 * @version $Id$:
 */

package com.nextlabs.destiny.sdk;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * An attribute is a key/value pair.  CENamedAttributes is a group of attributes with an associated
 * name (variously called a context or dimension).
 */

public class CENamedAttributes extends CEAttributes implements Serializable
{
    public static final long serialVersionUID = 1L;

    private final String name;

    public CENamedAttributes(String name) {
        super();
        this.name = name;
    }

    public CENamedAttributes(String name, String[] keyValues) {
        super(keyValues);
        this.name = name;
    }

    /**
     * @return the name associated with these CEAttributes
     */
    public String getName() {
        return name;
    }
    
    @Override
    public boolean equals(Object arg) {
        if (this == arg) return true;
        if (!(arg instanceof CENamedAttributes)) return false;
        CENamedAttributes that = (CENamedAttributes)arg;
        return (name.equals(that.name) && super.equals(arg));
    }
    
    @Override
    public int hashCode() {
        final int prime = 59;
        return prime + (name == null ? 0 : name.hashCode()) + super.hashCode();
    }
    
    private void writeObject(ObjectOutputStream stream) throws IOException {
        stream.defaultWriteObject();
    }
    
    private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
        stream.defaultReadObject();
    }
}
