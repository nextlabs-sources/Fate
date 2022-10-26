#include "utils.h"
#include "ole.h"
#include "pdf.h"
#include "ooxml.h"
#include "fsysea.h"
#include <gsf/gsf-utils.h>
namespace NSLinuxTagLib
{
const char Utils::TEMPFILE_TEMPLATE[]=".tagtemp.XXXXXX";
BOOL Utils::IsPDF(const char*pchFileName)
{
	return NSLinuxTagLib::PDFFileTagging::IsPDF(pchFileName);;
}
BOOL Utils::IsOLE(const char*pchFileName)
{
	return NSLinuxTagLib::OLEFileTagging::IsOLE(pchFileName);
}
BOOL Utils::IsOOXML(const char*pchFileName)
{
	return NSLinuxTagLib::OOXMLFileTagging::IsOOXML(pchFileName);
}
FileTagging* Utils::CreateFileTagging(const char*pchFileName)
{
	FileTagging* pFileTagging=NULL;
	if(IsOOXML (pchFileName)==TRUE)
		pFileTagging=new OOXMLFileTagging ();
	else if(IsPDF(pchFileName)==TRUE)
		pFileTagging=new PDFFileTagging2 ();
	else if(IsOLE(pchFileName)==TRUE)
		pFileTagging=new OLEFileTagging();
	else
	{
		//printf("It is not Office 2003, Office 2007 and PDF. Now manipulate it as file system extend attribute.\n");
		pFileTagging=new FSysEAFileTagging();
	}

	return pFileTagging;
}
BOOL Utils::GetTempFile(const char*pchFileName,std::string& strTempFile)
{
	char fnTemp[128]="";
	strncpy_s(fnTemp,128,TEMPFILE_TEMPLATE, _TRUNCATE);

	int fd=mkstemp(fnTemp);
	if(fd==-1)
		return FALSE;
	close(fd);
	remove(fnTemp);
	strTempFile=fnTemp;
	char* pSuffix = strrchr(pchFileName, L'.');
	if(pSuffix)
		strTempFile+=pSuffix;
	return TRUE;
}


}
