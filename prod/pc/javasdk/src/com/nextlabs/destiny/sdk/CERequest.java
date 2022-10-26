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
import java.util.Arrays;

public class CERequest implements Serializable {
    public static final long serialVersionUID = 1L;

    private final String action;
    private final CEResource source;
    private final CEAttributes sourceAttrs;
    private final CEResource dest;
    private final CEAttributes destAttrs;
    private final CEUser user;
    private final CEAttributes userAttrs;
    private final CEApplication app;
    private final CEAttributes appAttrs;
    private final String[] recipients;
    private final CENamedAttributes[] additionalAttrs;
    private final boolean performObligations;
    private final int noiseLevel;

    public CERequest(String action,
                     CEResource source, CEAttributes sourceAttrs,
                     CEResource dest, CEAttributes destAttrs,
                     CEUser user, CEAttributes userAttrs,
                     CEApplication app, CEAttributes appAttrs,
                     String[] recipients,
                     CENamedAttributes[] additionalAttrs,
                     boolean performObligations,
                     int noiseLevel) {
        this.action = action;
        this.source = source;
        this.sourceAttrs = sourceAttrs;
        this.dest = dest;
        this.destAttrs = destAttrs;
        this.user = user;
        this.userAttrs = userAttrs;
        this.app = app;
        this.appAttrs = appAttrs;
        this.recipients = recipients;
        this.additionalAttrs = additionalAttrs;
        this.performObligations = performObligations;
        this.noiseLevel = noiseLevel;
    }

    public String getAction() {
        return action;
    }

    public CEResource getSource() {
        return source;
    }

    public CEAttributes getSourceAttributes() {
        return sourceAttrs;
    }

    public CEResource getDest() {
        return dest;
    }

    public CEAttributes getDestAttributes() {
        return destAttrs;
    }

    public CEUser getUser() {
        return user;
    }

    public CEAttributes getUserAttributes() {
        return userAttrs;
    }

    public CEApplication getApplication() {
        return app;
    }

    public CEAttributes getApplicationAttributes(){
        return appAttrs;
    }

    public String[] getRecipients() {
        return recipients;
    }

    public CENamedAttributes[] getAdditionalAttributes() {
        return additionalAttrs;
    }

    public boolean getPerformObligations() {
        return performObligations;
    }

    public int getNoiseLevel() {
        return noiseLevel;
    }

    @Override
    public boolean equals(Object arg) {
        if (this == arg) {
            return true;
        }

        if (!(arg instanceof CERequest)) {
            return false;
        }

        CERequest that = (CERequest)arg;

        return (this.action.equals(that.action) &&
                this.source.equals(that.source) && attrsEqual(this.sourceAttrs, that.sourceAttrs) &&
                ((this.dest == null && that.dest == null) || (this.dest != null && that.dest != null && this.dest.equals(that.dest) && attrsEqual(this.destAttrs, that.destAttrs))) &&
                this.user.equals(that.user) && attrsEqual(this.userAttrs, that.userAttrs) &&
                this.user.equals(that.user) && attrsEqual(this.userAttrs, that.userAttrs) &&
                this.app.equals(that.app) && attrsEqual(this.appAttrs, that.appAttrs) &&
                Arrays.equals(this.recipients, that.recipients) &&
                Arrays.equals(this.additionalAttrs, that.additionalAttrs) &&
                (this.performObligations == that.performObligations) &&
                (this.noiseLevel== that.noiseLevel));
                
        
    }

    @Override
    public int hashCode() {
        final int prime = 53;
        int result = prime + action.hashCode();
        result = prime * result + source.hashCode();
        result = prime * result + (sourceAttrs == null ? 0 : sourceAttrs.hashCode());
        result = prime * result + (dest == null ? 0 : dest.hashCode());
        result = prime * result + (destAttrs == null ? 0 : destAttrs.hashCode());
        result = prime * result + (user == null ? 0 : user.hashCode());
        result = prime * result + (userAttrs == null ? 0 : userAttrs.hashCode());
        result = prime * result + (app == null ? 0 : app.hashCode());
        result = prime * result + (appAttrs == null ? 0 : appAttrs.hashCode());
        result = prime * result + (recipients == null ? 0 : recipients.hashCode());
        result = prime * result + (additionalAttrs == null ? 0 : additionalAttrs.hashCode());
        result = prime * result + (performObligations ? 1 : 0);
        result = prime * result + noiseLevel;

        return result;
    }

    private boolean attrsEqual(CEAttributes first, CEAttributes second) {
        return ((first != null && second != null && first.equals(second)) || (first == null && second == null));
    }

}
