/*========================CEPrivate.h=======================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2007 by Next Labs,    *
 * Sam Mateo CA, Ownership remains with Next Labs Inc,                      *
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/31/2007                                                       *
 * Note   : This file includes the declarations of the exported APIs of     *
 *          CEPrivate module.                                               *
 *==========================================================================*/
#ifndef __CE_PRIVATE_H
#define __CE_PRIVATE_H

#include "CEsdk.h"

/* ------------------------------------------------------------------------
 * CEResult_t CEP_StopPDP(CEHandle handle, CEString password);
 *
 * Stop the PDP. 
 * 
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * password (INPUT): password string to stop PDP
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEP_StopPDP(CEHandle handle, 
		       CEString password,
		       CEint32 timeout_in_seconds);

/* ------------------------------------------------------------------------
 * CEResult_t CEP_GetChallenge(CEHandle handle, CEString challenge)
 *
 * Get the PDP challenge. 
 * 
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * challenge (OUTPUT): returned challenge
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEP_GetChallenge(CEHandle handle, 
			    CEString challenge,
			    CEint32 timeout_in_seconds);
#endif 
