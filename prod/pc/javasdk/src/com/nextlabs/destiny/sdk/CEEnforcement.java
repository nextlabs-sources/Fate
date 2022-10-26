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
 * @version: $Id: //depot/main/Destiny/ceenforcement.java
 */


public class CEEnforcement implements Serializable
{
    private static final long serialVersionUID = 1L;
    
    public enum CEResponse {
        ALLOW("allow", 0),
        DENY("deny", 1),
        DONTCARE("dontcare", 2);

        private final String name;
        private final int index;
        private CEResponse(String name, int index) {
            this.name = name;
            this.index = index;
        }

        public static CEResponse get(final String name) {
            if (name == null) {
                throw new NullPointerException("name cannot be null");
            }

            if (name.equalsIgnoreCase("allow")) {
                return ALLOW;
            } else if (name.equalsIgnoreCase("deny")) {
                return DENY;
            } else if (name.equalsIgnoreCase("dontcare")) {
                return DONTCARE;
            }

            throw new IllegalArgumentException("Unknown response name " + name);
        }

        public String getName() {
            return name;
        }

        public int getIndex() {
            return index;
        }
    };

    private CEResponse response;
    private CEAttributes obligations;

    /**
     * Construct default enforcement object.  ALLOW with no obligations
     */
    public CEEnforcement()
    {
        response = CEResponse.ALLOW;
        obligations = new CEAttributes();
    }

    /**
     * Construct enforcment object with specified response and obligations
     * @param response the response
     * @param obligations the obligations
     */
    public CEEnforcement(CEResponse response, CEAttributes obligations)
    {
        this.response = response;
        if (obligations == null)
        {
            this.obligations = new CEAttributes();
        }
        else
        {
            this.obligations = obligations;
        }
    }

    /**
     * Construct enforcment object with specified response and obligations
     * @param response the response as a string ("allow", "deny", "dontcare")
     * @param obligations the obligations
     */
    public CEEnforcement(String response, CEAttributes obligations) {
        this(CEResponse.get(response), obligations);
    }

    /**
     * Set the response on an existing enforcement object
     * @param response the response (allow or deny)
     */
    public void setResponse(CEResponse response)
    {
        this.response = response;
    }

    /**
     * Set the response on an existing enforcement object
     * @param response the response as a sring ("allow", "deny", or "dontcare")
     */
    public void setResponse(String response)
    {
        this.response = CEResponse.get(response);
    }

    /**
     * Get the response
     * @return the response (allow or deny)
     */
    public CEResponse getResponse()
    {
        return response;
    }

    /**
     * Get the response
     * @return the response (allow or deny)
     */
    public String getResponseAsString()
    {
        return response.getName();
    }

    /**
     * Add a new key/value to the obligations
     * @param key the key
     * @param value the value
     */
    public void addObligation(String key, String value)
    {
        obligations.add(key, value);
    }

    /**
     * Set the obligations
     * @param obligations the obligations
     */
    public void setObligations(CEAttributes obligations)
    {
        if (obligations == null)
        {
            this.obligations = new CEAttributes();
        }
        else
        {
            this.obligations = obligations;
        }
    }
    
    /**
     * Get the obligations
     * @return the obligations
     */
    public CEAttributes getObligations()
    {
        return obligations;
    }
    
    /* Test if two Objects are the same CEEnforcement object.
     * @see java.lang.Object#equals(java.lang.Object)
     */
    @Override
    public boolean equals(Object arg) {
        if (this == arg) return true;
        if ( ! (arg instanceof CEEnforcement) ) return false;
        CEEnforcement that = (CEEnforcement) arg;
        return (this.response == that.response &&
                this.obligations.equals(that.obligations));
    }

    /* (non-Javadoc)
     * @see java.lang.Object#hashCode()
     */
    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + response.getIndex();
        result = prime * result + ((obligations.size() > 0) ? 0 : obligations.hashCode()); 
        return result;
    }
 
    private  void writeObject(ObjectOutputStream stream) 
        throws IOException { 
        stream.defaultWriteObject();
    } 
 
    private  void readObject(ObjectInputStream stream) 
        throws IOException, ClassNotFoundException { 
        stream.defaultReadObject();
    }

}
