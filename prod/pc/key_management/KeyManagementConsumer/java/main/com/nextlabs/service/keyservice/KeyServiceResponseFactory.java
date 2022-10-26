/*
 * Created on Mar 2, 2010
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2010 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */
package com.nextlabs.service.keyservice;

import java.security.GeneralSecurityException;
import java.util.Collection;

import javax.crypto.Cipher;

import com.bluejungle.pf.domain.destiny.serviceprovider.ExternalSPResponseFactory;
import com.bluejungle.pf.domain.destiny.serviceprovider.IDynamicExternalSPResponse;
import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProviderResponse;
import com.bluejungle.pf.domain.destiny.serviceprovider.INextlabsExternalSPResponseCode;
import com.nextlabs.service.keyservice.impl.CodecHelper;
import com.nextlabs.service.keyservice.impl.KeyId;

/**
 * @author hchan
 * @version $Id$
 */

class KeyServiceResponseFactory extends ExternalSPResponseFactory implements
        INextlabsExternalSPResponseCode {
    static final int HASH_LENGTH = 32;
    static final int KEY_LENGTH = 32; //must be multiple of 16 because of the AES encryption
    
    IExternalServiceProviderResponse fail(int errorCode, Throwable error) {
        return create(errorCode, error.toString());
    }

    IExternalServiceProviderResponse fail(KeyServiceException serviceException) {
        int errorCode = serviceException.getErrorCode();
        int resultCode;
        if ((errorCode & KeyServiceException.VALUE_MASK) == KeyServiceException.ALREADY_EXIST) {
            resultCode = CE_RESULT_ALREADY_EXISTS;
        // } else if ((errorCode & KeyServiceException.VALUE_MASK) == KeyServiceException.DOES_NOT_EXIST) {
        //     resultCode = CE_RESULT_    
        } else {
            resultCode = CE_RESULT_GENERAL_FAILED;
        }
        return fail(resultCode, serviceException);
    }
    
    IExternalServiceProviderResponse fail(Throwable error) {
        return fail(CE_RESULT_GENERAL_FAILED, error);
    }
    
    IDynamicExternalSPResponse ok(){
        return create(CE_RESULT_SUCCESS);
    }
    
    IExternalServiceProviderResponse convertKeyRings(Collection<String> keyRingNames) {
        IDynamicExternalSPResponse r = create(CE_RESULT_SUCCESS);
        r.add(keyRingNames.size());
        for (String keyRing : keyRingNames) {
            r.add(keyRing);
        }
        return r;
    }
    
    IKeyId revertToKeyId(String hashBase16, int timestampInS) throws KeyServiceException{
        byte[] hash = CodecHelper.base16Decode(hashBase16);
        if (hash.length != HASH_LENGTH) {
            throw new KeyServiceException("The length of the hash must be " + HASH_LENGTH
                    + ". But it is " + hash.length + ".", KeyServiceException.KEY | KeyServiceException.INVALID_FORMAT);
        }
        final long ts = timestampInS * 1000L;
        
        return new KeyId(hash, ts);
    }
    
    IExternalServiceProviderResponse convert(IKeyId keyId) {
        IDynamicExternalSPResponse r = ok();
        Object[] objs = convertKeyId(keyId);
        for (Object obj : objs) {
            r.add(obj);
        }
        return r;
    }
    
    Object[] convertKeyId(IKeyId keyId){
        Object[] o = new Object[2];
        o[0] = CodecHelper.base16Encode(ArrayHelper.pad(keyId.getId(), HASH_LENGTH));
        o[1] = (int) (keyId.getCreationTimeStamp() / 1000);
        return o;
    }
    
    IExternalServiceProviderResponse convertKeyIds(Collection<? extends IKeyId> keyIds) {
        IDynamicExternalSPResponse r = ok();
        r.add(keyIds.size());
        for (IKeyId keyId : keyIds) {
            Object[] objs = convertKeyId(keyId);
            for (Object obj : objs) {
                r.add(obj);
            }
        }
        return r;
    }
    
    IExternalServiceProviderResponse convert(IKey key, Cipher encryptor) throws KeyServiceException {
        IDynamicExternalSPResponse r = ok();
        r.add(key.getStructureVersion());
        r.add(CodecHelper.base16Encode(ArrayHelper.pad(key.getId(), HASH_LENGTH)));
        r.add((int) (key.getCreationTimeStamp() / 1000));
        byte[] keyValue = key.getEncoded();
        int keyLength = keyValue.length;
        try{
            r.add(CodecHelper.base16Encode(encryptor.doFinal(ArrayHelper.pad(keyValue, KEY_LENGTH))));
        } catch (GeneralSecurityException e) {
            throw new KeyServiceException(e, KeyServiceException.INTERNAL | KeyServiceException.UNKNOWN);
        }
        r.add(keyLength);
        return r;
    }
}
