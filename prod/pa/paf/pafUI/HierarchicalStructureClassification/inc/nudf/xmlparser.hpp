
#ifndef __NUDF_XMLPARSER_HPP__
#define __NUDF_XMLPARSER_HPP__


#include <atlbase.h>
#include <atlcomcli.h>
#include <string>
#include <vector>
#include <map>


namespace nudf
{
namespace util
{

class CXmlDocument
{
public:
    CXmlDocument();
    virtual ~CXmlDocument();

    virtual void Create();
    virtual void LoadFromFile(_In_ LPCWSTR file);
    virtual void LoadFromXml(_In_ LPCWSTR xml);
    virtual void SaveToFile(_In_ LPCWSTR file);
    virtual void Close() throw();
        

public:
    IXMLDOMDocument* GetDoc() throw();
    std::wstring GetXml() throw();
    bool GetDocRoot(IXMLDOMElement** root) throw();

public:
    DOMNodeType GetNodeType(_In_ IXMLDOMNode* node) throw();
    std::wstring GetNodeName(_In_ IXMLDOMNode* node) throw();
    std::wstring GetNodeValue(_In_ IXMLDOMNode* node) throw();
    void SetNodeValue(_In_ IXMLDOMNode* node, _In_ const std::wstring& value);
    std::wstring GetNodeXml(_In_ IXMLDOMNode* node) throw();
    std::wstring GetNodeText(_In_ IXMLDOMNode* node) throw();
    void SetNodeText(_In_ IXMLDOMNode* node, _In_ const std::wstring& text);
    
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ std::wstring& value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ int* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ __int64* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ unsigned int* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ unsigned __int64* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ float* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ SYSTEMTIME* value, _Out_opt_ bool* utc=NULL) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ FILETIME* value, _Out_opt_ bool* utc=NULL) throw();

    void SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const std::wstring& value);
    void SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ int value);
    void SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ __int64 value);
    void SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ unsigned int value);
    void SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ unsigned __int64 value);
    void SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ float value);
    void SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const SYSTEMTIME* value, _In_ bool utc=true);
    void SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const FILETIME& value, _In_ bool utc=true);

    bool FindChildElement(_In_ IXMLDOMNode* node, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem) throw();
    void CreateElement(_In_ LPCWSTR name, _Out_ IXMLDOMElement** ppelem);
    void AppendChildElement(_In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem);
    void AppendChildElement(_In_ IXMLDOMNode* node, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem);
    void CreateComment(_In_ LPCWSTR comment, _Out_opt_ IXMLDOMComment** ppcomment);
    void AppendChildComment(_In_ LPCWSTR comment, _Out_opt_ IXMLDOMNode** ppcomment);
    void AppendChildComment(_In_ IXMLDOMNode* node, _In_ LPCWSTR comment, _Out_opt_ IXMLDOMNode** ppcomment);


private:
    CComPtr<IXMLDOMDocument>  _doc;
};

namespace XmlUtil {

    DOMNodeType GetNodeType(_In_ IXMLDOMNode* node) throw();
    std::wstring GetNodeName(_In_ IXMLDOMNode* node) throw();
    std::wstring GetNodeValue(_In_ IXMLDOMNode* node) throw();
    void SetNodeValue(_In_ IXMLDOMNode* node, _In_ const std::wstring& value);
    std::wstring GetNodeXml(_In_ IXMLDOMNode* node) throw();
    std::wstring GetNodeText(_In_ IXMLDOMNode* node) throw();
    void SetNodeText(_In_ IXMLDOMNode* node, _In_ const std::wstring& text);
    
    bool FindChildElement(_In_ IXMLDOMNode* node, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem) throw();
    void CreateElement(_In_ IXMLDOMDocument* doc, _In_ LPCWSTR name, _Out_ IXMLDOMElement** ppelem);
    void AppendChildElement(_In_ IXMLDOMDocument* doc, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem);
    void AppendChildElement(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem);
    void CreateComment(_In_ IXMLDOMDocument* doc, _In_ LPCWSTR comment, _Out_opt_ IXMLDOMComment** ppcomment);
    void AppendChildComment(_In_ IXMLDOMDocument* doc, _In_ LPCWSTR comment, _Out_opt_ IXMLDOMNode** ppcomment);
    void AppendChildComment(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ LPCWSTR comment, _Out_opt_ IXMLDOMNode** ppcomment);


    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ std::wstring& value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ int* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ __int64* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ unsigned int* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ unsigned __int64* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ float* value) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ SYSTEMTIME* value, _Out_opt_ bool* utc=NULL) throw();
    bool GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ FILETIME* value, _Out_opt_ bool* utc=NULL) throw();

    void SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const std::wstring& value);
    void SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ int value);
    void SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ __int64 value);
    void SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ unsigned int value);
    void SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ unsigned __int64 value);
    void SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ float value);
    void SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const SYSTEMTIME* value, _In_ bool utc=true);
    void SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const FILETIME& value, _In_ bool utc=true);


}   // namespace nudf::util::XmlUtil



}   // namespace nudf::util
}   // namespace nudf




#endif  // #ifndef __NUDF_XMLPARSER_HPP__