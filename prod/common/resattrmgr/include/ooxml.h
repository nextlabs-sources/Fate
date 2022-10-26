#ifndef __TAGLIB_OOXML_H__
#define __TAGLIB_OOXML_H__ 
#include "resattrlib.h"
#include "utils.h"
#include "tag.h"
#include <gsf/gsf-open-pkg-utils.h>
namespace NSLinuxTagLib
{
	class OOXMLFileTagging:public FileTagging
	{
		enum {
			OOXML_CUSTPROPS_NS,
			OOXML_PROPVTYPE_NS
		};
		enum OOXMLTYPE{
			OOXML_XMLTYPE_CUSTPROPS,
			OOXML_XMLTYPE_CONTENTTYPES,
			OOXML_XMLTYPE_RELS
		};
		static const GsfXMLInNS 	gooxml_custprops_ns[];
		static const GsfXMLInNode 	gooxml_custprops_dtd[] ;
		
		static const char XML_ENCODING_DEFAULT[];
		
		static const char RELTYPE_OFFICE_DOCUMENT[];
		static const char RELTARGET_OFFICE_DOCUMENT_XLSX[];
		static const char RELTARGET_OFFICE_DOCUMENT_XLSB[];
		static const char RELTARGET_OFFICE_DOCUMENT_DOCX[];
		static const char RELTARGET_OFFICE_DOCUMENT_PPTX[];
		
		static const char RELTYPE_CUSTOM_PROPERTIES[];
		static const char FMTID_CUSTPROPS[];
		static const char XML_CUSTPROPS_ELE_PROPERTIES[];
		static const char XML_CUSTPROPS_ELE_PROPERTIE[];
		static const char XML_CUSTPROPS_ELEPROP_FMTID[];
		static const char XML_CUSTPROPS_ELEPROP_NAME[];
		static const char XML_CUSTPROPS_ELEPROP_PID[];
		static const char XML_CUSTPROPS_FILENAME[];
		static const char XML_CUSTPROPS_DIRNAME[];
		
		static const char XML_RELS_FILENAME[];
		static const char XML_RELS_DIRNAME[];
		static const char XML_RELS_ELE_RELATIONSHIP[];
		static const char XML_RELS_ELEPROP_TYPE[];
		static const char XML_RELS_ELEPROP_TYPE_VALUE_CUSTPROPS[];
		static const char XML_RELS_ELEPROP_ID[];
		static const char XML_RELS_ELEPROP_TARGET[];
		static const char XML_RELS_ELEPROP_TARGET_VALUE_CUSTPROPS[];
		
		
		static const char XML_CONTTYPE_FILENAME[];
		static const char XML_CONTTYPE_ELE_OVERRIDE[];
		static const char XML_CONTTYPE_ELEPROP_PARTNAME[];
		static const char XML_CONTTYPE_ELEPROP_PARTNAME_VALUE_CUSTPROPS[];
		static const char XML_CONTTYPE_ELEPROP_CONTENTTYPE[];
		static const char XML_CONTTYPE_ELEPROP_CONTENTTYPE_VALUE_CUSTPROPS[];
		
		
		static const char GSFOBJECT_PROPERTY_COMPRESSLEVEL[];
		static const char OOXML_MAGIC_PROPVALUE[];
		
		struct PropReadState
		{
			ResourceAttributes* pAttr;
			char*pchCurrAttrName;
			int					iAttrIdx;
			int					iBigestPid;
			bool				bReadAll;//true read all tags, false only read tag(s) in the pAttr.
			//bool				bIgnoreAtPropValueEnd;//true means ignore at the of Porperty Value End callback
		};
		struct PropWriteState
		{
			ResourceAttributes* pAttr;
			bool				bCustPropsRelExist;
			//GsfInput*			gsfInputCustPropsRel;
			int					iCurrDirDepth;//the depth from the root in a zip file,e.g the depth for [Content_types].xml is 1 and docProps/app.xml is 2.
			bool				bRemove;//true means remove
			
		};
		static void XMLPropertyElementStart  (GsfXMLIn *xin, xmlChar const **attrs);
		static void XMLPropertyElementEnd   (GsfXMLIn *xin, GsfXMLBlob *unknown);
		static void XMLPropValueElementStart (GsfXMLIn *xin, xmlChar const **attrs);
		static void XMLPropValueElementEnd_str   (GsfXMLIn *xin, GsfXMLBlob *unknown);
		static void XMLPropValueElementEnd_bool   (GsfXMLIn *xin, GsfXMLBlob *unknown);
		
		static BOOL Clone(GsfInfile *in, GsfOutfile *out,PropWriteState*pWriteState);
		static BOOL CreateCustomPropsXML(GsfOutput*output,ResourceAttributes*pAttr);
		
		static BOOL ModifyXML(GsfInput*in,GsfOutput*out,OOXMLTYPE type);
		static BOOL ModifyXMLContentTypes(GsfInput*in,GsfOutput*out);
		static BOOL ModifyXMLRels(GsfInput*in,GsfOutput*out);
		static BOOL ModifyXMLCustomProperty(GsfInput*in,GsfOutput*out,ResourceAttributes*pAttr,bool bRemove);
		
		BOOL ModifyTags(void*pluginData,const char *pchFileName, ResourceAttributes *pAttr,bool bRemove);
	public:
		static void ForeachOOXMLRel(GsfInput *opkg,GsfOpenPkgRel const *rel,gpointer    user_data);
	private:
		GsfInfile* 	GetGsfInfile(GsfInput*input,GError**err);
		GsfOutfile* GetGsfOutfile(GsfOutput*output,GError**err);
		
	public:
		OOXMLFileTagging();
		~OOXMLFileTagging();
		static BOOL IsOOXML(const char*pchFileName);	
		BOOL 		GetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		SetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		RemoveTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
		
	};
	
	
	
}

#endif //__TAG_OLE_H__

