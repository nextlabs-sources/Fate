#ifndef CEATTRREADER_H
#define CEATTRREADER_H
#endif

#include "resattrlib.h"
#include "resattrmgr.h"

int DefImpl_Initialize(void **pluginData);
int DefImpl_Deinitialize(void *pluginData);
int DefImpl_GetAttributes(void *pluginData, const WCHAR *resourceName, ResourceAttributes *attrs, TagType );
int DefImpl_SetAttributes(void *pluginData, const WCHAR *resourceName, ResourceAttributes *attrs, TagType );
int DefImpl_RemoveAttributes(void *pluginData, const WCHAR *resourceName, ResourceAttributes *attrs);
int Convert_Raw_2_PC_For_Non_Office_Imp(ResourceAttributes *raw_attrs,ResourceAttributes* PC_attrs);
int Convert_PC_2_RAW_For_Non_Office_Imp(ResourceAttributes *PC_attrs,ResourceAttributes* raw_attrs);
int Convert4GetAttributes(ResourceAttributes* attrs, ResourceAttributes* existing_attrs);
int Convert4SetAttributes(ResourceAttributes* attrs_to_set, ResourceAttributes* merged_attrs);
