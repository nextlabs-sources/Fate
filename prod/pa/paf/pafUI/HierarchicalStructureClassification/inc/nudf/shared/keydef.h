

#ifndef __NUDF_SHARE_KEY_DEFINES_H__
#define __NUDF_SHARE_KEY_DEFINES_H__


#ifdef __cplusplus
extern "C" {
#endif

//
//  Defines & Enums
//
    
enum _NXRMALGORITHM {
    NXRM_ALGORITHM_NONE      = 0,    /**< No algorithm (No encrypted) */
    NXRM_ALGORITHM_AES128    = 1,    /**< AES 128 bits */
    NXRM_ALGORITHM_AES256    = 2,    /**< AES 256 bits (Default content encryption algorithm) */
    NXRM_ALGORITHM_RSA1024   = 3,    /**< RSA 1024 bits */
    NXRM_ALGORITHM_RSA2048   = 4,    /**< RSA 2048 bits */
    NXRM_ALGORITHM_SHA1      = 5,    /**< SHA1 (Default hash algorithm) */
    NXRM_ALGORITHM_SHA256    = 6,    /**< SHA256 */
    NXRM_ALGORITHM_MD5       = 7     /**< MD5 */
};

#define NXRM_CEKEY_MAXLEN   32
#define NXRM_KEYID_MAXLEN   60

//
//  Declare Structs
//

#pragma pack(push, 8)

typedef struct _NXRM_KEY_ID {
	unsigned long   Algorithm : 16;         /**< Algorithm */
	unsigned long   IdSize : 16;            /**< Id size */	
	unsigned char   Id[NXRM_KEYID_MAXLEN];  /**< Id */
}NXRM_KEY_ID, *PNXRM_KEY_ID;
typedef const NXRM_KEY_ID* PCNXRM_KEY_ID;

typedef struct _NXRM_KEY_BLOB {	
	NXRM_KEY_ID		KeKeyId;                    /**< KEK Id */	
	unsigned char   Key[NXRM_CEKEY_MAXLEN];     /**< KEY */
}NXRM_KEY_BLOB, *PNXRM_KEY_BLOB;
typedef const NXRM_KEY_BLOB* PCNXRM_KEY_BLOB;

#pragma pack(pop)


#ifdef __cplusplus
}
#endif


#endif  // #ifndef __NUDF_SHARE_KEY_DEFINES_H__