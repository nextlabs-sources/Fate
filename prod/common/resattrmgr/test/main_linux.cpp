#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <string>
#include <vector>
#include <gsf/gsf-utils.h>
#include "resattrmgr.h"

#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-zip.h>

#include <gsf/gsf-utils.h>

#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>

#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-msole.h>

#include <gsf/gsf-open-pkg-utils.h>

typedef std::pair<std::string,std::string> NVPair;
typedef std::vector<NVPair> TagList;
	
int WriteTag(const char*pchFileName,TagList&tagList)
{
	ResourceAttributeManager *pMgr;
	
	if (0 != CreateAttributeManager(&pMgr) || NULL == pMgr)
	{
		printf("ReadTag: Can't create attribute mgr\n");
		pMgr = NULL;
		return 1;
	}
	
	ResourceAttributes *pAttr = NULL;

	if (0 != AllocAttributes(&pAttr) || NULL == pAttr)
	{
		printf("SapPsstTag: Can't alloc attribute\n");
		return 1;
	}
	for(int i=0;i<tagList.size();i++)
		AddAttribute(pAttr,tagList[i].first.c_str(),tagList[i].second.c_str());
	
	
	WriteResourceAttributes(pMgr,pchFileName,pAttr);
	
    FreeAttributes(pAttr);
    CloseAttributeManager(pMgr);
	return 0;
}

void PrintHex(void* p,int len)
{
	int idx=0;
	char*pch=(char*)p;
	for(idx=0;idx<len;idx++)
	{
		if(idx!=0&&idx%16==0)
			printf("\n");
		printf("%02X,",pch[idx]);
	}
	printf("\n");
}

int ReadTag(const char*pchFileName,TagList& tagList)
{
	ResourceAttributeManager *pMgr;
	
	if (0 != CreateAttributeManager(&pMgr) || NULL == pMgr)
	{
		printf("ReadTag: Can't create attribute mgr\n");
		pMgr = NULL;
		return 1;
	}
	
	ResourceAttributes *pAttr = NULL;

	if (0 != AllocAttributes(&pAttr) || NULL == pAttr)
	{
		printf("SapPsstTag: Can't alloc attribute\n");
		return 1;
	}
	
	for(int i=0;i<tagList.size();i++)
		AddAttribute(pAttr,tagList[i].first.c_str(),tagList[i].second.c_str());
	
	long lCountBegin=GetAttributeCount(pAttr);
	ReadResourceAttributes(pMgr,pchFileName,pAttr);
	long lAttrCount=GetAttributeCount(pAttr); 
	if(lAttrCount==0)
	{
		printf("no attribute!\n");
		return 1;
	}
	//if(lCountBegin==GetAttributeCount(pAttr))
	{
    	for(long lIdx=0;lIdx<lAttrCount;lIdx++)
    	{
    		printf("||%s=%s\n",GetAttributeName(pAttr,lIdx),GetAttributeValue(pAttr,lIdx));	
    	}
	}
    FreeAttributes(pAttr);
    CloseAttributeManager(pMgr);
	return 0;
}
int RemoveTag(const char*pchFileName,TagList&tagList)
{
	ResourceAttributeManager *pMgr;
	
	if (0 != CreateAttributeManager(&pMgr) || NULL == pMgr)
	{
		printf("ReadTag: Can't create attribute mgr\n");
		pMgr = NULL;
		return 1;
	}
	
	ResourceAttributes *pAttr = NULL;

	if (0 != AllocAttributes(&pAttr) || NULL == pAttr)
	{
		printf("SapPsstTag: Can't alloc attribute\n");
		return 1;
	}
	for(int i=0;i<tagList.size();i++)
		AddAttribute(pAttr,tagList[i].first.c_str(),tagList[i].second.c_str());
	
	
	RemoveResourceAttributes(pMgr,pchFileName,pAttr);
	
    FreeAttributes(pAttr);
    CloseAttributeManager(pMgr);
	return 0;
}

enum {
	XL_NS_SS,
	XL_NS_SS_DRAW,
	XL_NS_CHART,
	XL_NS_DRAW,
	XL_NS_DOC_REL,
	XL_NS_PKG_REL,
	OOXML_CUSTPROPS,
	OOXML_PROPVTYPE
};
static GsfXMLInNode const xlsx_workbook_dtd[] = {
GSF_XML_IN_NODE_FULL (START, START, -1, NULL, GSF_XML_NO_CONTENT, FALSE, TRUE, NULL, NULL, 0),
GSF_XML_IN_NODE_FULL (START, WORKBOOK, XL_NS_SS, "workbook", GSF_XML_NO_CONTENT, FALSE, TRUE, NULL, NULL, 0),
  GSF_XML_IN_NODE (WORKBOOK, VERSION, XL_NS_SS,	   "fileVersion", GSF_XML_NO_CONTENT, NULL, NULL),
  GSF_XML_IN_NODE (WORKBOOK, PROPERTIES, XL_NS_SS, "workbookPr", GSF_XML_NO_CONTENT, NULL, NULL),
  GSF_XML_IN_NODE (WORKBOOK, CALC_PROPS, XL_NS_SS, "calcPr", GSF_XML_NO_CONTENT, NULL, NULL),
  GSF_XML_IN_NODE (WORKBOOK, VIEWS,	 XL_NS_SS, "bookViews",	GSF_XML_NO_CONTENT, NULL, NULL),
    GSF_XML_IN_NODE (VIEWS,  VIEW,	 XL_NS_SS, "workbookView",  GSF_XML_NO_CONTENT, NULL, NULL),
  GSF_XML_IN_NODE (WORKBOOK, SHEETS,	 XL_NS_SS, "sheets", GSF_XML_NO_CONTENT, NULL, NULL),
    GSF_XML_IN_NODE (SHEETS, SHEET,	 XL_NS_SS, "sheet", GSF_XML_NO_CONTENT, NULL, NULL),
  GSF_XML_IN_NODE (WORKBOOK, WEB_PUB,	 XL_NS_SS, "webPublishing", GSF_XML_NO_CONTENT, NULL, NULL),
  GSF_XML_IN_NODE (WORKBOOK, EXTERNS,	 XL_NS_SS, "externalReferences", GSF_XML_NO_CONTENT, NULL, NULL),
    GSF_XML_IN_NODE (EXTERNS, EXTERN,	 XL_NS_SS, "externalReference", GSF_XML_NO_CONTENT, NULL, NULL),
  GSF_XML_IN_NODE (WORKBOOK, NAMES,	 XL_NS_SS, "definedNames", GSF_XML_NO_CONTENT, NULL, NULL),
    GSF_XML_IN_NODE (NAMES, NAME,	 XL_NS_SS, "definedName", GSF_XML_NO_CONTENT, NULL, NULL),
  GSF_XML_IN_NODE (WORKBOOK, RECOVERY,	 XL_NS_SS, "fileRecoveryPr", GSF_XML_NO_CONTENT, NULL, NULL),

GSF_XML_IN_NODE_END
};
static GsfXMLInNS const xlsx_ns[] = {
	GSF_XML_IN_NS (XL_NS_SS,	"http://schemas.microsoft.com/office/excel/2006/2"), 			  /* Office 12 BETA-1 Technical Refresh */
	GSF_XML_IN_NS (XL_NS_SS,	"http://schemas.openxmlformats.org/spreadsheetml/2006/main"), 			  /* Office 12 BETA-1 Technical Refresh */
	
	GSF_XML_IN_NS (XL_NS_SS,	"http://schemas.openxmlformats.org/spreadsheetml/2006/5/main"),		  /* Office 12 BETA-2 */
	GSF_XML_IN_NS (XL_NS_SS,	"http://schemas.openxmlformats.org/spreadsheetml/2006/7/main"),		  /* Office 12 BETA-2 Technical Refresh */
	GSF_XML_IN_NS (XL_NS_SS_DRAW,	"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"),	  /* Office 12 BETA-2 */
	GSF_XML_IN_NS (XL_NS_SS_DRAW,	"http://schemas.openxmlformats.org/drawingml/2006/3/spreadsheetDrawing"), /* Office 12 BETA-2 Technical Refresh */
	GSF_XML_IN_NS (XL_NS_CHART,	"http://schemas.openxmlformats.org/drawingml/2006/3/chart"),		  /* Office 12 BETA-2 */
	GSF_XML_IN_NS (XL_NS_CHART,	"http://schemas.openxmlformats.org/drawingml/2006/chart"),		  /* Office 12 BETA-2 Technical Refresh */
	GSF_XML_IN_NS (XL_NS_DRAW,	"http://schemas.openxmlformats.org/drawingml/2006/3/main"),		  /* Office 12 BETA-2 */
	GSF_XML_IN_NS (XL_NS_DRAW,	"http://schemas.openxmlformats.org/drawingml/2006/main"),		  /* Office 12 BETA-2 Technical Refresh */
	GSF_XML_IN_NS (XL_NS_DOC_REL,	"http://schemas.openxmlformats.org/officeDocument/2006/relationships"),
	GSF_XML_IN_NS (XL_NS_PKG_REL,	"http://schemas.openxmlformats.org/package/2006/relationships"),
	{ NULL }
};

void xml_prop_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	for (; attrs != NULL && attrs[0] && attrs[1] ; attrs += 2)
		printf("%s=%s\n",attrs[0],attrs[1]);
}
void xml_prop_end   (GsfXMLIn *xin, GsfXMLBlob *unknown)
{
	printf("xml_prop_end\n");
}

static GsfXMLInNode const garray_ooxml_custom_properties_dtd[] = {
GSF_XML_IN_NODE_FULL (START, START, -1, NULL, GSF_XML_NO_CONTENT, FALSE, TRUE, NULL, NULL, 0),
GSF_XML_IN_NODE_FULL (START, PROPERTIES, OOXML_CUSTPROPS, "Properties", GSF_XML_NO_CONTENT, FALSE, TRUE, NULL, NULL, 0),
	GSF_XML_IN_NODE (PROPERTIES, PROPERTY, OOXML_CUSTPROPS, "property", GSF_XML_NO_CONTENT, &xml_prop_start, &xml_prop_end),
		GSF_XML_IN_NODE (PROPERTY, LPWSTR, OOXML_PROPVTYPE, "lpwstr", GSF_XML_NO_CONTENT, NULL, NULL),
		//GSF_XML_IN_NODE (PROPERTY, FILETIME, OOXML_PROPVTYPE, "filetime", GSF_XML_NO_CONTENT, NULL, NULL),
		//GSF_XML_IN_NODE (PROPERTY, BOOL, OOXML_PROPVTYPE, "bool", GSF_XML_NO_CONTENT, NULL, NULL),
		//GSF_XML_IN_NODE (PROPERTY, R8, OOXML_PROPVTYPE, "r8", GSF_XML_NO_CONTENT, NULL, NULL),

GSF_XML_IN_NODE_END
};

static GsfXMLInNS const garray_ooxml_props_ns[] = {
	GSF_XML_IN_NS (OOXML_CUSTPROPS,	"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties"),
	GSF_XML_IN_NS (OOXML_PROPVTYPE,	"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"),
	{ NULL }
};

static gboolean xlsx_parse_stream (void *state, GsfInput *in, GsfXMLInNode const *dtd)
{
	gboolean  success = FALSE;

	if (NULL != in) {
		GsfXMLInDoc *doc = gsf_xml_in_doc_new (dtd, garray_ooxml_props_ns);

		success = gsf_xml_in_doc_parse (doc, in, state);

		if (!success)
			printf("it is corrupted!\n");

		gsf_xml_in_doc_free (doc);
		g_object_unref (G_OBJECT (in));
	}
	return success;
}
int TestXLSX(const char*pchFileName)
{
	GError *err=NULL;
	gsf_init();
	GsfInput  *input=gsf_input_stdio_new (pchFileName, NULL);
	GsfInfile *infile=gsf_infile_zip_new (input, NULL);
	
	GsfInput* relCustomProp=gsf_open_pkg_open_rel_by_type(GSF_INPUT(infile),
		"http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties",
		&err);
	gsf_input_dump(GSF_INPUT(relCustomProp),false);
	gsf_input_seek(relCustomProp,0,G_SEEK_SET);
	if(NULL!=relCustomProp)
	{
		xlsx_parse_stream (NULL, relCustomProp, garray_ooxml_custom_properties_dtd);
	}
	//GsfInput *wb_part = gsf_open_pkg_get_rel_by_type (GSF_INPUT (infile),
	//		"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");
	//if (NULL != wb_part)
	//{
	//	xlsx_parse_stream (NULL, wb_part, xlsx_workbook_dtd);
	//}
	gsf_shutdown();
}


void Usage(const char*pchProgName)
{
	fprintf (stderr, "Usage: %s [-read|-write|-remove] -file filename [-name tagname] [-value tagvalue] [-tag tagname=tagvalue]\n\n", pchProgName);
	//fprintf (stderr, "For example:\n");
	//fprintf (stderr, "\t 1. \"%s -read A B\" write a tag named A with value B to file X.\n", pchProgName);
	//fprintf (stderr, "\t 2. \"%s X A\" read a tag named A from file X.\n", pchProgName);
	//fprintf (stderr, "\t 3. \"%s -cpole X Y\" copy file X to Y. If Y is not exist then create it.\n", pchProgName);
	return ;
}
/*
Assert Model:
	Every number in the assert model string represent an operation on the target file:
	1: Read tag(s) which defined by TagList
	2: Set tag(s) defined by TagList
	3: Remove tag(s) defined by TagList
	4: Read all tags on file
*/
int AssertTag(std::vector<std::string>&vecFiles,TagList& tagList,const char*pchAssertModel,bool bOneFileByOneFile=true)
{
	/*
	static char chDefaultModel[]="34243";
	"34243" means following logic:
	1)	Delete ¡°X=Y¡± on files in the given directory (currently for PDF, we have around 120 PDF Files)
	2)	Read Tags on all the files and print with time taken to read tags
	3)	Add Tag ¡°X=Y¡± on all the files, and assert whether ¡°X=Y¡± is present on the file
	4)	Repeat Step 2 -> i.e. Read Tags on all the files and print with time taken to read tags
	5)	Repeat Step 1 -> i.e. Delete ¡°X=Y¡± on files in the given directory
	*/
	static char chDefaultModel[]="34243";
	const char*pchModel;
	if(pchAssertModel==NULL)
		pchModel=chDefaultModel;
	else
		pchModel=pchAssertModel;
	
	int idx=0,fidx=0;
	time_t tmBegin,tmEnd;
	time_t tmSessionBegin,tmSessionEnd;
	if(bOneFileByOneFile==true)
	{
		for(fidx=0;fidx<vecFiles.size();fidx++)
		{
			idx=0;
			printf("========%s \n",vecFiles[fidx].c_str()); 
			tmSessionBegin=time(NULL);
			while(pchModel[idx])
			{
				switch(pchModel[idx])
				{
				case '1':
					tmBegin=time(NULL);
					ReadTag(vecFiles[fidx].c_str(),tagList);
					tmEnd=time(NULL);
					printf("\tTime on Read:%d\n",tmEnd-tmBegin);
					break;
				case '2':
					tmBegin=time(NULL);
					WriteTag(vecFiles[fidx].c_str(),tagList);
					tmEnd=time(NULL);printf("\tTime on Write:%d\n",tmEnd-tmBegin);
					break;
				case '3':
					tmBegin=time(NULL);
					RemoveTag(vecFiles[fidx].c_str(),tagList);
					tmEnd=time(NULL);
					printf("\tTime on Remove:%d\n",tmEnd-tmBegin);
					break;
				case '4':
				{
					TagList tempTagList;
					tmBegin=time(NULL);
					ReadTag(vecFiles[fidx].c_str(),tempTagList);
					tmEnd=time(NULL);
					printf("\tTime on ReadAll:%d\n",tmEnd-tmBegin);
					break;
				}
				default:
					break;
				}
				
				idx++;
			}
			tmSessionEnd=time(NULL);
			printf("========%s [Second(s)=%d]\n",vecFiles[fidx].c_str(),tmSessionEnd-tmSessionBegin); 
		}
	}
	else
	{
		while(pchModel[idx])
		{
			switch(pchModel[idx])
			{
			case '1':
				for(fidx=0;fidx<vecFiles.size();fidx++)
					ReadTag(vecFiles[fidx].c_str(),tagList);
				break;
			case '2':
				for(fidx=0;fidx<vecFiles.size();fidx++)
					WriteTag(vecFiles[fidx].c_str(),tagList);
				break;
			case '3':
				for(fidx=0;fidx<vecFiles.size();fidx++)
					RemoveTag(vecFiles[fidx].c_str(),tagList);
				break;
			case '4':
			{
				TagList tempTagList;
				for(fidx=0;fidx<vecFiles.size();fidx++)
					ReadTag(vecFiles[fidx].c_str(),tempTagList);
				break;
			}
			default:
				break;
			}
			
			idx++;
		}
	}
	
}
enum OPCODE{UNKNOWN,GET=1,SET,REMOVE,ASSERT};

int ZipMain(int argc,char*argv[]);

int GetAllFilesInDir(const char*pchDir,std::vector<std::string>&vecFiles,bool bRecursive=false,bool bIncludeDir=false)
{
	GError* err=NULL;
	GDir* dir=g_dir_open(pchDir,0,&err);
	if (dir == NULL)
	{
		g_return_val_if_fail (err != NULL, 1);
		g_warning ("'%s' error: %s",pchDir, err->message);
		g_error_free (err);
		return 1;
	}
	const gchar* file=g_dir_read_name(dir);
	std::string strEntry;
	while(file)
	{
		strEntry=pchDir;
		strEntry+="/";
		strEntry+=file;
		if(!g_file_test(strEntry.c_str(),G_FILE_TEST_IS_DIR))
			vecFiles.push_back(strEntry.c_str());
		else
		{
			if(bIncludeDir==true)
				vecFiles.push_back(strEntry.c_str());
			if(bRecursive==true)
				GetAllFilesInDir(strEntry.c_str(),vecFiles,true);
		}
		file=	g_dir_read_name(dir);
	}
	g_dir_close(dir);
	return 0;
}

static void CloneMSOleStorage (GsfInfile *in, GsfOutfile *out);

static void CloneMSOleStream (GsfInput *input, GsfOutput *output)
{
	if (gsf_input_size (input) > 0) {
		guint8 const *data;
		size_t len;

		while ((len = gsf_input_remaining (input)) > 0) {
			/* copy in odd sized chunks to exercise system */
			if (len > 314)
				len = 314;
			if (NULL == (data = gsf_input_read (input, len, NULL))) {
				g_warning ("error reading ?");
				return;
			}
			if (!gsf_output_write (output, len, data)) {
				g_warning ("error writing ?");
				return;
			}
		}
	} else if (GSF_IS_INFILE(input))
		CloneMSOleStorage (GSF_INFILE(input), GSF_OUTFILE(output));

	gsf_output_close (output);
	g_object_unref (G_OBJECT (output));
	g_object_unref (G_OBJECT (input));
}

static void CloneMSOleStorage (GsfInfile *in, GsfOutfile *out)
{
	GsfInput *new_input;
	GsfOutput *new_output;
	gboolean is_dir;
	int i;

	for (i = 0 ; i < gsf_infile_num_children (in) ; i++) 
	{
		new_input = gsf_infile_child_by_index (in, i);

		/* In theory, if new_file is a regular file (not directory),
		 * it should be GsfInput only, not GsfInfile, as it is not
		 * structured.  However, having each Infile define a 2nd class
		 * that only inherited from Input was cumbersome.  So in
		 * practice, the convention is that new_input is always
		 * GsfInfile, but regular file is distinguished by having -1
		 * children.
		 */
		is_dir = GSF_IS_INFILE (new_input) &&
			gsf_infile_num_children (GSF_INFILE (new_input)) >= 0;
		const gchar *name=gsf_infile_name_by_index(in, i);
		if(strcmp(name,"MsoDataStore")==0)
		{
			printf("The name is %s\n",name);
		}
		else
		{
			new_output = gsf_outfile_new_child  (out,name,is_dir);

			CloneMSOleStream (new_input, new_output);
		}
	}
	/* An observation: when you think about the explanation to is_dir
	 * above, you realize that clone_dir is called even for regular files.
	 * But nothing bad happens, as the loop is never entered.
	 */
}

int CloneMSOleTest(const char*pchSrcFile,const char*pchDestFile)
{
	GsfInput   *input;
	GsfInfile  *infile;
	GsfOutput  *output;
	GsfOutfile *outfile;
	GError    *err = NULL;

	gsf_init();
	fprintf (stderr, "%s\n", pchSrcFile);
	input = gsf_input_stdio_new (pchSrcFile, &err);
	if (input == NULL) {
		g_return_val_if_fail (err != NULL, 1);

		g_warning ("'%s' error: %s", pchSrcFile, err->message);
		g_error_free (err);
		return 1;
	}

	infile = gsf_infile_msole_new (input, &err);
	g_object_unref (G_OBJECT (input));

	if (infile == NULL) {
		g_return_val_if_fail (err != NULL, 1);

		g_warning ("'%s' Not an OLE file: %s", pchSrcFile, err->message);
		g_error_free (err);
		return 1;
	}

	output = gsf_output_stdio_new (pchDestFile, &err);
	if (output == NULL) {
		g_return_val_if_fail (err != NULL, 1);

		g_warning ("'%s' error: %s", pchDestFile, err->message);
		g_error_free (err);
		g_object_unref (G_OBJECT (infile));
		return 1;
	}

	outfile = gsf_outfile_msole_new (output);
	g_object_unref (G_OBJECT (output));
	CloneMSOleStream (GSF_INPUT (infile), GSF_OUTPUT (outfile));

	gsf_shutdown();
	return 0;
}	

int main(int argc, char *argv[])
{
	//return TestXLSX(argv[1]);
	printf("libgsf version is %d.%d.%d\n",libgsf_major_version,libgsf_minor_version,libgsf_micro_version);
	if(argc==3)
		return CloneMSOleTest(argv[1],argv[2]);
	//return ZipMain(argc,argv);
	int 	iRet=0;
	char *	pchProgName = argv[0];
	OPCODE 	opCode=UNKNOWN;
	bool 	bParaError=false;
	std::string strFileName,strDir;
	std::string strTagName,strTagValue;
	std::string strAssertModel;
	TagList		tagList;
	for(int iArg=1;iArg<argc;iArg++)
	{
		if(strcasecmp(argv[iArg],"-read")==0||strcasecmp(argv[iArg],"-get")==0)
		{
			if(opCode!=UNKNOWN)
				bParaError=true;
			opCode=GET;
		}
		else if(strcasecmp(argv[iArg],"-write")==0||strcasecmp(argv[iArg],"-set")==0)
		{
			if(opCode!=UNKNOWN)
				bParaError=true;
			opCode=SET;
		}
		else if(strcasecmp(argv[iArg],"-remove")==0)
		{
			if(opCode!=UNKNOWN)
				bParaError=true;
			opCode=REMOVE;
		}
		else if(strcasecmp(argv[iArg],"-assert")==0)
		{
			if(opCode!=UNKNOWN)
				bParaError=true;
			opCode=ASSERT;
			if(argv[iArg+1][0]!='-')
				strAssertModel=argv[++iArg];
		}
		else if(strcasecmp(argv[iArg],"-file")==0)
		{
			if(iArg+1<argc&&argv[iArg+1][0]!='-')
				strFileName=argv[++iArg];
			else
				bParaError=true;
		}
		else if(strcasecmp(argv[iArg],"-dir")==0)
		{
			if(iArg+1<argc&&argv[iArg+1][0]!='-')
				strDir=argv[++iArg];
			else
				bParaError=true;
		}
		else if(strcasecmp(argv[iArg],"-name")==0)
		{
			if(strTagName.length()==0)
			{
				if(iArg+1<argc&&argv[iArg+1][0]!='-')
					strTagName=argv[++iArg];
				else
					bParaError=true;
			}
			else
				bParaError=true;
		}
		else if(strcasecmp(argv[iArg],"-value")==0)
		{
			if(strTagValue.length()==0)
			{
				if(iArg+1<argc&&argv[iArg+1][0]!='-')
					strTagValue=argv[++iArg];
				else
					bParaError=true;
			}
			else
				bParaError=true;
		}
		else if(strcasecmp(argv[iArg],"-tag")==0)
		{
			if(iArg+1<argc&&argv[iArg+1][0]!='-')
			{
				std::string strKeyValue=argv[++iArg];
				std::string::size_type pos=strKeyValue.find('=');
				NVPair keyvalue;
				if(pos==std::string::npos)
					keyvalue.first=strKeyValue;
				else
				{
					keyvalue.first=strKeyValue.substr(0,pos);
					keyvalue.second=strKeyValue.substr(pos+1);
				}
				tagList.push_back(keyvalue);
			}
			else
				bParaError=true;
		}
	}
	
	if(strTagName.length())
		tagList.push_back(NVPair(strTagName,strTagValue));
	
	int iTagCount=tagList.size();
	if(iTagCount)
		printf("Tag List:\n");
	for(int i=0;i<iTagCount;i++)
		printf("  %s=%s\n",tagList[i].first.c_str(),tagList[i].second.c_str());
	
	std::vector<std::string> vecFiles;
	if(strDir.length())
	{
		if(!g_file_test(strDir.c_str(),G_FILE_TEST_IS_DIR))
		{
			g_error("Error: You must input a directory for -dir.\n");
			Usage(pchProgName);
			return 1;
		}
		GetAllFilesInDir(strDir.c_str(),vecFiles,true);		
	}
	if(strFileName.length())
		vecFiles.push_back(strFileName);
	if(vecFiles.size()==0)
		bParaError=true;
	if(opCode==UNKNOWN)
		bParaError=true;
	if(tagList.size()==0&&opCode!=GET)
		bParaError=true;

	if(bParaError==true)
	{
		Usage(pchProgName);
		return 1;
	}

	for(int i=0;i<vecFiles.size();i++)
		printf("File:%s\n",vecFiles[i].c_str());
	
	if(opCode==ASSERT)
		iRet=AssertTag(vecFiles,tagList,strAssertModel.length()==0?NULL:strAssertModel.c_str());
	else
	{
		for(int idx=0;idx<vecFiles.size();idx++)
		{
			switch(opCode)
			{
			case GET:
				iRet=ReadTag(vecFiles[idx].c_str(),tagList);
				break;
			case SET:
				iRet=WriteTag(vecFiles[idx].c_str(),tagList);
				break;
			case REMOVE:
				iRet=RemoveTag(vecFiles[idx].c_str(),tagList);
				break;
			default:
				break;
			}
		}
	}
	

	return iRet;
}
static void ls_R (GsfInput *input)
{
	int i;
	char const *name = gsf_input_name (GSF_INPUT (input));
	gboolean is_dir = GSF_IS_INFILE (input) &&
		(gsf_infile_num_children (GSF_INFILE (input)) >= 0);
	
	printf ("%c '%s'\t\t%" GSF_OFF_T_FORMAT "\n",
		(is_dir ? 'd' : ' '),
		(name != NULL) ? name : "",
		gsf_input_size (GSF_INPUT (input)));

	if (is_dir) {
		puts ("{");
		for (i = 0 ; i < gsf_infile_num_children (GSF_INFILE (input)) ; i++)
			ls_R (gsf_infile_child_by_index (GSF_INFILE (input), i));
		puts ("}");
	}

	g_object_unref (G_OBJECT (input));
}
int ZipTest(int argc, char*argv[])
{
		GsfInput  *input;
	GsfInfile *infile;
	GError    *err = NULL;

	fprintf (stderr, "%s\n", argv [1]);
	input = gsf_input_stdio_new (argv[1], &err);
	if (input == NULL) {

		g_return_val_if_fail (err != NULL, 1);

		g_warning ("'%s' error: %s", argv[1], err->message);
		g_error_free (err);
		return 1;
	}

	input = gsf_input_uncompress (input);
	infile = gsf_infile_zip_new (input, &err);
	g_object_unref (G_OBJECT (input));

	if (infile == NULL) {
		g_return_val_if_fail (err != NULL, 1);

		g_warning ("'%s' Not an OLE file: %s", argv[1], err->message);
		g_error_free (err);
		return 1;
	}

	if (argc > 2) {
		int i;
		GsfInput *child, *ptr = GSF_INPUT (infile);
		for (i = 2 ; i < argc && ptr != NULL; i++, ptr = child) {
			fprintf (stderr, "--> '%s'\n", argv [i]);
			if (GSF_IS_INFILE (ptr) &&
			    gsf_infile_num_children (GSF_INFILE (ptr)) >= 0) {
				child = gsf_infile_child_by_name (GSF_INFILE (ptr), argv [i]);

				if (child == NULL) {
					g_warning ("No child named '%s'", argv [i]);
					child = NULL;
					break;
				}
			} else {
				g_warning ("stream is not a directory '%s'", argv [i]);
				child = NULL;
				break;
			}
			g_object_unref (G_OBJECT (ptr));
		}
		if (ptr != NULL) {
			if (GSF_IS_INFILE (ptr) &&
			    gsf_infile_num_children (GSF_INFILE (ptr)) >= 0)
				ls_R (ptr); /* unrefs infile */
			else {
				gsf_input_dump (GSF_INPUT (ptr), FALSE);
				g_object_unref (G_OBJECT (ptr));
			}
		}
	} else
		ls_R (GSF_INPUT (infile)); /* unrefs infile */

	return 0;
}
int ZipMain(int argc,char*argv[])
{
	int res;

	if (argc < 2) {
		fprintf (stderr, "%s : file stream stream ...\n", argv [0]);
		return 1;
	}

	gsf_init ();
	res = ZipTest (argc, argv);
	gsf_shutdown ();

	return res;
	
}

