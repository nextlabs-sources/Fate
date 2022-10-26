#ifndef _WORKTHREADS_H_
#define _WORKTHREADS_H_

unsigned __stdcall DoAutomaticTaggingThread( void* pArguments );
unsigned __stdcall DoManualTaggingThread( void* pArguments );
unsigned __stdcall DoRemoveTagsThread( void* pArguments );
unsigned __stdcall GetTagValueByNameThread( void* pArguments );
unsigned __stdcall GetAllTagValuesThread(void* pArguments );

#endif