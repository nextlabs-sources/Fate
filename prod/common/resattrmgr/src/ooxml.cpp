#include "ooxml.h"
#include <errno.h>
#include <gsf/gsf-utils.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-infile-zip.h>
#include <gsf/gsf-outfile-zip.h>
#include <gsf/gsf-libxml.h>

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_OOXMLCPP)

namespace NSLinuxTagLib
{



void OOXMLFileTagging::XMLPropertyElementStart (GsfXMLIn *xin, xmlChar const **attrs)
{
	bool bIsCustPropFMTID=false;
	char const *name = NULL;
	PropReadState*pReadState=(PropReadState*)xin->user_state;
	for (; attrs != NULL && attrs[0] && attrs[1] ; attrs += 2)
	{
		if (0 == strcmp ((const char*)attrs[0], "name"))
			name =(char*) attrs[1];
		else if (0==strcmp((const char*)attrs[0],"fmtid")&&0==strcmp((const char*)attrs[1],FMTID_CUSTPROPS))
			bIsCustPropFMTID=true;
		else if (0 == strcmp ((const char*)attrs[0], "pid"))
		{
			int i=atoi((const char*)attrs[1]);
			if(i>pReadState->iBigestPid)
				pReadState->iBigestPid=i;
		}
	}
	pReadState->pchCurrAttrName=NULL;
	pReadState->iAttrIdx=-1;
	//we only care those custom property with fmtid={D5CDD505-2E9C-101B-9397-08002B2CF9AE}
	if(bIsCustPropFMTID==true)
	{
		if(pReadState->bReadAll==true)
			pReadState->pchCurrAttrName=strdup(name);
		else
			FindAttribute(pReadState->pAttr,name,pReadState->iAttrIdx);
	}
}
void OOXMLFileTagging::XMLPropertyElementEnd   (GsfXMLIn *xin, GsfXMLBlob *unknown)
{
	//printf("xml_prop_end\n");
}
void OOXMLFileTagging::XMLPropValueElementStart (GsfXMLIn *xin, xmlChar const **attrs)
{
	//for (; attrs != NULL && attrs[0] && attrs[1] ; attrs += 2)
	//	printf("%s=%s\n",attrs[0],attrs[1]);
}

void OOXMLFileTagging::XMLPropValueElementEnd_str   (GsfXMLIn *xin, GsfXMLBlob *unknown)
{
	PropReadState*pReadState=(PropReadState*)xin->user_state;
	if(pReadState->iAttrIdx==-1&&pReadState->pchCurrAttrName==NULL)
		return;
	if(pReadState->pchCurrAttrName)
	{
		AddAttribute(pReadState->pAttr,pReadState->pchCurrAttrName,xin->content->str);
		free(pReadState->pchCurrAttrName);
		pReadState->pchCurrAttrName=NULL;
		return;
	}
	SetAttributeValue(pReadState->pAttr,pReadState->iAttrIdx,xin->content->str);
	pReadState->iAttrIdx=-1;
	return;
}
void OOXMLFileTagging::XMLPropValueElementEnd_bool   (GsfXMLIn *xin, GsfXMLBlob *unknown)
{
	PropReadState*pReadState=(PropReadState*)xin->user_state;
	if(pReadState->iAttrIdx==-1&&pReadState->pchCurrAttrName==NULL)
		return;
	if(pReadState->pchCurrAttrName)
	{
		if(strcasecmp(xin->content->str,"false")==0)
			AddAttribute(pReadState->pAttr,pReadState->pchCurrAttrName,"No");
		else
			AddAttribute(pReadState->pAttr,pReadState->pchCurrAttrName,"Yes");
		free(pReadState->pchCurrAttrName);
		pReadState->pchCurrAttrName=NULL;
		return;
	}
	if(strcasecmp(xin->content->str,"false")==0)
		SetAttributeValue(pReadState->pAttr,pReadState->iAttrIdx,"No");
	else
		SetAttributeValue(pReadState->pAttr,pReadState->iAttrIdx,"Yes");
	
	pReadState->iAttrIdx=-1;
	return;
	//if(pReadState->bIgnoreAtPropValueEnd==true)
	//	pReadState->bIgnoreAtPropValueEnd=false;
}


const char OOXMLFileTagging::XML_ENCODING_DEFAULT[]="UTF-8";	

const char OOXMLFileTagging::RELTYPE_OFFICE_DOCUMENT[]="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument";
const char OOXMLFileTagging::RELTYPE_CUSTOM_PROPERTIES[]="http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties";
const char OOXMLFileTagging::RELTARGET_OFFICE_DOCUMENT_XLSX[]="xl/workbook.xml";
const char OOXMLFileTagging::RELTARGET_OFFICE_DOCUMENT_XLSB[]="xl/workbook.bin";
const char OOXMLFileTagging::RELTARGET_OFFICE_DOCUMENT_DOCX[]="word/document.xml";
const char OOXMLFileTagging::RELTARGET_OFFICE_DOCUMENT_PPTX[]="ppt/presentation.xml";

const char OOXMLFileTagging::FMTID_CUSTPROPS[]="{D5CDD505-2E9C-101B-9397-08002B2CF9AE}";
const char OOXMLFileTagging::XML_CUSTPROPS_ELE_PROPERTIES[]="Properties";
const char OOXMLFileTagging::XML_CUSTPROPS_ELE_PROPERTIE[]="property";
const char OOXMLFileTagging::XML_CUSTPROPS_ELEPROP_FMTID[]="fmtid";
const char OOXMLFileTagging::XML_CUSTPROPS_ELEPROP_NAME[]="name";
const char OOXMLFileTagging::XML_CUSTPROPS_ELEPROP_PID[]="pid";
const char OOXMLFileTagging::XML_CUSTPROPS_FILENAME[]="custom.xml";
const char OOXMLFileTagging::XML_CUSTPROPS_DIRNAME[]="docProps";

const char OOXMLFileTagging::XML_RELS_FILENAME[]=".rels";
const char OOXMLFileTagging::XML_RELS_DIRNAME[]="_rels";
const char OOXMLFileTagging::XML_RELS_ELE_RELATIONSHIP[]="Relationship";
const char OOXMLFileTagging::XML_RELS_ELEPROP_TYPE[]="Type";
const char OOXMLFileTagging::XML_RELS_ELEPROP_TYPE_VALUE_CUSTPROPS[]="http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties";
const char OOXMLFileTagging::XML_RELS_ELEPROP_ID[]="Id";
const char OOXMLFileTagging::XML_RELS_ELEPROP_TARGET[]="Target";
const char OOXMLFileTagging::XML_RELS_ELEPROP_TARGET_VALUE_CUSTPROPS[]="docProps/custom.xml";

		
const char OOXMLFileTagging::XML_CONTTYPE_FILENAME[]="[Content_Types].xml";
const char OOXMLFileTagging::XML_CONTTYPE_ELE_OVERRIDE[]="Override";
const char OOXMLFileTagging::XML_CONTTYPE_ELEPROP_PARTNAME[]="PartName";
const char OOXMLFileTagging::XML_CONTTYPE_ELEPROP_PARTNAME_VALUE_CUSTPROPS[]="/docProps/custom.xml";
const char OOXMLFileTagging::XML_CONTTYPE_ELEPROP_CONTENTTYPE[]="ContentType";
const char OOXMLFileTagging::XML_CONTTYPE_ELEPROP_CONTENTTYPE_VALUE_CUSTPROPS[]="application/vnd.openxmlformats-officedocument.custom-properties+xml";
	
		
const char OOXMLFileTagging::GSFOBJECT_PROPERTY_COMPRESSLEVEL[]="compression-level";
const char OOXMLFileTagging::OOXML_MAGIC_PROPVALUE[]="~!@#$%^&*()";

static char const *custprops_ns_uri	 = "http://schemas.openxmlformats.org/officeDocument/2006/custom-properties";  
static char const *propvtype_ns_uri	 = "http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes";  

const GsfXMLInNS   OOXMLFileTagging::gooxml_custprops_ns[] = {
	GSF_XML_IN_NS (OOXML_CUSTPROPS_NS,	custprops_ns_uri),
	GSF_XML_IN_NS (OOXML_PROPVTYPE_NS,	propvtype_ns_uri),
	{ NULL }
};
const GsfXMLInNode OOXMLFileTagging::gooxml_custprops_dtd[] = {
GSF_XML_IN_NODE_FULL (START, START, -1, NULL, GSF_XML_NO_CONTENT, FALSE, TRUE, NULL, NULL, 0),
GSF_XML_IN_NODE_FULL (START, PROPS, OOXML_CUSTPROPS_NS, "Properties", GSF_XML_NO_CONTENT, FALSE, TRUE, NULL, NULL, 0),
	GSF_XML_IN_NODE  (PROPS, PROP, OOXML_CUSTPROPS_NS, "property", GSF_XML_NO_CONTENT, &XMLPropertyElementStart, &XMLPropertyElementEnd),
		GSF_XML_IN_NODE (PROP, VECTOR, 	OOXML_PROPVTYPE_NS, "vector", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, BLOB, 	OOXML_PROPVTYPE_NS, "blob", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, OBLOB, 	OOXML_PROPVTYPE_NS, "oblob", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, EMPTY, 	OOXML_PROPVTYPE_NS, "empty", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, NULL, 	OOXML_PROPVTYPE_NS, "null", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, I1	, 	OOXML_PROPVTYPE_NS, "i1", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, I2	, 	OOXML_PROPVTYPE_NS, "i2", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, I4	, 	OOXML_PROPVTYPE_NS, "i4", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, I8	, 	OOXML_PROPVTYPE_NS, "i8", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, INT, 	OOXML_PROPVTYPE_NS, "int", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, UI1, 	OOXML_PROPVTYPE_NS, "ui1", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, UI2, 	OOXML_PROPVTYPE_NS, "ui2", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, UI4, 	OOXML_PROPVTYPE_NS, "ui4", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, UI8, 	OOXML_PROPVTYPE_NS, "ui8", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, UINT, 	OOXML_PROPVTYPE_NS, "uint", 	GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, R4	, 	OOXML_PROPVTYPE_NS, "r4", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, R8	, 	OOXML_PROPVTYPE_NS, "r8", 		GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, DECIMAL,	OOXML_PROPVTYPE_NS, "decimal", 	GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, LPSTR, 	OOXML_PROPVTYPE_NS, "lpstr", 	GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, LPWSTR, 	OOXML_PROPVTYPE_NS, "lpwstr", 	GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, BSTR, 	OOXML_PROPVTYPE_NS, "bstr", 	GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, DATE, 	OOXML_PROPVTYPE_NS, "date", 	GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, FILETIME,OOXML_PROPVTYPE_NS, "filetime", GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, BOOL, 	OOXML_PROPVTYPE_NS, "bool", 	GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_bool),
		GSF_XML_IN_NODE (PROP, CY	, 	OOXML_PROPVTYPE_NS, "cy", 		GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, ERROR, 	OOXML_PROPVTYPE_NS, "error", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, STREAM, 	OOXML_PROPVTYPE_NS, "stream", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, OSTREAM,	OOXML_PROPVTYPE_NS, "ostream", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, STORAGE,	OOXML_PROPVTYPE_NS, "storage", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, OSTORAGE,OOXML_PROPVTYPE_NS, "ostorage", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, VSTREAM,	OOXML_PROPVTYPE_NS, "vstream", 	GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PROP, CLSID, 	OOXML_PROPVTYPE_NS, "clsid", 	GSF_XML_CONTENT, XMLPropValueElementStart, XMLPropValueElementEnd_str),
		GSF_XML_IN_NODE (PROP, CF	, 	OOXML_PROPVTYPE_NS, "cf", 		GSF_XML_CONTENT, NULL, NULL),

GSF_XML_IN_NODE_END
};

OOXMLFileTagging::OOXMLFileTagging()
{
	gsf_init ();
}
OOXMLFileTagging::~OOXMLFileTagging()
{
	gsf_shutdown ();
}
int gIndent=0;
void OOXMLFileTagging::ForeachOOXMLRel(GsfInput *opkg,GsfOpenPkgRel const *rel,gpointer    user_data)
{
	const char*pch;
	char tabs[32]="";
	for(int i=0;i<gIndent;i++)
		strncat_s(tabs,32,"\t", _TRUNCATE);
	printf("%s====================================!\n",tabs);
	printf("%sType:%s\n",tabs,(pch=gsf_open_pkg_rel_get_type(rel))==NULL?"null":pch);
	printf("%sTarget:%s\n",tabs,(pch=gsf_open_pkg_rel_get_target(rel))==NULL?"null":pch);
	printf("%sExtern:%s\n",tabs,gsf_open_pkg_rel_is_extern(rel)==true?"Yes":"No");
	
	GError*err=NULL;
	if(opkg!=NULL&&rel!=NULL)
	{
		{
			GsfInput      *relInput=gsf_open_pkg_open_rel_by_type(opkg, gsf_open_pkg_rel_get_type(rel),&err);
			if(relInput)
			{
				pch=NULL;
				g_object_get (G_OBJECT ((GsfOpenPkgRel*)rel), "content-type", &pch, NULL);
			}
			
		}
		GsfInput*input=gsf_open_pkg_open_rel(opkg,rel,&err);
		
		if(input==NULL)
		{
			g_return_if_fail (err != NULL);
			g_warning ("'%s' error: %s",(pch=gsf_open_pkg_rel_get_type(rel))==NULL?"null":pch, err->message);
			g_error_free (err);
			return;
		}
		pch=gsf_input_name(input);
		printf("%sInput name:%s\n",tabs,pch==NULL?"null":pch);
		gIndent++;
		gsf_open_pkg_foreach_rel(input,(GsfOpenPkgIter)ForeachOOXMLRel,NULL);
		gIndent--;
	}
	return ;
}
BOOL OOXMLFileTagging::IsOOXML(const char*pchFileName)
{
	GsfInput *input;
	GsfInfile *infile;
	GError    *err = NULL;
	gsf_init ();
	const char*pch;
	input = gsf_input_stdio_new (pchFileName, &err);
	
	if (input == NULL)
	{
		g_return_val_if_fail (err != NULL, 1);
		//g_warning ("'%s' error: %s",pchFileName, err->message);
		g_error_free (err);
		return FALSE;
	}
	
	infile = gsf_infile_zip_new (input, &err);
	g_object_unref (G_OBJECT (input));
	BOOL bRet = FALSE;
	if (infile == NULL) 
	{
		g_return_val_if_fail (err != NULL, 1);
		//g_warning ("'%s' Not an Office Open XML file: %s", pchFileName, err->message);
		g_error_free (err);
		return bRet;
	}
	GsfOpenPkgRel* relSignature=gsf_open_pkg_lookup_rel_by_type(GSF_INPUT(infile),RELTYPE_OFFICE_DOCUMENT);
	if(relSignature==NULL)
		return FALSE;
	const char*pname=gsf_open_pkg_rel_get_target(relSignature);
	
	if(pname==NULL)
		return FALSE;
	if(bRet==FALSE&&strcasecmp(pname,RELTARGET_OFFICE_DOCUMENT_XLSX)==0)
		bRet=TRUE;
	if(bRet==FALSE&&strcasecmp(pname,RELTARGET_OFFICE_DOCUMENT_XLSB)==0)
		bRet=TRUE;
	if(bRet==FALSE&&strcasecmp(pname,RELTARGET_OFFICE_DOCUMENT_DOCX)==0)
		bRet=TRUE;
	if(bRet==FALSE&&strcasecmp(pname,RELTARGET_OFFICE_DOCUMENT_PPTX)==0)
		bRet=TRUE;
	g_object_unref (G_OBJECT (infile));
	return bRet;
}
GsfInfile* 	OOXMLFileTagging::GetGsfInfile(GsfInput*input,GError**err)
{
	return  gsf_infile_zip_new (input, err);
}
GsfOutfile* 	OOXMLFileTagging::GetGsfOutfile(GsfOutput*output,GError**err)
{
	return  gsf_outfile_zip_new (output, err);
}
BOOL OOXMLFileTagging::GetTags(void*pluginData,const char *pchFileName, ResourceAttributes *pAttr)
{NLCELOG_ENTER
	GError* err=NULL;
	GsfInfile* infile = FileTagging::GetGsfInfile(pchFileName,&err);

	if (infile == NULL) 
		NLCELOG_RETURN_VAL( FALSE )
	
	GsfInput* relCustomProp=gsf_open_pkg_open_rel_by_type(GSF_INPUT(infile),RELTYPE_CUSTOM_PROPERTIES,&err);
	if(!relCustomProp)
		NLCELOG_RETURN_VAL( FALSE )
	//printf("\n====Custom Properties==============================\n");
	//printf("find rel %s\n",RELTYPE_CUSTOM_PROPERTIES);
	//gsf_input_dump(GSF_INPUT(relCustomProp),false);
	//gsf_input_seek(relCustomProp,0,G_SEEK_SET);
	//printf("\n===================================================\n");

	GsfXMLInDoc *doc = gsf_xml_in_doc_new (gooxml_custprops_dtd, 
											gooxml_custprops_ns);
	if(NULL==doc)
		NLCELOG_RETURN_VAL( FALSE )
	
	PropReadState state;
	memset(&state,0,sizeof(state));
	state.pAttr=pAttr;
	state.pchCurrAttrName=NULL;
	state.iAttrIdx=-1;
	state.iBigestPid=0;
	//state.bIgnoreAtPropValueEnd=false;
	if(GetAttributeCount(pAttr))
		state.bReadAll=false;
	else
		state.bReadAll=true;
	gboolean  success =gsf_xml_in_doc_parse (doc, relCustomProp, &state);
	gsf_xml_in_doc_free(doc);
	if(!success)
		NLCELOG_RETURN_VAL( FALSE )
	//int child_count=gsf_infile_num_children (infile);
	//printf("child count=%d\n",child_count);
	//for(int i=0;i<child_count;i++)
	//{
	//	int j=gsf_infile_num_children((GsfInfile*)gsf_infile_child_by_index (infile,i));
	//	printf("Child %d's name[%d]:%s\n",i+1,j,gsf_infile_name_by_index (infile,i));
	//}
	//gIndent=0;
	//gsf_open_pkg_foreach_rel(GSF_INPUT(infile),(GsfOpenPkgIter)ForeachOOXMLRel,NULL);
	//printf("This is an OOXML file\n");
	NLCELOG_RETURN_VAL( TRUE )
}

//static BOOL TestXML()
//{
//	unsigned char sArr[]="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?> \
//		<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties\" xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">\
//		<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"2\" name=\"SetTags\">\
//		<vt:lpwstr>OMGss</vt:lpwstr> \
//		</property>\
//		</Properties>";
//	GsfInput* 	gsfMemInput=gsf_input_memory_new (sArr,sizeof(sArr),false);
//	if(gsfMemInput==NULL)
//		return FALSE;
//	GsfOutput* 	output = gsf_output_stdio_new ("test.xml", NULL);
//	if(output==NULL)
//		return FALSE;
//	
//	GsfXMLOut *xmlout=gsf_xml_out_new (output);
//	g_object_unref (G_OBJECT (output));
//	if(xmlout==NULL)
//		return FALSE;
//	gsf_input_copy(gsfMemInput,GSF_OUTPUT(xmlout));
//	g_object_unref (G_OBJECT (gsfMemInput));
//	gsf_output_close (output);
//	g_object_unref (G_OBJECT (xmlout));
//	return TRUE;
//	
//} 

//libgsf 1.14.18
//typedef enum {
//	GSF_XML_OUT_NOCONTENT,
//	GSF_XML_OUT_CHILD,
//	GSF_XML_OUT_CONTENT
//} tagGsfXMLOutState;
//struct _tagGsfXMLOut {
//	GObject	   base;
//
//	GsfOutput	 *output;
//	char		 *doc_type;
//	GSList		 *stack;
//	tagGsfXMLOutState	  state;
//	unsigned   	  indent;
//	gboolean	  needs_header;
//	gboolean	  pretty_print;
//};
BOOL OOXMLFileTagging::CreateCustomPropsXML(GsfOutput*output,ResourceAttributes*pAttr)
{
	GsfXMLOut *xml=gsf_xml_out_new (output);
	
	//((_tagGsfXMLOut*)xml)->needs_header=false;
	//static unsigned char const header0[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n";
	//gsf_output_write (((_tagGsfXMLOut*)xml)->output, sizeof (header0) - 1, header0);
	//((_tagGsfXMLOut*)xml)->needs_header=true;
	
	gsf_xml_out_start_element (xml, XML_CUSTPROPS_ELE_PROPERTIES);
	gsf_xml_out_add_cstr_unchecked (xml, "xmlns", custprops_ns_uri);
 	gsf_xml_out_add_cstr_unchecked (xml, "xmlns:vt", propvtype_ns_uri);
 	int idx=0,lCount=GetAttributeCount(pAttr);
 	for(idx=0;idx<lCount;idx++)
 	{
 		gsf_xml_out_start_element (xml, XML_CUSTPROPS_ELE_PROPERTIE);
 		gsf_xml_out_add_cstr (xml, XML_CUSTPROPS_ELEPROP_FMTID, FMTID_CUSTPROPS);
		gsf_xml_out_add_int (xml, XML_CUSTPROPS_ELEPROP_PID, idx+2);
		gsf_xml_out_add_cstr (xml, XML_CUSTPROPS_ELEPROP_NAME, GetAttributeName(pAttr,idx));
			gsf_xml_out_simple_element(xml,"vt:lpwstr",GetAttributeValue(pAttr,idx));
		gsf_xml_out_end_element (xml);//property
	}
 	gsf_xml_out_end_element (xml);//Properties
	gsf_output_close (output);
	g_object_unref (G_OBJECT (xml));
	g_object_unref (G_OBJECT (output));
}
BOOL OOXMLFileTagging::ModifyTags(void*pluginData,const char *pchFileName, ResourceAttributes *pAttr,bool bRemove)
{NLCELOG_ENTER
	GError* err=NULL;
	GsfInfile* infile = FileTagging::GetGsfInfile(pchFileName,&err);

	if (infile == NULL) 
		NLCELOG_RETURN_VAL( FALSE )
	
	PropWriteState writeState;
	memset(&writeState,0,sizeof(writeState));
	writeState.pAttr=pAttr;
	
	GsfInput* custPropsRel=gsf_open_pkg_open_rel_by_type(GSF_INPUT(infile),RELTYPE_CUSTOM_PROPERTIES,&err);
	if(!custPropsRel)
	{
		writeState.bCustPropsRelExist=false;
	}
	else
	{
		writeState.bCustPropsRelExist=true;
		//writeState.gsfInputCustPropsRel=custPropsRel;
	}

	std::string strNewFileName,strFullPath;
	strNewFileName=pchFileName;
	std::string::size_type pos=strNewFileName.rfind("/");
	if(pos!=std::string::npos)
		strFullPath=strNewFileName.substr(0,pos+1);
	else
		strFullPath="";
	if(NSLinuxTagLib::Utils::GetTempFile(pchFileName,strNewFileName)==FALSE)
	{
		g_object_unref (G_OBJECT (infile));
		NLCELOG_RETURN_VAL( FALSE )
	}

	//strFullPath+=strNewFileName;
	
	GsfOutput* output= gsf_output_stdio_new (strNewFileName.c_str(), NULL);
	
	if (output == NULL)
		NLCELOG_RETURN_VAL( FALSE )
	
	GsfOutfile*outfile =  gsf_outfile_zip_new (output, NULL);
	g_object_unref (G_OBJECT (output));
	
	if (outfile == NULL) 
		NLCELOG_RETURN_VAL( FALSE )
	writeState.iCurrDirDepth=1;
	writeState.bRemove=bRemove;
	if( Clone(infile,outfile,&writeState)==TRUE)
	{
		std::string strCmd="mv -f ";
		strCmd+=strNewFileName;
		strCmd+=" ";
		strCmd+=pchFileName;
		
		int iRet = system(strCmd.c_str());

		//if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
		//	break;
    if(iRet)
    {
    	remove(strNewFileName.c_str());
    	NLCELOG_RETURN_VAL( FALSE )
    }
    else
    	NLCELOG_RETURN_VAL( TRUE )
		//int iRet=rename(strFullPath.c_str(),pchFileName);
		//if(iRet)
		//{
		//	printf("rename failed. errno=%d\n",errno);
		//	perror("rename error");
		//	remove(strFullPath.c_str());
		//	return FALSE;
		//}
		//else
		//	return TRUE;
	}
	else
	{
		remove(strNewFileName.c_str());
		NLCELOG_RETURN_VAL( FALSE )
	}
}
BOOL OOXMLFileTagging::SetTags(void*pluginData,const char *pchFileName, ResourceAttributes *pAttr)
{
	return ModifyTags(pluginData,pchFileName,pAttr,false);	
}
BOOL OOXMLFileTagging::RemoveTags(void*pluginData,const char *pchFileName, ResourceAttributes *pAttr)
{
	return ModifyTags(pluginData,pchFileName,pAttr,true);	
}
BOOL OOXMLFileTagging::ModifyXMLContentTypes(GsfInput*in,GsfOutput*out)
{
	bool bElementExist=false;
	xmlDocPtr doc;
	gsf_off_t len=gsf_input_size(in);
	const char* content=(const char*)gsf_input_read(in,len,NULL);
	//printf("[Content_Types].xml:%s\n",content);
	if(len==-1||len==0||content==NULL)
		return FALSE;
	doc=xmlReadMemory(content,len,XML_ENCODING_DEFAULT,NULL,0);
	if(doc==NULL)
		return FALSE;
	{
		xmlNode*root=xmlDocGetRootElement(doc);
		xmlNode*child=NULL;
		if(root)
			child=root->children;
		while(child)
		{
			const char* pPartNameValue=(const char*)xmlGetProp(child,(const xmlChar*)XML_CONTTYPE_ELEPROP_PARTNAME);
			if(pPartNameValue&&strcasecmp(pPartNameValue,XML_CONTTYPE_ELEPROP_PARTNAME_VALUE_CUSTPROPS)==0)
			{
				bElementExist=true;
				break;
			}
			child=child->next;
		}	
	}
	if(bElementExist==false)
	{
		xmlNodePtr root=xmlDocGetRootElement(doc);
		//printf("Root element name:%s\n",root->name);
		xmlNodePtr node=xmlNewChild(root,NULL,(const xmlChar*)XML_CONTTYPE_ELE_OVERRIDE,NULL);
		xmlNewProp(node,(const xmlChar*)XML_CONTTYPE_ELEPROP_PARTNAME,
						(const xmlChar*)XML_CONTTYPE_ELEPROP_PARTNAME_VALUE_CUSTPROPS);
		xmlNewProp(node,(const xmlChar*)XML_CONTTYPE_ELEPROP_CONTENTTYPE,
						(const xmlChar*)XML_CONTTYPE_ELEPROP_CONTENTTYPE_VALUE_CUSTPROPS);
		gsf_xmlDocFormatDump(out,doc,XML_ENCODING_DEFAULT,TRUE);
	}
	else
	{
		gsf_input_seek(in,0,G_SEEK_SET);
		gsf_input_copy(in,out);
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return TRUE;
}

BOOL OOXMLFileTagging::ModifyXMLCustomProperty(GsfInput*in,GsfOutput*out,ResourceAttributes*pAttr,bool bRemove)
{
	bool bElementExist=false;
	xmlDocPtr doc;
	gsf_off_t len=gsf_input_size(in);
	const char* content=(const char*)gsf_input_read(in,len,NULL);
	if(len==-1||len==0||content==NULL)
		return FALSE;
	doc=xmlReadMemory(content,len,XML_ENCODING_DEFAULT,NULL,0);
	if(doc==NULL)
		return FALSE;
	
	xmlNode*root=xmlDocGetRootElement(doc);
	xmlNode*child=NULL;
	xmlNsPtr ns=NULL;
	if(root)
		child=root->children;
	while(child)
	{
		if(child->type==XML_ELEMENT_NODE&&
		   xmlStrcmp(child->name,(xmlChar*)XML_CUSTPROPS_ELE_PROPERTIE)==0)
		   break;
		else
			child=child->next;
	}
	int iMaxPid=0;
	while(child)
	{
		if(child->type!=XML_ELEMENT_NODE||
		   xmlStrcmp(child->name,(xmlChar*)XML_CUSTPROPS_ELE_PROPERTIE))
			break;
		const char* name=(const char*)xmlGetProp(child,(const xmlChar*)XML_CUSTPROPS_ELEPROP_NAME);
		const char* fmtid=(const char*)xmlGetProp(child,(const xmlChar*)XML_CUSTPROPS_ELEPROP_FMTID);
		const char* pid=(const char*)xmlGetProp(child,(const xmlChar*)XML_CUSTPROPS_ELEPROP_PID);
		if(!name||!fmtid||!pid)
		{
			child=child->next;
			continue;
		}
		int ipid=atoi(pid);
		if(ipid>iMaxPid)
			iMaxPid=ipid;
		int iAttrIdx=-1;
		if(bRemove==true)
		{
			if(FindAttribute(pAttr,name,iAttrIdx)&&iAttrIdx!=-1)
			{
				//xmlNode*prop=child;
				xmlUnlinkNode(child);
				xmlFreeNode(child);
			}
		}
		else
		{
			if(ns==NULL&&child->children)
			{
				ns=child->children->ns;
			}
			if(FindAttribute(pAttr,name,iAttrIdx)&&iAttrIdx!=-1)
			{
				xmlNode*propVType=child->children;
				if(propVType)
				{
					xmlUnlinkNode(propVType);
					xmlNewChild(child,ns,(const xmlChar*)"lpwstr",(const xmlChar*)GetAttributeValue(pAttr,iAttrIdx));
					xmlFreeNode(propVType);
					SetAttributeValue(pAttr,iAttrIdx,OOXML_MAGIC_PROPVALUE);
				}
			}
		}
		child=child->next;
	}//while
	if(bRemove==false) 
	{
		for(int i=0;i<GetAttributeCount(pAttr);i++)
		{
			if(strcmp(GetAttributeValue(pAttr,i),OOXML_MAGIC_PROPVALUE))
			{
				child= xmlNewChild(root,NULL,(const xmlChar*)XML_CUSTPROPS_ELE_PROPERTIE,NULL);
				if(child)
				{
					char chPid[16]="";
					xmlSetProp(child,(const xmlChar*)XML_CUSTPROPS_ELEPROP_FMTID, (const xmlChar*)FMTID_CUSTPROPS);	
					_snprintf_s(chPid,16, _TRUNCATE, "%d",++iMaxPid);
					xmlSetProp(child,(const xmlChar*)XML_CUSTPROPS_ELEPROP_PID, (const xmlChar*)chPid);	
					xmlSetProp(child,(const xmlChar*)XML_CUSTPROPS_ELEPROP_NAME, (const xmlChar*)GetAttributeName(pAttr,i));
					char chEleVT[32]="";
					if(ns==NULL)
						strncpy_s(chEleVT,32,"vt:lpwstr",_TRUNCATE);
					else
						strncpy_s(chEleVT,32,"lpwstr",_TRUNCATE);
					xmlNewChild(child,ns,(const xmlChar*)chEleVT,(const xmlChar*)GetAttributeValue(pAttr,i));
				}
			}	
		}//for 
	}
	
	gsf_xmlDocFormatDump(out,doc,XML_ENCODING_DEFAULT,FALSE);
	//xmlSaveFile("-",doc);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	gsf_output_close(GSF_OUTPUT(out));
	g_object_unref (G_OBJECT (out)); 
	return TRUE;
}


BOOL OOXMLFileTagging::ModifyXMLRels(GsfInput*in,GsfOutput*out)
{
	bool 		bElementExist=false;
	int			iMaxId=0;
	xmlDocPtr 	doc;
	gsf_off_t 	len=gsf_input_size(in);
	const char* content=(const char*)gsf_input_read(in,len,NULL);
	if(len==-1||len==0||content==NULL)
		return FALSE;
	doc=xmlReadMemory(content,len,XML_ENCODING_DEFAULT,NULL,0);
	if(doc==NULL)
		return FALSE;
	{
		xmlNode*root=xmlDocGetRootElement(doc);
		xmlNode*child=NULL;
		if(root)
			child=root->children;
		while(child)
		{
			const char* pValue=(const char*)xmlGetProp(child,(const xmlChar*)XML_RELS_ELEPROP_TYPE);
			if(pValue&&strcasecmp(pValue,XML_RELS_ELEPROP_TYPE_VALUE_CUSTPROPS)==0)
			{
				bElementExist=true;
				break;
			}
			pValue=(const char*)xmlGetProp(child,(const xmlChar*)XML_RELS_ELEPROP_ID);
			if(pValue)
			{
				int iTmp=atoi(pValue+3);
				if(iTmp>iMaxId)
					iMaxId=iTmp;
			}
			child=child->next;
		}	
	}
	if(bElementExist==false)
	{
		xmlNodePtr root=xmlDocGetRootElement(doc);
		//printf("Root element name:%s\n",root->name);
		xmlNodePtr node=xmlNewChild(root,NULL,(const xmlChar*)XML_RELS_ELE_RELATIONSHIP,NULL);
		char chId[32]="";
		_snprintf_s(chId,32, _TRUNCATE, "rId%d",iMaxId+1);
		xmlNewProp(node,(const xmlChar*)XML_RELS_ELEPROP_ID,
						(const xmlChar*)chId);
		xmlNewProp(node,(const xmlChar*)XML_RELS_ELEPROP_TYPE,
						(const xmlChar*)XML_RELS_ELEPROP_TYPE_VALUE_CUSTPROPS);
		xmlNewProp(node,(const xmlChar*)XML_RELS_ELEPROP_TARGET,
						(const xmlChar*)XML_RELS_ELEPROP_TARGET_VALUE_CUSTPROPS);
						
		gsf_xmlDocFormatDump(out,doc,XML_ENCODING_DEFAULT,TRUE);
	}
	else
	{
		gsf_input_seek(in,0,G_SEEK_SET);
		gsf_input_copy(in,out);
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return TRUE;
}
BOOL OOXMLFileTagging::ModifyXML(GsfInput*in,GsfOutput*out,OOXMLTYPE type)
{
	BOOL bRet=TRUE;
	switch(type)
	{
	case OOXML_XMLTYPE_CONTENTTYPES:
		bRet=ModifyXMLContentTypes(in,out);
		break;
	case OOXML_XMLTYPE_CUSTPROPS:
		break;
	case OOXML_XMLTYPE_RELS:
		bRet=ModifyXMLRels(in,out); 
		break;
	default:
		break;
	}
	return bRet;	
}
BOOL OOXMLFileTagging::Clone(GsfInfile *in, GsfOutfile *out,PropWriteState*pWriteState)
{
	GsfInput *input = GSF_INPUT (in);
	GsfOutput *output = GSF_OUTPUT (out);
	BOOL bRet=TRUE;
	if (gsf_input_size (input) > 0) 
	{
		//size_t 	len;
		gsf_input_seek(input,0,G_SEEK_SET);
		gsf_output_seek(output,0,G_SEEK_SET);
		bRet=gsf_input_copy(input,output);
		//while ((len = gsf_input_remaining (input)) > 0) {
		//	guint8 const *data;
		//	/* copy in odd sized chunks to exercise system */
		//	if (len > 314)
		//		len = 314;
		//	if (NULL == (data = gsf_input_read (input, len, NULL))) {
		//		g_warning ("error reading ?");
		//		break;
		//	}
		//	if (!gsf_output_write (output, len, data)) {
		//		g_warning ("error writing ?");
		//		break;
		//	}
		//}
	}
	else
	{
		int i, n = gsf_infile_num_children (in);
		const char*currInputName=gsf_input_name(GSF_INPUT(in));
		if(pWriteState->bCustPropsRelExist==false&&
			currInputName&&
			strcasecmp(currInputName,XML_CUSTPROPS_DIRNAME)==0
			)
		{
			output = gsf_outfile_new_child_full  (out, XML_CUSTPROPS_FILENAME, false, GSFOBJECT_PROPERTY_COMPRESSLEVEL, 8, NULL);
			if(CreateCustomPropsXML(output,pWriteState->pAttr)==FALSE)
				return FALSE;
			
			//gsf_output_close (GSF_OUTPUT (output));
		}
		for (i = 0 ; i < n; i++)
		{
			int 		compressLevel;
			gboolean 	is_dir;
			char const *childName = gsf_infile_name_by_index (in, i);
			char *		childDisplayName = childName ? g_filename_display_name (childName) : NULL;

			input = gsf_infile_child_by_index (in, i);
			if (NULL == input)
			{
				g_print ("Error opening '%s, index = %d\n", childDisplayName ? childDisplayName : "?", i);
				g_free (childDisplayName);
				continue;
			}

			is_dir = gsf_infile_num_children (GSF_INFILE (input)) >= 0;

			g_object_get (G_OBJECT (input), GSFOBJECT_PROPERTY_COMPRESSLEVEL, &compressLevel, NULL);
			
			//g_print ("%s(%s):depth=%d, size=%ld, level=%d, %s\n",
			//	 		childDisplayName ? childDisplayName : "?",
			//	 		currInputName ? currInputName : "?",
			//	 		pWriteState->iCurrDirDepth,
			//	 		(long)gsf_input_size (input),
			//	 		compressLevel,
			//	 		is_dir ? "directory" : "file");

			output = gsf_outfile_new_child_full  (out, childName, is_dir, GSFOBJECT_PROPERTY_COMPRESSLEVEL, compressLevel, NULL);
			if(pWriteState->bCustPropsRelExist==false)
			{
				if(pWriteState->iCurrDirDepth==1&&
					strcasecmp(childDisplayName,XML_CONTTYPE_FILENAME)==0)
				{
					g_free (childDisplayName);
					if((bRet=ModifyXML(input,output,OOXML_XMLTYPE_CONTENTTYPES))==FALSE)
						break;
					gsf_output_close (GSF_OUTPUT (output));
					g_object_unref (G_OBJECT (output));
					g_object_unref (G_OBJECT (input));
					
				}
				else if(pWriteState->iCurrDirDepth==2&&
					    strcasecmp(childDisplayName,XML_RELS_FILENAME)==0&&
					    strcasecmp(currInputName,XML_RELS_DIRNAME)==0)
				{
					g_free (childDisplayName);
					if((bRet=ModifyXML(input,output,OOXML_XMLTYPE_RELS))==FALSE)
						break;
					gsf_output_close (GSF_OUTPUT (output));
					g_object_unref (G_OBJECT (output));
					g_object_unref (G_OBJECT (input));
				}
				else
				{
					g_free (childDisplayName);
					pWriteState->iCurrDirDepth++;
					bRet=Clone (GSF_INFILE (input), GSF_OUTFILE (output),pWriteState);
					pWriteState->iCurrDirDepth--;
					if(bRet==FALSE)
						break;
				}	
			}
			else
			{
				if(pWriteState->iCurrDirDepth==2&&
					    strcasecmp(childDisplayName,XML_CUSTPROPS_FILENAME)==0&&
					    strcasecmp(currInputName,XML_CUSTPROPS_DIRNAME)==0)
				{
					g_free (childDisplayName);
					if((bRet=ModifyXMLCustomProperty(input,output,pWriteState->pAttr,pWriteState->bRemove))==FALSE)
						break;
				}
				else
				{
					g_free (childDisplayName);
					pWriteState->iCurrDirDepth++;
					bRet=Clone (GSF_INFILE (input), GSF_OUTFILE (output),pWriteState);
					pWriteState->iCurrDirDepth--;
					if(bRet==FALSE)
						break;
				} 
			}
		}
	}
	gsf_output_close (GSF_OUTPUT (out));
	g_object_unref (G_OBJECT (out));
	g_object_unref (G_OBJECT (in));
	return bRet;
}
//BOOL OOXMLFileTagging::Clone(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *attrs)
//{
//	GsfInfile  *infile;
//	GsfOutput	*output;
//	GsfOutfile *outfile;
//	GError    *err = NULL;
//
//	infile=FileTagging::GetGsfInfile(pchFileName,NULL);
//	if (infile == NULL) 
//		return FALSE;
//	
//	std::string strNewFileName;
//	if(NSLinuxTagLib::Utils::GetTempFile(pchFileName,strNewFileName)==FALSE)
//	{
//		g_object_unref (G_OBJECT (infile));
//		return FALSE;
//	}
//	output= gsf_output_stdio_new (strNewFileName.c_str(), NULL);
//	
//	if (output == NULL)
//		return FALSE;
//	
//	outfile =  gsf_outfile_zip_new (output, NULL);
//	g_object_unref (G_OBJECT (output));
//	
//	if (outfile == NULL) 
//		return FALSE;
//
//	//GsfOutfile* root=gsf_outfile_open_pkg_new (outfile);
//	
//	gsf_open_pkg_foreach_rel(GSF_INPUT(infile),(GsfOpenPkgIter)ForeachOOXMLRel,outfile);
//	
//	//GsfOutfile *xl_dir    = (GsfOutfile *)gsf_outfile_new_child (root, "xl", TRUE);
//	//GsfOutfile *wb_part   = (GsfOutfile *)gsf_outfile_open_pkg_add_rel (xl_dir, "workbook.xml",
// 	// 		"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml",
// 	// 		root,
// 	// 		"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");
//	//char* pch=NULL;
//	//g_object_get (G_OBJECT (xl_dir), "content-type", &pch, NULL);
//	//g_object_get (G_OBJECT (wb_part), "content-type", &pch, NULL);
// 	//gsf_output_close (GSF_OUTPUT(wb_part));
// 	//gsf_output_close (GSF_OUTPUT(xl_dir)); 
//	//gsf_output_close (GSF_OUTPUT(root));
//	//
//	//gsf_output_close (GSF_OUTPUT(output));
//	//
//	//g_object_unref (G_OBJECT (wb_part));
//	g_object_unref (G_OBJECT (outfile));
//	g_object_unref (G_OBJECT (infile));
//	return TRUE;
//}	

}

