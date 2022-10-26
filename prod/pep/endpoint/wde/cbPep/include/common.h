#ifndef COMMON_H
#define COMMON_H

#include <ShObjIdl.h>

typedef struct
{
	IFileOperation* pObj;
	ULONG refCount;
}OBJ_INFO;

#endif