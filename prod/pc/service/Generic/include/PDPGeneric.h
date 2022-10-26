/*=========================PDPGeneric.h=====================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2008 by NextLabs, Inc.*
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Note   : This file includes the declarations of the exported APIs of PDP *
 *          service "generic" function call module                          *
 *=========================PDPGeneric.h=====================================*/

#ifndef CE_PDPGENERIC_H
#define CE_PDPGENERIC_H

#include <vector>
#include <jni.h>
#include "CEsdk.h"

/* Just call once per thread */
CEResult_t PDP_GenericCallInit(JavaVM *PDP_jvm);

CEResult_t PDP_GenericCall(JavaVM *PDP_jvm,
                           jobject cmStub,
                           jclass cmStubClass,
                           nlsocket responseSocket,
                           std::vector<void *> &inputArgs);

CEResult_t PDP_GenericResponse(JNIEnv *env,
                               nlsocket jhandle,
                               jstring reqId,
                               jobjectArray response);
#endif
