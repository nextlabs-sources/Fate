/******************************************************************************************
 *
 * NextLabs System Encryption (NLSE) Data Types
 *
 ******************************************************************************************/

#ifndef __SYSTEM_ENCRYPTION_TYPES_H__
#define __SYSTEM_ENCRYPTION_TYPES_H__

#define NLFSE_ADS_SUFFIX                    L":nlse_stream"
#define NLFSE_ADS_FULL_NAME                 L":nlse_stream:$DATA"
#define NLSE_KEY_RING_LOCAL                 "NL_LOCAL"
#define NLSE_KEY_RING_SHARING               "NL_SHARING"
#define NLSE_STREAM_NAME                    "NLSE"
#define NLSE_KEY_LENGTH_IN_BYTES            16  /* 128 bit */
#define NLSE_PADDING_DATA_LEN               16  /* 128 bit */
#define NLSE_KEY_ID_LENGTH_IN_BYTES         32  /* 256 bit */
#define NLSE_KEY_RING_NAME_MAX_LEN          16  /* 16 bytes */
#define NLFSE_ADS_FULL_NAME_LENGTH_IN_BYTES 36

/** NextLabsFile_Header_t
 *
 *  \brief NextLabs File Type
 */
typedef struct
{
  unsigned char version_major;	/* Version Major */
  unsigned char version_minor;	/* Minor Version */
  unsigned char reserved[2];	/* Reserved */
  unsigned int  header_size;	/* Header Size (bytes) */
  unsigned int stream_count; 	/* Stream Count */
  unsigned char cookie[8];	/* Cookie(fixed), always 'NextLabs' */
} NextLabsFile_Header_t;

/** NextLabsFile_StreamHeader_t
 *
 *  \brief NextLabs File stream header.
 */
typedef struct
{
  unsigned int  stream_size;	/* Size (bytes) */
  unsigned char stream_name[8];	/* Stream Name */
} NextLabsFile_StreamHeader_t;

/** SystemEncryption_KeyID_t
 *
 *  \brief Policy Controller key ID
 */
typedef struct
{
  unsigned char hash[NLSE_KEY_ID_LENGTH_IN_BYTES]; /* SHA1 hash */
  ULONG         timestamp;                         /* creation time */
} SystemEncryption_KeyID_t;

/** SystemEncryption_Header_t
 *
 *  \brief Extension written to the end of encrypted file.
 */
typedef struct
{ 
  NextLabsFile_StreamHeader_t   sh;                                         /* stream header */
  unsigned char                 version_major;	                            /* Major version */
  unsigned char                 version_minor;	                            /* Minor version */
  unsigned char                 reserved1[2];	                            /* Reserved 1 */
  char                          pcKeyRingName[NLSE_KEY_RING_NAME_MAX_LEN];  /* key ring */
  SystemEncryption_KeyID_t      pcKeyID;                                    /* key ID */
  unsigned __int64              fileRealLength;                             /* Real length of file */
  unsigned char                 key[NLSE_KEY_LENGTH_IN_BYTES];              /* AES key */
  char                          reserve[16];                                /* reserve field */
  ULONG                         paddingLen;                                 /* padding length */
  unsigned char                 paddingData[NLSE_PADDING_DATA_LEN];         /* padding data */
} SystemEncryptionFile_Header_t;

#endif /*  __SYSTEM_ENCRYPTION_TYPES_H__ */
