

#ifndef __NUDF_SHARE_FLT_DEFINES_H__
#define __NUDF_SHARE_FLT_DEFINES_H__


#ifdef __cplusplus
extern "C" {
#endif

//
//  Macros & Enums
//
  
#define NXRM_EA_ENCRYPT_CONTENT					"NXRM_ENCRYPT_CONTENT"
#define NXRM_EA_IS_CONTENT_ENCRYPTED			"NXRM_IS_CONTENT_ENCRYPTED"
#define NXRM_EA_CHECK_RIGHTS					"NXRM_CHECK_RIGHTS"
#define NXRM_EA_CHECK_RIGHTS_NONECACHE			"NXRM_CHECK_RIGHTS_NONECACHE"
#define NXRM_EA_SYNC_HEADER						"NXRM_SYNC_HEADER"
#define NXRM_EA_TAG								"NXRM_NXL_TAG"
#define NXRM_EA_SET_SOURCE						"NXRM_SET_SOURCE"

#define NXRM_CONTENT_IS_ENCRYPTED				0x01
#define NXRM_CONTENT_IS_NOT_ENCRYPTED			0x02

//
//  Declare Structs
//

#pragma pack(push, 8)



#pragma pack(pop)


#ifdef __cplusplus
}
#endif


#endif  // #ifndef __NUDF_SHARE_FLT_DEFINES_H__