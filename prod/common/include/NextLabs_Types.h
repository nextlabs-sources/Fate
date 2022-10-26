/******************************************************************************************
 *
 * NextLabs (NL) Data Types
 *
 ******************************************************************************************/

#ifndef __NEXTLABS_TYPES_H__
#define __NEXTLABS_TYPES_H__



#define NL_FILE_EXT                         L".nxl"



/** NextLabsFile_Header_1_0_t
 *
 *  \brief NextLabs File Type v1.0
 */

#define NL_FILE_VERSION_1_0_MAJOR           1
#define NL_FILE_VERSION_1_0_MINOR           0

extern const __declspec(selectany) unsigned char NL_FILE_1_0_COOKIE[8] = {'N', 'e', 'x', 't', 'L', 'a', 'b', 's'};

typedef struct
{
  unsigned char version_major;	/* Version Major */
  unsigned char version_minor;	/* Minor Version */
  unsigned char reserved[2];	/* Reserved */
  unsigned int  header_size;	/* Header Size (bytes) */
  unsigned int  stream_count; 	/* Stream Count */
  unsigned char cookie[8];	/* Cookie(fixed), always 'NextLabs' */
} NextLabsFile_Header_1_0_t;



/** NextLabsFile_Header_t
 *
 *  \brief NextLabs File Type v2.1
 */

#define NL_FILE_VERSION_MAJOR               2
#define NL_FILE_VERSION_MINOR               1

extern const __declspec(selectany) unsigned char NL_FILE_COOKIE[8] = {0x4E, 0xE5, 0x78, 0xF4, 0x4C, 0xE1, 0x62, 0xF3};

/* Flags */
#define NLF_WRAPPED                     0x0000000000000001ui64

typedef struct
{
  unsigned char version_major;	/* Version Major */
  unsigned char version_minor;	/* Minor Version */
  unsigned char reserved[2];	/* Reserved */
  unsigned int  header_size;	/* Header Size (bytes) */
  unsigned int  stream_count; 	/* Stream Count */
  unsigned char cookie[8];		/* Cookie(fixed), always NL_FILE_COOKIE */
  unsigned long attrs;          /* Original file attributes */
  unsigned __int64 flags;       /* Flags */
  unsigned char unused[480];    /* Unused */
  wchar_t orig_file_name[256];  /* original file name (not null-terminated if
                                   file name is 256 chars) */
} NextLabsFile_Header_t;



/** NextLabsFile_StreamHeader_t
 *
 *  \brief NextLabs File stream header.
 */
typedef struct
{
  unsigned int  stream_size;	/* Size (bytes) */
  char stream_name[8];			/* Stream Name */
} NextLabsFile_StreamHeader_t;



#endif /* __NEXTLABS_TYPES_H__ */
