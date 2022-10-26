/******************************************************************************************
 *
 * NextLabs Encryption (NLE) Data Types
 *
 ******************************************************************************************/

#ifndef __NEXTLABS_ENCRYPTION_TYPES_H__
#define __NEXTLABS_ENCRYPTION_TYPES_H__

#include "NextLabs_Types.h"

#define NLFSE_ADS_SUFFIX                    L":nlse_stream"
#define NLFSE_ADS_FULL_NAME                 L":nlse_stream:$DATA"

/* the fixed local key ring name */
#define NLE_KEY_RING_LOCAL                  "NL_LOCAL"

/* the default shared key ring name */
#define NLE_KEY_RING_SHARE_DEFAULT          "NL_SHARE_DEFAULT"

#define NLE_KEY_LENGTH_IN_BYTES             16  /* 128 bit */
#define NLE_PADDING_DATA_LEN                16  /* 128 bit */
#define NLE_KEY_ID_LENGTH_IN_BYTES          32  /* 256 bit */
#define NLE_KEY_RING_NAME_MAX_LEN           16  /* 16 bytes */
#define NLFSE_ADS_FULL_NAME_LENGTH_IN_BYTES 36



/** NextLabsEncryption_KeyID_t
 *
 *  \brief Policy Controller key ID
 */
typedef struct
{
  unsigned char hash[NLE_KEY_ID_LENGTH_IN_BYTES];  /* SHA1 hash */
  ULONG         timestamp;                         /* creation time */
} NextLabsEncryption_KeyID_t;



/** NextLabsEncryption_Header_1_0_t
 *
 *  \brief Extension written to the ADS of encrypted file.
 */

#define NLE_STREAM_1_0_NAME                 "NLSE"
#define NLE_FILE_VERSION_1_0_MAJOR          1
#define NLE_FILE_VERSION_1_0_MINOR          0

typedef struct
{ 
  NextLabsFile_StreamHeader_t   sh;                                         /* stream header */
  unsigned char                 version_major;	                            /* Major version */
  unsigned char                 version_minor;	                            /* Minor version */
  unsigned char                 reserved1[2];	                            /* Reserved 1 */
  char                          pcKeyRingName[NLE_KEY_RING_NAME_MAX_LEN];   /* key ring */
  NextLabsEncryption_KeyID_t    pcKeyID;                                    /* key ID */
  unsigned __int64              fileRealLength;                             /* Real length of file */
  unsigned char                 key[NLE_KEY_LENGTH_IN_BYTES];               /* AES key */
  char                          reserve[16];                                /* reserve field */
  ULONG                         paddingLen;                                 /* padding length */
  unsigned char                 paddingData[NLE_PADDING_DATA_LEN];          /* padding data */
} NextLabsEncryptionFile_Header_1_0_t;



/** NextLabsEncryptionFile_Header_t
 *
 *  \brief Extension written to the header of encrypted file.
 */

#define NLE_STREAM_NAME                     "NLE"
#define NLE_FILE_VERSION_MAJOR              2
#define NLE_FILE_VERSION_MINOR              0

/* Flags */
#define NLEF_REQUIRES_LOCAL_ENCRYPTION  0x0000000000000001ui64

typedef struct
{ 
  NextLabsFile_StreamHeader_t   sh;                                         /* Stream header */
  unsigned char                 version_major;	                            /* Major version */
  unsigned char                 version_minor;	                            /* Minor version */
  unsigned char                 reserved1[2];	                            /* Reserved 1 */
  char                          pcKeyRingName[NLE_KEY_RING_NAME_MAX_LEN];   /* key ring */
  NextLabsEncryption_KeyID_t    pcKeyID;                                    /* key ID */
  unsigned char                 unused1[4];                                 /* Unused 1 */
  unsigned __int64              fileRealLength;                             /* Real length of file */
  unsigned char                 key[NLE_KEY_LENGTH_IN_BYTES];               /* AES key */
  unsigned __int64              flags;                                      /* Flags */
  unsigned char                 reserved2[8];                               /* Reserved 2 */
  ULONG                         paddingLen;                                 /* padding length */
  unsigned char                 paddingData[NLE_PADDING_DATA_LEN];          /* padding data */
  unsigned char                 unused2[380];                               /* Unused 2 */
} NextLabsEncryptionFile_Header_t;



#endif /*  __NEXTLABS_ENCRYPTION_TYPES_H__ */
