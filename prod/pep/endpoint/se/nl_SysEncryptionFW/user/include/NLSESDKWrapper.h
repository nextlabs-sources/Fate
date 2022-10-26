#ifndef __NLSE_SDK_WRAPPER_H__
#define __NLSE_SDK_WRAPPER_H__

#include "NLSECommon.h"
#include "cesdk.h"

/** CESDKInit
 *
 *  Load the SDK libraries and functions for use.
 */
CEResult_t CESDKKeyManagementAPI(NLSE_MESSAGE *inMsg,
				 NLSE_MESSAGE *outMsg);
/** CESDKInit
 *
 *  Load the SDK libraries and functions for use.
 */
bool CESDKInit(void);

/** CESDKConnect
 *
 *  \brief Perform sdk connection to policy controller
 *
 *  \param resource_from (in) (Device) resource.
 *  \param user_name (in)     User name for evaluation.
 *  \param user_sid (in)      User SID for evaluation.
 *  \param deny (out)         Result of evaluation.
 *
 *  \return false on error.
 */
bool CESDKConnect();

/** CESDKDisconnect
 *
 *  \brief Perform sdk disconnection
 *
 *  \return false on error.
 */
bool CESDKDisconnect();

/** CESDKCheckFileSimple
 *
 *  \brief Perform a simple check-file SDK evaluation without all the functionality in the full-blown CEEVALUATE_CheckFile().
 *
 *  \return CE_RESULT_SUCCESS on success.
 */
CEResult_t CESDKCheckFileSimple(const WCHAR *sourceFullFileName,
                                const WCHAR *targetFullFileName,
                                CEAction_t action, 
                                const WCHAR *appName,
                                CEResponse_t *pResponse);
#endif
