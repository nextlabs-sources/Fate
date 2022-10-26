#include "nl_sysenc_lib.h"
#include "nl_sysenc_lib_fw.h"
#include "NextLabsTaggingLib.h"
#include "FileAttributeReaderWriter.h"
#include "nxlAttrs.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_NXLATTRSCPP)

//
// Exported functions
//

bool IsNXLFile(const wchar_t *fileName)
{
  return SE_IsNXLFile(fileName) && !SE_IsEncryptedFW(fileName);
}

int GetNXLFileProps(const wchar_t *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
  unsigned long count;
  wchar_t **keyValuePairs;

  if (!NLT_GetAllTags(filename, &count, &keyValuePairs))
  {
    NLCELOG_RETURN_VAL( -1 )
  }

  // Just to keep PREfast happy.
  if (keyValuePairs == NULL)
  {
    NLCELOG_RETURN_VAL( 0 )
  }

  for (unsigned long i = 0; i < count; i++)
  {
#pragma warning(push)
#pragma warning(disable: 6011)
    GenericNextLabsTagging::AddKeyValueHelperW(attrs, keyValuePairs[i*2],
                                               keyValuePairs[i*2+1]);
#pragma warning(pop)
  }

  NLT_FreeAllTags(count, keyValuePairs);
  NLCELOG_RETURN_VAL( 0 )
}

int SetNXLFileProps(const wchar_t *filename, const ResourceAttributes *attrs)
{NLCELOG_ENTER
  const int count = GetAttributeCount(attrs);
  int i;

  for (i = 0; i < count; i++)
  {
    if (!NLT_AddTag(filename, GetAttributeName(attrs, i),
                    GetAttributeValue(attrs, i)))
    {
      NLCELOG_RETURN_VAL( -1 )
    }
  }

  NLCELOG_RETURN_VAL( 0 )
}

int RemoveNXLFileProps(const wchar_t *filename, const ResourceAttributes *attrs)
{NLCELOG_ENTER
  const int count = GetAttributeCount(attrs);
  int i;

  for (i = 0; i < count; i++)
  {
    if (!NLT_DeleteTag(filename, GetAttributeName(attrs, i)))
    {
      NLCELOG_RETURN_VAL( -1 )
    }
  }

  NLCELOG_RETURN_VAL( 0 )
}

bool IsNXL10File(const wchar_t *fileName)
{
  return SE_IsEncryptedFW(fileName);
}

int GetNXL10FileProps(const wchar_t *filename, ResourceAttributes *attrs)
{
  // Currently, GetNXLFileProps() can also handle heavy-write files.  So we
  // just call that function to do the work.
  return GetNXLFileProps(filename, attrs);
}
