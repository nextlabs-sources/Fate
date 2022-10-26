/******************************************************************************************
 *
 * NextLabs Tagging (NLT) Data Types
 *
 ******************************************************************************************/

#ifndef __NEXTLABS_TAGGING_TYPES_H__
#define __NEXTLABS_TAGGING_TYPES_H__

#include "NextLabs_Types.h"



#define NLT_STREAM_NAME                     "NLT"
#define NLT_FILE_VERSION_MAJOR              1
#define NLT_FILE_VERSION_MINOR              0



/** NextLabsTaggingFile_Header_t
 *
 *  \brief Extension written to the header of tagged file.
 */
#define MAX_TAG_DATA_SIZE  2030
typedef struct
{
  NextLabsFile_StreamHeader_t   sh;             /* Stream Header */
  unsigned char                 version_major;  /* Major version */
  unsigned char                 version_minor;  /* Minor version */
  unsigned char                 reserved1[2];   /* Reserved 1 */
  unsigned long                 tagsSize;       /* Size of all tags in bytes */
  unsigned char                 reserved2[16];  /* Reserved 2 */
  wchar_t                       tag_data[MAX_TAG_DATA_SIZE]; /* Tag data */
} NextLabsTaggingFile_Header_t;



#endif /* __NEXTLABS_TAGGING_TYPES_H__ */
