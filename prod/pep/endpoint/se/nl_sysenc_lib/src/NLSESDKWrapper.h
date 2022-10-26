#ifndef __NLSE_SDK_WRAPPER_H__
#define __NLSE_SDK_WRAPPER_H__

#include "NLSECommon.h"
#include "cesdk.h"

/** CESDKKeyManagementAPI
 *
 *  Load the SDK libraries and functions for use.
 */
CEResult_t CESDKKeyManagementAPI(NLSE_MESSAGE *inMsg,
				 NLSE_MESSAGE *outMsg);

/** CESDKMakeProcessTrusted
 *
 *  \brief Make the process a trusted process.
 *
 *  \param password (in)      Policy Controller administrator password
 *  \return CE_RESULT_SUCCESS if okay
 */
CEResult_t CESDKMakeProcessTrusted(const wchar_t *password);

/** CESDKDrmGetPaths
 *
 *  \brief Get the list of DRM paths from PC Service.
 *
 *  \param out_paths (out)    blob of DRM paths concatenated together
 *  \param out_path_size (out) # of bytes in out_paths
 *
 *  \return CE_RESULT_SUCCESS if okay
 */
CEResult_t CESDKDrmGetPaths(wchar_t **out_paths, int *out_paths_size);
CEResult_t CESDKDrmGetFwPaths(wchar_t **out_paths, int *out_paths_size);

/** CESDKDrmAddPath
 *
 *  \brief Tell the PC Service to add the path to the DRM path list.
 *
 *  \param in_paths (in)      path to add
 *
 *  \return CE_RESULT_SUCCESS if okay
 */
CEResult_t CESDKDrmAddPath(const wchar_t *in_path);
CEResult_t CESDKDrmAddFwPath(const wchar_t *in_path);

/** CESDKDrmRemovePath
 *
 *  \brief Tell the PC Service to remove the path from the DRM path list.
 *
 *  \param in_paths (in)      path to remove
 *
 *  \return CE_RESULT_SUCCESS if okay
 */
CEResult_t CESDKDrmRemovePath(const wchar_t *in_path);
CEResult_t CESDKDrmRemoveFwPath(const wchar_t *in_path);

#endif
