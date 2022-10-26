/*========================PEPMan.h=========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/09/2007                                                       *
 * Note   : This file includes the declarations of the exported APIs of PEP *
 *          management module.                                              *
 *==========================================================================*/

#ifndef __CE_PEPMAN_H
#define __CE_PEPMAN_H

#include "CEsdk.h"
#include "nlstrings.h"
#include <vector>

using namespace std;

/* =======================PEPMAN_RPCCall==================================*
 * This function send the request for the client. It will Block the client*
 * thread until the answer arrives or timeout.                            *
 *                                                                        *
 * Parameters:                                                            * 
 * reqID  (input): ID string (in the format of "<tid>+time") of request   *
 * reqStr (input): client's request.                                      *
 * reqLen (input): the length of request                                  *
 * c (input): client's handler                                            *
 * funcName (output): the name of RPC                                     *
 * reqResult (output): RPC returned code                                  *
 * outputs (output): RPC returned parameters                              *
 * timeout_in_milisec (input): desirable timeout of this RPC call         *
 *                                                                        *
 * Return:                                                                *
 *   It returns CERESULT_SUCCESS, if the RPC succeeds.                    *
 * =======================================================================*/
CEResult_t PEPMAN_RPCCall(nlstring &reqID,
			  char *reqStr, 
			  size_t reqLen, 
			  CEHandle c,
			  nlstring &funcName, 
			  CEResult_t &reqResult, 
			  std::vector<void *> &outputs,
			  int timeout_in_millisec); 

/* =======================PEPMAN_Init=====================================*
 * This function does intializations. It includes,                        *
 * 1. Open log file                                                       *
 * 2. Creat the backend receiving thread.                                 *
 * 3. Creat the socket connection.                                        *
 * If the initialization has been done already, increat the count number  *
 * of connected clients.                                                  *
 *                                                                        *
 * Parameters:                                                            *
 * pdpHost (input): the PDP host name                                     *
 * socketFd (output): returned socket descriptor.                         *
 * bJoin (input): if the value is true, the back-end thread will be waited*
 *                for its termination. This variable will be ingored if   *
 *                the back-end thread is already running.                 *
 *                                                                        *
 * Return:                                                                *
 *   It returns CERESULT_SUCCESS, if it succeeds.                         *
 * =======================================================================*/
CEResult_t PEPMAN_Init(CEString pdpHost, nlsocket &socketFd, bool bJoin);

/* =======================PEPMAN_Close====================================*
 * This function will decrese the number of clients by 1.                 *
 * If the number of clients becomes 0, it will do the following,          *
 * 1. Close log file                                                      *
 * 2. Clear the table "requests".                                         *
 *                                                                        *
 * If the variable "bCloseSocket" is true, when the number of clients     *
 * becomes 0, it will                                                     *
 * 1. Close the socket connection.                                        *
 * 2. the backend receiving thread will exit.                             *
 * Return:                                                                *
 *   It returns CE_RESULT_SUCCESS, if it succeeds.                        *
 * =======================================================================*/
CEResult_t PEPMAN_Close(bool bCloseSocket);
/* =======================PEPMAN_IsRunning================================*
 * This function will check if the initialization has been done           *
 * successfully.                                                          *
 *                                                                        *
 * Return:                                                                *
 *   It returns CE_RESULT_SUCCESS, if it PEPMan initalization has been    *
 *   done successfully.                                                   *
 * =======================================================================*/
CEResult_t PEPMAN_IsRunning();

/* =======================PEPMAN_GetUniqueRequestID=======================*
 * This function will return the unique request ID within the process.    *
 *                                                                        *
 * Return:                                                                *
 *   Return the unique request ID within the process                      *
 * =======================================================================*/
unsigned long PEPMAN_GetUniqueRequestID();
#endif
