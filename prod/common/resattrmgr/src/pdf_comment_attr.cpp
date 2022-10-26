
#include <string>

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "resattrlib.h"
#include "pdf_comment_attr.h"
#include "FileAttributeReaderWriter.h"
#include "base64.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_PDF_COMMENT_ATTRCPP)

#define POLICY_CONTROLLER_SUBKEY_PATH L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller"
#define POLICY_CONTROLLER_PDF_DOCINFO_VALUE_NAME L"PdfTagDocInfo"

static BOOL Is64bitOS()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	if(si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		return TRUE;
	else
		return FALSE;
}

static BOOL tagUsingPoDoFo = FALSE;
 
void InitializePDFTaggingMethod()
{
    HKEY hPCKey;
    DWORD dwPdfTagDocInfo=0;
    DWORD dwLen=sizeof(DWORD);
    REGSAM samDesired=KEY_ALL_ACCESS;
    if(Is64bitOS()) {
        samDesired|=KEY_WOW64_64KEY;
    }
    long lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, POLICY_CONTROLLER_SUBKEY_PATH, 0, samDesired, &hPCKey);

    if (lResult != ERROR_SUCCESS)
        return;

    lResult = RegQueryValueExW(hPCKey, POLICY_CONTROLLER_PDF_DOCINFO_VALUE_NAME, NULL, NULL,(BYTE*)&dwPdfTagDocInfo,&dwLen);

    if(lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hPCKey);
        return;
    }
    RegCloseKey(hPCKey);

    tagUsingPoDoFo = dwPdfTagDocInfo;
}

BOOL IsTagWithPoDoFo()
{
    return tagUsingPoDoFo;
}


//return 0 <==> not found
int FindAttribute(ResourceAttributes* pAttr,const WCHAR* pwName,int &idx)
{
	int count=GetAttributeCount(pAttr);
	if(count==0)
		return 0;
	if(pwName==NULL)
		return 0;
	for(int i=0;i<count;i++)
	{
		const WCHAR *pname=GetAttributeName(pAttr,i);
		if(wcscmp(pname,pwName)==0)
		{
			idx=i;
			return 1;
		}
	}
	return 0;
}
BOOL InitPDFWithCommentAttr(const WCHAR *fileName, int *fd, int *size)
{
	int pfh=0;
	*fd=_wsopen_s(&pfh,fileName,_O_RDWR|_O_BINARY,_SH_DENYNO ,_S_IREAD | _S_IWRITE);
	if(*fd != 0 || pfh == -1)
		return FALSE;
	if(size!=NULL)
	{
		*size=_filelength(*fd);
		if(*size==-1)
		{
			_close(*fd);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL ReadPDFFilePropsWithCommentAttr(const WCHAR * fileName, ResourceAttributes *attrs,int *tailLen=NULL,int *filesize=NULL)
{NLCELOG_ENTER
	int fd=0;
	if(InitPDFWithCommentAttr(fileName,&fd,filesize)==FALSE)
		NLCELOG_RETURN_VAL( FALSE )
	
	if(_lseek(fd,-PDFTAG_COMMNET_ATTR_LINE3_LEN,SEEK_END)==-1)
	{
		_close(fd);
		NLCELOG_RETURN_VAL( FALSE )
	}
	char wzLine3[33]="";
	if(_read(fd,wzLine3,PDFTAG_COMMNET_ATTR_LINE3_LEN)<=0)
	{
		_close(fd);
		NLCELOG_RETURN_VAL( FALSE )
	}
	//validation for Line 3
	if(strncmp(wzLine3,PDFTAG_COMMENT_ATTR_LINE3_HEADER,strlen(PDFTAG_COMMENT_ATTR_LINE3_HEADER)))
	{
		_close(fd);
		NLCELOG_RETURN_VAL( FALSE )
	}
	long lTagLen=atol(wzLine3+strlen(PDFTAG_COMMENT_ATTR_LINE3_HEADER));
	if(lTagLen==0)
	{
		_close(fd);
		NLCELOG_RETURN_VAL( FALSE )
	}
	char *pL1L2=(char*)malloc(lTagLen+1);
	if(pL1L2==NULL)
	{
		_close(fd);
		NLCELOG_RETURN_VAL( FALSE )
	}
	memset(pL1L2,0,lTagLen+1);
	//seek the begin of line 1
	if(_lseek(fd,-(PDFTAG_COMMNET_ATTR_LINE3_LEN+lTagLen),SEEK_END)==-1)
	{
		_close(fd);
		free(pL1L2);
		NLCELOG_RETURN_VAL( FALSE )
	}
	//read the line 1 and 2
	if(_read(fd,pL1L2,lTagLen)<=0)
	{
		_close(fd);
		free(pL1L2);
		NLCELOG_RETURN_VAL( FALSE )
	}

	if(strncmp(pL1L2,PDFTAG_COMMENT_ATTR_LINE1_HEADER,strlen(PDFTAG_COMMENT_ATTR_LINE1_HEADER)))
	{
		_close(fd);
		free(pL1L2);
		NLCELOG_RETURN_VAL( FALSE )
	}

	char* pVersion=pL1L2+strlen(PDFTAG_COMMENT_ATTR_LINE1_HEADER);
	if(strncmp(pVersion,PDFTAG_COMMENT_ATTR_VERSION_STR,strlen(PDFTAG_COMMENT_ATTR_VERSION_STR)))
	{
		_close(fd);
		free(pL1L2);
		NLCELOG_RETURN_VAL( FALSE )
	}

	char* pLine2=pL1L2+strlen(PDFTAG_COMMENT_ATTR_LINE1_HEADER)+strlen(PDFTAG_COMMENT_ATTR_VERSION_STR)+2;
	if(strncmp(pLine2,PDFTAG_COMMENT_ATTR_LINE2_HEADER,strlen(PDFTAG_COMMENT_ATTR_LINE2_HEADER)))
	{
		_close(fd);
		free(pL1L2);
		NLCELOG_RETURN_VAL( FALSE )
	}
	if(tailLen!=NULL)
		*tailLen=lTagLen+PDFTAG_COMMNET_ATTR_LINE3_LEN;
	unsigned char *pEncodedTags=(unsigned char*)(pLine2+strlen(PDFTAG_COMMENT_ATTR_LINE2_HEADER));
	Base64 base64;
#ifdef _DEBUG	
	//unsigned char tags1[]="name1\0value1\0name2\0value2\0";
	//std::string tags1re=base64.base64_encode(tags1,26);
#endif

	size_t tagsLen=lTagLen-strlen(PDFTAG_COMMENT_ATTR_LINE1_HEADER)-strlen(PDFTAG_COMMENT_ATTR_VERSION_STR)-2-strlen(PDFTAG_COMMENT_ATTR_LINE2_HEADER);
	
	std::string strTags=base64.base64_decode(pEncodedTags,(unsigned int)tagsLen);
	size_t len=strTags.length();
	size_t lenDone=0;
	while(lenDone<len)
	{
		const char* pName=strTags.c_str()+lenDone;
		lenDone+=strlen(pName)+1;
		if(len<lenDone)
			break;
		const char* pValue=strTags.c_str()+lenDone;
		lenDone+=strlen(pValue)+1;
		if(len<lenDone)
			break;
		if(strlen(pName)&&strlen(pValue))
			GenericNextLabsTagging::AddKeyValueHelperA(attrs, pName,pValue);
	}
	_close(fd);
	free(pL1L2);
	NLCELOG_RETURN_VAL( TRUE )

}
BOOL GetPDFFilePropsWithCommentAttr(const WCHAR *fileName, ResourceAttributes *attrs)
{
	BOOL bRet=TRUE;

	bRet=ReadPDFFilePropsWithCommentAttr(fileName,attrs);

	return bRet;
}

BOOL WriteCommentAttributes(const WCHAR * fileName, const char* commentAttrs,int truncsize=-1)
{NLCELOG_ENTER
	int pfh=0;
	int fd=_wsopen_s(&pfh,fileName,_O_RDWR|_O_BINARY,_SH_DENYNO ,_S_IREAD | _S_IWRITE);
	if(fd != 0 || pfh == -1)
		NLCELOG_RETURN_VAL( FALSE )
	if(truncsize!=-1)
	{
		if (_chsize(fd,truncsize) != 0)
		{
    		_close(fd);
    		NLCELOG_RETURN_VAL( FALSE )
		}
	}
	if(_lseek(fd,0,SEEK_END)==-1)
	{
		_close(fd);
		NLCELOG_RETURN_VAL( FALSE )
	}
	if(commentAttrs&&_write(fd,commentAttrs,(unsigned int)strlen(commentAttrs))==-1)
	{
		_close(fd);
		NLCELOG_RETURN_VAL( FALSE )
	}
	_close(fd);
	NLCELOG_RETURN_VAL( TRUE )
}
void CompositeCommentAttributes(ResourceAttributes *pAttrNew,ResourceAttributes *pAttrOld,std::string&strComment,bool bRemove=false)
{NLCELOG_ENTER
	int attrsNew=GetAttributeCount(pAttrNew);
	int attrsOld=GetAttributeCount(pAttrOld);

	strComment=PDFTAG_COMMENT_ATTR_LINE1_HEADER;
	strComment+=PDFTAG_COMMENT_ATTR_VERSION_STR;
	strComment+="\r\n";
	
	std::string strTags;
	
	if(bRemove==false)
	{
		for(int i=0;i<attrsNew;i++)
		{
			size_t size = 0;
			const WCHAR*pwName=GetAttributeName(pAttrNew,i);
			wcstombs_s(&size,NULL,0,pwName,0);
			char *pcName = new char[size];
			wcstombs_s(&size, pcName, size, pwName, _TRUNCATE);
			
			size = 0;
			const WCHAR*pwValue=GetAttributeValue(pAttrNew,i);
			wcstombs_s(&size,NULL,0,pwValue,0);
			char *pcValue = new char[size];
			wcstombs_s(&size, pcValue, size, pwValue, _TRUNCATE);
			
			strTags+=pcName;
			strTags.resize(strTags.size()+1,'\0');
			strTags+=pcValue;
			strTags.resize(strTags.size()+1,'\0');
		}
	}
	int existTagsNumForRemove=0;
	if(attrsOld)
	{	
		for(int i=0;i<attrsOld;i++)
		{
			int idx=0;
			size_t size = 0;
			const WCHAR*pwName=GetAttributeName(pAttrOld,i);
			const WCHAR*pwValue=GetAttributeValue(pAttrOld,i);
			if(pwName&&FindAttribute(pAttrNew,pwName,idx)==0)
			{
				size=0;
				wcstombs_s(&size,NULL,0,pwName,0);
				char *pcName = new char[size];
				wcstombs_s(&size, pcName, size, pwName, _TRUNCATE);
				
				size = 0;
				wcstombs_s(&size,NULL,0,pwValue,0);
				char *pcValue = new char[size];
				wcstombs_s(&size, pcValue, size, pwValue, _TRUNCATE);

				strTags+=pcName;
				strTags.resize(strTags.size()+1,'\0');
				strTags+=pcValue;
				strTags.resize(strTags.size()+1,'\0');
				existTagsNumForRemove++;
			}
		}
	}
	if(existTagsNumForRemove==0&&bRemove==true)
	{
		strComment="";
		NLCELOG_RETURN
	}
	//int tagslen=strTags.length();
	Base64 base;
	std::string strEncodedTags=base.base64_encode(strTags);
	strComment+=PDFTAG_COMMENT_ATTR_LINE2_HEADER;
	strComment+=strEncodedTags;
	strComment+="\r\n";

	char cLine3[PDFTAG_COMMNET_ATTR_LINE3_LEN+1]="";
	memset(cLine3,' ',PDFTAG_COMMNET_ATTR_LINE3_LEN);
	cLine3[PDFTAG_COMMNET_ATTR_LINE3_LEN-2]='\r';
	cLine3[PDFTAG_COMMNET_ATTR_LINE3_LEN-1]='\n';
	cLine3[PDFTAG_COMMNET_ATTR_LINE3_LEN]=0;
	size_t lL1L2=strComment.length();
	_snprintf_s(cLine3,PDFTAG_COMMNET_ATTR_LINE3_LEN+1, _TRUNCATE,"%s%d",PDFTAG_COMMENT_ATTR_LINE3_HEADER,lL1L2);
	cLine3[strlen(cLine3)]=' ';
	strComment+=cLine3;
}
BOOL SetPDFFilePropsWithCommentAttr(const WCHAR *fileName, ResourceAttributes *attrs)
{NLCELOG_ENTER
	NLCELOG_DEBUGLOG( L"The Parameters are: fileName=%s, attrs=%p\n", fileName, attrs );
	int attrsCount=GetAttributeCount(attrs);
	if(attrsCount==0)
		NLCELOG_RETURN_VAL( TRUE )

	BOOL bRet=TRUE;
	int tailLen=0,filesize=0;
	ResourceAttributes *pAttr = NULL;
	if (0 != AllocAttributes(&pAttr) || NULL == pAttr)
	{
		NLCELOG_RETURN_VAL( FALSE )
	}

	bRet=ReadPDFFilePropsWithCommentAttr(fileName,pAttr,&tailLen,&filesize);
	if(bRet==FALSE)
	{
		std::string strCommentAttrs;
		CompositeCommentAttributes(attrs,pAttr,strCommentAttrs);
		bRet=WriteCommentAttributes(fileName,strCommentAttrs.c_str());
	}
	else
	{
		int truncsize=filesize-tailLen;
		if(truncsize<=0)
			bRet=FALSE;
		else
		{
			std::string strCommentAttrs;
			CompositeCommentAttributes(attrs,pAttr,strCommentAttrs);
			bRet=WriteCommentAttributes(fileName,strCommentAttrs.c_str(),truncsize);
		}
	}
	FreeAttributes(pAttr);
	NLCELOG_RETURN_VAL( bRet )
}


BOOL RemovePDFFilePropsWithCommentAttr(const WCHAR* fileName, ResourceAttributes *attrs)
{NLCELOG_ENTER
	int attrsCount=GetAttributeCount(attrs);
	if(attrsCount==0)
		NLCELOG_RETURN_VAL( TRUE )

	BOOL bRet=TRUE;
	int tailLen=0,filesize=0;
	ResourceAttributes *pAttr = NULL;
	if (0 != AllocAttributes(&pAttr) || NULL == pAttr)
	{
		NLCELOG_RETURN_VAL( FALSE )
	}

	bRet=ReadPDFFilePropsWithCommentAttr(fileName,pAttr,&tailLen,&filesize);
	if(GetAttributeCount(pAttr)==0)
	{
		FreeAttributes(pAttr);
		NLCELOG_RETURN_VAL( TRUE )
	}
	if(bRet==FALSE)
		bRet=TRUE;
	else
	{
		int truncsize=filesize-tailLen;
		if(truncsize<=0)
			bRet=FALSE;
		else
		{
			std::string strCommentAttrs;
			CompositeCommentAttributes(attrs,pAttr,strCommentAttrs,true);
			bRet=WriteCommentAttributes(fileName,strCommentAttrs.c_str(),truncsize);
		}
	}
	FreeAttributes(pAttr);
	NLCELOG_RETURN_VAL( bRet )
}
