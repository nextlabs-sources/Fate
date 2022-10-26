
/**
 * \file <rmp/nxlfmt.h>
 * \brief Header file for NextLabs File Format
 *
 * This header file defines NextLabs File Format
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NXL_FORMAT_H__
#define __NXL_FORMAT_H__



#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup nxlfmt NextLabs File Format
 * NextLabs File Format is also called NXL Format, which is defined by Nextlabs, Inc.
 * The NXL Format supports:
 *     - Encryption
 *     - Meta Data
 *     - Extra Attributes
 * @{
 */



/**
 * \defgroup nxlfmt-const Consts
 * CONST values used by NXL Format
 * @{
 */


#define NXL_SIGNATURE_HIGH      '\0!TM'                 /**< NXL Signatrue (HighPart) */
#define NXL_SIGNATURE_LOW       'FLXN'                  /**< NXL Signatrue (LowPart) */
#define NXL_VERSION_10          0x00010000              /**< NXL Format Version 1.0 */


#define NXL_PAGE_SIZE           0x1000                  /**< NXL Format Page Size */
#define NXL_CBC_SIZE            0x200                   /**< NXL Format CBC Size */
#define NXL_MIN_SIZE			0x3000                  /**< NXL Format Minimum File Size */

#define NX_CEK_MAX_LEN			256                     /**< Maximum Content Encrypt Key Size */
#define NX_KEK_MAX_LEN			512                     /**< Maximum Key Encrypt Key Size */

#define MAX_SECTION_COUNT       72                      /**< Maximum section count */

/**
 * \defgroup nxlfmt-const-flag Flags
 * CONST strings used by NXL Format
 * @{
 */

// NXL File Flags
#define NXL_FLAGS_NONE          0x00000000              /**< NXL Format File Flag: None */

// NXL File Crypto Flags
#define NXL_CRYPTO_FLAGS_NONE   0x00000000              /**< NXL Format Crypto Flag: None */

/**@}*/ // Group End: nxlfmt-const-flag

/**
 * \defgroup nxlfmt-const-str Strings
 * CONST strings used by NXL Format
 * @{
 */

#define NXL_DEFAULT_MSG         L"This is a NXL File!"  /**< NXL Format default message */

#define NXL_SECTION_ATTRIBUTES  ".Attrs"                /**< Name of default section "Attributes" */
#define NXL_SECTION_TEMPLATES   ".Rights"               /**< Name of default section "Rights" */
#define NXL_SECTION_TAGS        ".Tags"                 /**< Name of default section "Tags" */

#define NXL_SECTION_ATTRIBUTESW L".Attrs"                /**< Name of default section "Attributes" */
#define NXL_SECTION_TEMPLATESW  L".Rights"               /**< Name of default section "Rights" */
#define NXL_SECTION_TAGSW       L".Tags"                 /**< Name of default section "Tags" */

/**@}*/ // Group End: nxlfmt-const-str


/**@}*/ // Group End: nxlfmt-const


/**
 * \defgroup nxlfmt-enum Enum
 * Enum
 * @{
 */

/**
 *  \enum _NXLALGORITHM
 *  Enum algorithm supported by NXL Format
 */
enum _NXLALGORITHM {
    NXL_ALGORITHM_NONE      = 0,    /**< No algorithm (No encrypted) */
    NXL_ALGORITHM_AES128    = 1,    /**< AES 128 bits */
    NXL_ALGORITHM_AES256    = 2,    /**< AES 256 bits (Default content encryption algorithm) */
    NXL_ALGORITHM_RSA1024   = 3,    /**< RSA 1024 bits */
    NXL_ALGORITHM_RSA2048   = 4,    /**< RSA 2048 bits */
    NXL_ALGORITHM_SHA1      = 5,    /**< SHA1 (Default hash algorithm) */
    NXL_ALGORITHM_SHA256    = 6,    /**< SHA256 */
    NXL_ALGORITHM_MD5       = 7     /**< MD5 */
};

typedef enum _NXLALGORITHM  NXLALGORITHM;   /**< Type of enum _NXLALGORITHM */

#define  NXL_ALGORITHM      NXLALGORITHM    /**< Alias */


/**
 *  \enum _NXLCBCSIZE
 *  Enum CBC size supported by NXL Format
 */
enum _NXLCBCSIZE {
    NXL_CBC_512 = 0x200     /**< CBC size is 512 bytes */
};

typedef enum _NXLCBCSIZE    NXLCBCSIZE;      /**< Type of enum _NXLCBCSIZE */

/**@}*/ // Group End: nxlfmt-enum




//
//  Structures
//

// ****************************************************************************
//  Specifies packing alignment for structure, union, and class members.
// ****************************************************************************
#pragma pack(push)
#pragma pack(8)

/**
 * \defgroup nxlfmt-struct Structs
 * Structures used by NXL Format
 * @{
 */

/**
 * \defgroup nxlfmt-struct-sig Signature Header
 * Signature Header
 * @{
 */


/**
 *  \union _SIGNATURE_CODE
 *  Signature Code Union
 */
union _SIGNATURE_CODE {
    struct {
        unsigned long LowPart;      /**< Low part */
        unsigned long HighPart;     /**< High part */
    };
    struct {
        unsigned long LowPart;      /**< Low part */
        unsigned long HighPart;     /**< High part */
    } u;
    unsigned __int64 QuadPart;      /**< Quad part */
};
typedef union _SIGNATURE_CODE   SIGNATURE_CODE; /**< Type of union _SIGNATURE_CODE */


/**
 * \struct _NXL_SIGNATURE
 * Signature Header
 */
struct _NXL_SIGNATURE {
    SIGNATURE_CODE  Code;           /**< Signature code */
    wchar_t         Message[68];    /**< NXL Message */
};
typedef struct _NXL_SIGNATURE   NXL_SIGNATURE;      /**< Type of struct _NXL_SIGNATURE */
typedef struct _NXL_SIGNATURE*  PNXL_SIGNATURE;     /**< Type of struct _NXL_SIGNATURE pointer */
typedef const NXL_SIGNATURE*    PCNXL_SIGNATURE;    /**< Type of const struct _NXL_SIGNATURE pointer */



/**@}*/ // Group End: nxlfmt-struct-sig



/**
 * \defgroup nxlfmt-struct-bsc Basic Header
 * Basic Header
 * @{
 */

/**
 * \struct _NXL_BASIC_INFORMATION
 * Basic Information Header
 */
struct _NXL_BASIC_INFORMATION {
    unsigned char   Thumbprint[16];     /**< Thumbprint, which is unique for each NXL file */
    unsigned long   Version;            /**< NXL Format version */
    unsigned long   Flags;              /**< NXL file flags */
    unsigned long   Alignment;          /**< NXL file alignment (should be NXL_PAGE_SIZE) */
    unsigned long   PointerOfContent;   /**< The offset of the real content */
};
typedef struct _NXL_BASIC_INFORMATION   NXL_BASIC_INFORMATION;      /**< Type of struct _NXL_BASIC_INFORMATION */
typedef struct _NXL_BASIC_INFORMATION*  PNXL_BASIC_INFORMATION;     /**< Type of struct _NXL_BASIC_INFORMATION pointer */
typedef const NXL_BASIC_INFORMATION*    PCNXL_BASIC_INFORMATION;    /**< Type of const struct _NXL_BASIC_INFORMATION pointer */



/**@}*/ // Group End: nxlfmt-struct-bsc



/**
 * \defgroup nxlfmt-struct-crypto Crypto Header
 * Crypto Header
 * @{
 */



/**
 * \struct _NXL_KEKEY_ID
 * NXL KEK Id Struct (Size is 64 bytes)
 */
struct _NXL_KEKEY_ID {
    unsigned long   Algorithm : 16; /**< Algorithm */
    unsigned long   IdSize : 16;    /**< Id size */
    unsigned char   Id[60];         /**< Id */
};
typedef struct _NXL_KEKEY_ID   NXL_KEKEY_ID;      /**< Type of struct _NXL_KEKEY_ID */
typedef struct _NXL_KEKEY_ID*  PNXL_KEKEY_ID;     /**< Type of struct _NXL_KEKEY_ID pointer */
typedef const NXL_KEKEY_ID*    PCNXL_KEKEY_ID;    /**< Type of const struct _NXL_KEKEY_ID pointer */


/**
 * \struct _NEXTLABS_KEY_ID
 * NextLabs Key Id Struct
 */
struct _NEXTLABS_KEY_ID {
    char            Name[8];    /**< Key-ring Name */
    unsigned char   Hash[32];   /**< Hash of this key */
    int             Timestamp;  /**< Key's creation time */
};
typedef struct _NEXTLABS_KEY_ID   NEXTLABS_KEY_ID;      /**< Type of struct _NEXTLABS_KEY_ID */
typedef struct _NEXTLABS_KEY_ID*  PNEXTLABS_KEY_ID;     /**< Type of struct _NEXTLABS_KEY_ID pointer */
typedef const NEXTLABS_KEY_ID*    PCNEXTLABS_KEY_ID;    /**< Type of const struct _NEXTLABS_KEY_ID pointer */


/**
 * \struct _NXL_KEKEY_BLOB
 * NXL KEK Blob Struct
 */
struct _NXL_KEKEY_BLOB {
    NXL_KEKEY_ID    KeyId;                  /**< Key Id */
    unsigned long   keySize;                /**< Key Size */
    unsigned char   Key[NX_KEK_MAX_LEN];    /**< Key */
};
typedef struct _NXL_KEKEY_BLOB   NXL_KEKEY_BLOB;      /**< Type of struct _NXL_KEKEY_BLOB */
typedef struct _NXL_KEKEY_BLOB*  PNXL_KEKEY_BLOB;     /**< Type of struct _NXL_KEKEY_BLOB pointer */
typedef const NXL_KEKEY_BLOB*    PCNXL_KEKEY_BLOB;    /**< Type of const struct _NXL_KEKEY_BLOB pointer */


/**
 * \struct _NXL_KEY_BLOB
 * NXL Key Blob Struct (Size is 320 bytes)
 */
struct _NXL_KEY_BLOB {
    NXL_KEKEY_ID    KeKeyId;                /**< KEK Id */
    unsigned char   CeKey[NX_CEK_MAX_LEN];  /**< CEK */
};
typedef struct _NXL_KEY_BLOB   NXL_KEY_BLOB;      /**< Type of struct _NXL_KEY_BLOB */
typedef struct _NXL_KEY_BLOB*  PNXL_KEY_BLOB;     /**< Type of struct _NXL_KEY_BLOB pointer */
typedef const NXL_KEY_BLOB*    PCNXL_KEY_BLOB;    /**< Type of const struct _NXL_KEY_BLOB pointer */


/**
 * \struct _NXL_PADDING
 * NXL Padding Struct (Size is 32 bytes)
 */
struct _NXL_PADDING {
    unsigned char   Size;       /**< Padding size */
    unsigned char   Data[31];   /**< Padding data */
};
typedef struct _NXL_PADDING   NXL_PADDING;      /**< Type of struct _NXL_PADDING */
typedef struct _NXL_PADDING*  PNXL_PADDING;     /**< Type of struct _NXL_PADDING pointer */
typedef const NXL_PADDING*    PCNXL_PADDING;    /**< Type of const struct _NXL_PADDING pointer */


/**
 * \struct _NXL_CRYPTO_INFORMATION
 * NXL Crypto Header Struct (Size is 696 bytes)
 */
struct _NXL_CRYPTO_INFORMATION {
    unsigned long   Algorithm;      /**< Algorithm used to encrypt content */
    unsigned long   CbcSize;        /**< Algorithm CBC size */
    NXL_KEY_BLOB    PrimaryKey;     /**< Primary KEK */
    NXL_KEY_BLOB    RecoveryKey;    /**< Recovery KEK, used by Administrator to recovery data */
    __int64         ContentLength;  /**< Content length */
    __int64         AllocateLength; /**< Allocation size for the content */
    NXL_PADDING     Padding;        /**< Padding data */
};
typedef struct _NXL_CRYPTO_INFORMATION   NXL_CRYPTO_INFORMATION;      /**< Type of struct _NXL_CRYPTO_INFORMATION */
typedef struct _NXL_CRYPTO_INFORMATION*  PNXL_CRYPTO_INFORMATION;     /**< Type of struct _NXL_CRYPTO_INFORMATION pointer */
typedef const NXL_CRYPTO_INFORMATION*    PCNXL_CRYPTO_INFORMATION;    /**< Type of const struct _NXL_CRYPTO_INFORMATION pointer */



/**@}*/ // Group End: nxlfmt-struct-crypto



/**
 * \defgroup nxlfmt-struct-section Section Header
 * Section Header
 * @{
 */


/**
 * \defgroup nxlfmt-struct-section-table Section Table
 * Section Table
 * @{
 */


/**
 * \struct _NXL_SECTION
 * NXL Section Information Struct (Size is 16 bytes)
 */
struct _NXL_SECTION {
    char            Name[8];    /**< Section Name */
    unsigned long   Size;       /**< Section Size */
    unsigned long   Checksum;   /**< Section data checksum */
};
typedef struct _NXL_SECTION   NXL_SECTION;      /**< Type of struct _NXL_SECTION */
typedef struct _NXL_SECTION*  PNXL_SECTION;     /**< Type of struct _NXL_SECTION pointer */
typedef const NXL_SECTION*    PCNXL_SECTION;    /**< Type of const struct _NXL_SECTION pointer */


/**
 * \struct _NXL_SECTION_TABLE
 * NXL Section Table Struct (Size is 1176 bytes)
 */
struct _NXL_SECTION_TABLE {
    unsigned char   Checksum[16];                   /**< Section table checksum */
    unsigned long   Count;                          /**< Section count */
    unsigned char   Unused[4];                      /**< Unused */
    NXL_SECTION     Sections[MAX_SECTION_COUNT];    /**< Sections */
};
typedef struct _NXL_SECTION_TABLE   NXL_SECTION_TABLE;      /**< Type of struct _NXL_SECTION_TABLE */
typedef struct _NXL_SECTION_TABLE*  PNXL_SECTION_TABLE;     /**< Type of struct _NXL_SECTION_TABLE pointer */
typedef const NXL_SECTION_TABLE*    PCNXL_SECTION_TABLE;    /**< Type of const struct _NXL_SECTION_TABLE pointer */


/**@}*/ // Group End: nxlfmt-struct-section-table

/**@}*/ // Group End: nxlfmt-struct-section



/**
 * \struct _NXL_HEADER
 * Whole NXL Header Struct (W/O Section Data).
 * Size is 2048 bytes.
 */
struct _NXL_HEADER {
    NXL_SIGNATURE           Signature;      /**< Signature Header */
    NXL_BASIC_INFORMATION   Basic;          /**< Basic Header */
    NXL_CRYPTO_INFORMATION  Crypto;         /**< Crypto Header */
    NXL_SECTION_TABLE       Sections;       /**< Section Table */
};
typedef struct _NXL_HEADER   NXL_HEADER;      /**< Type of struct _NXL_HEADER */
typedef struct _NXL_HEADER*  PNXL_HEADER;     /**< Type of struct _NXL_HEADER pointer */
typedef const NXL_HEADER*    PCNXL_HEADER;    /**< Type of const struct _NXL_HEADER pointer */



#define NXL_BASICINFO_OFFSET    ((unsigned long)FIELD_OFFSET(NXL_HEADER, Basic))        /**< Offset to BasicInformation Header */
#define NXL_CRYPTOINFO_OFFSET   ((unsigned long)FIELD_OFFSET(NXL_HEADER, Crypto))       /**< Offset to CryptoInformation Header */
#define NXL_SCNINFO_OFFSET      ((unsigned long)FIELD_OFFSET(NXL_HEADER, Sections))     /**< Offset to Section Table Header */
#define NXL_SCNDATA_OFFSET      ((unsigned long)(sizeof(NXL_HEADER)))                   /**< Offset to begining of Section Data */



/**@}*/ // Group End: nxlfmt-struct

// ****************************************************************************
// Recover original alignment
// ****************************************************************************
#pragma pack(pop)		// #pragma pack(push)

/**@}*/ // Group End: nxlfmt

static_assert(0x90 == sizeof(NXL_SIGNATURE), "incorrect NXL_SIGNATURE size");
static_assert(0x20 == sizeof(NXL_BASIC_INFORMATION), "incorrect NXL_BASIC_INFORMATION size");
static_assert(0x2B8 == sizeof(NXL_CRYPTO_INFORMATION), "incorrect NXL_CRYPTO_INFORMATION size");
static_assert(0x10 == sizeof(NXL_SECTION), "incorrect NXL_SECTION size");
static_assert(0x498 == sizeof(NXL_SECTION_TABLE), "incorrect NXL_SECTION_TABLE size");
static_assert(0x800 == sizeof(NXL_HEADER), "incorrect NXL_HEADER size");
static_assert(0x90 == NXL_BASICINFO_OFFSET, "incorrect NXL_BASICINFO_OFFSET");
static_assert(0xB0 == NXL_CRYPTOINFO_OFFSET, "incorrect NXL_CRYPTOINFO_OFFSET");
static_assert(0x368 == NXL_SCNINFO_OFFSET, "incorrect NXL_SCNINFO_OFFSET");
static_assert(0x800 == NXL_SCNDATA_OFFSET, "incorrect size");

#ifdef __cplusplus
}
#endif



#endif