


#include <Windows.h>
#include <string>
#include <algorithm>

#include <nudf\exception.hpp>
#include <nudf\xmlparser.hpp>
//#include <nudf\regex.hpp>
#include <nudf\string.hpp>


using namespace ATL;
using namespace nudf::util;


CXmlDocument::CXmlDocument()
{
    CoInitialize(NULL);
}

CXmlDocument::~CXmlDocument()
{
    Close();
    CoUninitialize();
}

void CXmlDocument::Create()
{
    HRESULT      hr = S_OK;

    Close();

    hr = _doc.CoCreateInstance(L"MSXML2.DOMDocument.6.0");
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    _doc->put_async(VARIANT_FALSE);
}

void CXmlDocument::LoadFromFile(_In_ LPCWSTR file)
{
    HRESULT      hr = S_OK;
    CComVariant  varPath(file);
    CComBSTR     bstrName;
    VARIANT_BOOL varResult = VARIANT_FALSE;

    hr = _doc.CoCreateInstance(L"MSXML2.DOMDocument.6.0");
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    _doc->put_async(VARIANT_FALSE);

    hr = _doc->load(varPath, &varResult);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    if(VARIANT_FALSE == varResult) {
        CComPtr<IXMLDOMParseError> spError;
        LONG errCode = 0;
        hr = _doc->get_parseError(&spError);
        if(SUCCEEDED(hr)) {
            spError->get_errorCode(&errCode);
        }
        _doc.Release();
        throw WIN32ERROR2(errCode);
    }
}

void CXmlDocument::LoadFromXml(_In_ LPCWSTR xml)
{
    HRESULT      hr = S_OK;
    CComBSTR     varXML(xml);
    CComBSTR     bstrName;
    VARIANT_BOOL varResult = VARIANT_FALSE;
    
    hr = _doc.CoCreateInstance(L"MSXML2.DOMDocument.6.0");
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    _doc->put_async(VARIANT_FALSE);  

    hr = _doc->loadXML(varXML, &varResult);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    if(VARIANT_FALSE == varResult) {
        CComPtr<IXMLDOMParseError> spError;
        LONG errCode = 0;
        hr = _doc->get_parseError(&spError);
        if(SUCCEEDED(hr)) {
            spError->get_errorCode(&errCode);
        }
        _doc.Release();
        throw WIN32ERROR2(errCode);
    }
}

void CXmlDocument::SaveToFile(_In_ LPCWSTR file)
{
    HRESULT     hr = S_OK;
    CComVariant varPath(file);

    if(_doc == NULL) {
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
    if(NULL == file || L'\0'==file[0]) {
        throw WIN32ERROR2(ERROR_INVALID_PARAMETER);
    }
    
    hr = _doc->save(varPath);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
}

std::wstring CXmlDocument::GetXml() throw()
{
    HRESULT      hr = S_OK;
    CComBSTR     varXml;
    std::wstring xml;
    CComPtr<IXMLDOMElement> spRoot;

    if(_doc == NULL) {
        return L"";
    }
    if(!GetDocRoot(&spRoot)) {
        return L"";
    }

    hr = spRoot->get_xml(&varXml);
    if(SUCCEEDED(hr) && NULL!=varXml.m_str) {
        xml = varXml.m_str;
    }

    return xml;
}

VOID CXmlDocument::Close() throw()
{
    _doc.Release();
}

IXMLDOMDocument* CXmlDocument::GetDoc() throw()
{
    return _doc.p;
}

bool CXmlDocument::GetDocRoot(IXMLDOMElement** root) throw()
{
    if(_doc == NULL) {
        return false;
    }

    HRESULT hr = _doc->get_documentElement(root);
    if(FAILED(hr)) {
        *root = NULL;
        return false;
    }

    return true;
}

DOMNodeType CXmlDocument::GetNodeType(_In_ IXMLDOMNode* node) throw()
{
    return XmlUtil::GetNodeType(node);
}

std::wstring CXmlDocument::GetNodeName(_In_ IXMLDOMNode* node) throw()
{
    return XmlUtil::GetNodeName(node);
}

std::wstring CXmlDocument::GetNodeValue(_In_ IXMLDOMNode* node) throw()
{
    return XmlUtil::GetNodeValue(node);
}

VOID CXmlDocument::SetNodeValue(_In_ IXMLDOMNode* node, _In_ const std::wstring& value)
{
    return XmlUtil::SetNodeValue(node, value);
}

std::wstring CXmlDocument::GetNodeXml(_In_ IXMLDOMNode* node) throw()
{
    return XmlUtil::GetNodeXml(node);
}

std::wstring CXmlDocument::GetNodeText(_In_ IXMLDOMNode* node) throw()
{
    return XmlUtil::GetNodeText(node);
}

VOID CXmlDocument::SetNodeText(_In_ IXMLDOMNode* node, _In_ const std::wstring& text)
{
    return XmlUtil::SetNodeText(node, text);
}


bool CXmlDocument::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ std::wstring& value) throw()
{
    return XmlUtil::GetNodeAttribute(node, name, value);
}

bool CXmlDocument::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ int* value) throw()
{
    return XmlUtil::GetNodeAttribute(node, name, value);
}

bool CXmlDocument::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ __int64* value) throw()
{
    return XmlUtil::GetNodeAttribute(node, name, value);
}

bool CXmlDocument::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ unsigned int* value) throw()
{
    return XmlUtil::GetNodeAttribute(node, name, value);
}

bool CXmlDocument::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ unsigned __int64* value) throw()
{
    return XmlUtil::GetNodeAttribute(node, name, value);
}

bool CXmlDocument::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ float* value) throw()
{
    return XmlUtil::GetNodeAttribute(node, name, value);
}

bool CXmlDocument::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ SYSTEMTIME* value, _Out_opt_ bool* utc) throw()
{
    return XmlUtil::GetNodeAttribute(node, name, value, utc);
}

bool CXmlDocument::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ FILETIME* value, _Out_opt_ bool* utc) throw()
{
    return XmlUtil::GetNodeAttribute(node, name, value, utc);
}

void CXmlDocument::SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const std::wstring& value)
{
    XmlUtil::SetNodeAttribute(_doc, node, name, value);
}

void CXmlDocument::SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ int value)
{
    XmlUtil::SetNodeAttribute(_doc, node, name, value);
}

void CXmlDocument::SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ __int64 value)
{
    XmlUtil::SetNodeAttribute(_doc, node, name, value);
}

void CXmlDocument::SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ unsigned int value)
{
    XmlUtil::SetNodeAttribute(_doc, node, name, value);
}

void CXmlDocument::SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ unsigned __int64 value)
{
    XmlUtil::SetNodeAttribute(_doc, node, name, value);
}

void CXmlDocument::SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ float value)
{
    XmlUtil::SetNodeAttribute(_doc, node, name, value);
}

void CXmlDocument::SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const SYSTEMTIME* value, _In_ bool utc)
{
    XmlUtil::SetNodeAttribute(_doc, node, name, value, utc);
}

void CXmlDocument::SetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const FILETIME& value, _In_ bool utc)
{
    XmlUtil::SetNodeAttribute(_doc, node, name, value, utc);
}

bool CXmlDocument::FindChildElement(_In_ IXMLDOMNode* node, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem)
{
    return XmlUtil::FindChildElement(node, name, ppelem);
}

void CXmlDocument::CreateElement(_In_ LPCWSTR name, _Out_ IXMLDOMElement** ppelem)
{
    XmlUtil::CreateElement(_doc, name, ppelem);
}

void CXmlDocument::AppendChildElement(_In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem)
{
    XmlUtil::AppendChildElement(_doc, name, ppelem);
}

void CXmlDocument::AppendChildElement(_In_ IXMLDOMNode* node, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem)
{
    XmlUtil::AppendChildElement(_doc, node, name, ppelem);
}

void CXmlDocument::CreateComment(_In_ LPCWSTR comment, _Out_opt_ IXMLDOMComment** ppcomment)
{
    XmlUtil::CreateComment(_doc, comment, ppcomment);
}

void CXmlDocument::AppendChildComment(_In_ LPCWSTR comment, _Out_opt_ IXMLDOMNode** ppcomment)
{
    XmlUtil::AppendChildComment(_doc, comment, ppcomment);
}

void CXmlDocument::AppendChildComment(_In_ IXMLDOMNode* node, _In_ LPCWSTR comment, _Out_opt_ IXMLDOMNode** ppcomment)
{
    XmlUtil::AppendChildComment(_doc, node, comment, ppcomment);
}



//
//  XmlUtil
//

DOMNodeType XmlUtil::GetNodeType(_In_ IXMLDOMNode* pNode) throw()
{
    //HRESULT hr = S_OK;
    DOMNodeType nodeType;
    (VOID)pNode->get_nodeType(&nodeType);
    return nodeType;
}

std::wstring XmlUtil::GetNodeName(_In_ IXMLDOMNode* node) throw()
{
    HRESULT hr = S_OK;
    CComBSTR bstrName;
    std::wstring name;
    hr = node->get_nodeName(&bstrName);
    if(SUCCEEDED(hr)) {
        name = (NULL == bstrName.m_str) ? L"" : bstrName.m_str;
    }
    return name;
}

std::wstring XmlUtil::GetNodeValue(_In_ IXMLDOMNode* node) throw()
{
    HRESULT hr = S_OK;
    CComVariant varValue;
    std::wstring value;
    hr = node->get_nodeValue(&varValue);
    if(SUCCEEDED(hr)) {
        value = (VT_BSTR != varValue.vt || NULL == varValue.bstrVal) ? L"" : varValue.bstrVal;
    }
    return value;
}

void XmlUtil::SetNodeValue(_In_ IXMLDOMNode* node, _In_ const std::wstring& value)
{
    HRESULT hr = S_OK;
    CComVariant varValue(value.c_str());
    hr = node->put_nodeValue(varValue);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
}

std::wstring XmlUtil::GetNodeXml(_In_ IXMLDOMNode* node) throw()
{
    HRESULT hr = S_OK;
    CComBSTR bstrXml;
    std::wstring xml;
    hr = node->get_xml(&bstrXml);
    if(SUCCEEDED(hr)) {
        xml = (NULL == bstrXml.m_str) ? L"" : bstrXml.m_str;
    }
    return xml;
}

std::wstring XmlUtil::GetNodeText(_In_ IXMLDOMNode* node)
{
    HRESULT hr = S_OK;
    CComBSTR bstrText;
    std::wstring text;
    hr = node->get_text(&bstrText);
    if(SUCCEEDED(hr)) {
        text = (NULL == bstrText.m_str) ? L"" : bstrText.m_str;
    }
    return text;
}

void XmlUtil::SetNodeText(_In_ IXMLDOMNode* node, _In_ const std::wstring& text)
{
    HRESULT hr = S_OK;
    CComBSTR bstrText(text.c_str());
    hr = node->put_text(bstrText);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
}

bool XmlUtil::FindChildElement(_In_ IXMLDOMNode* node, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem) throw()
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList> spNodeList;
    LONG count = 0;

    *ppelem = NULL;

    hr = node->get_childNodes(&spNodeList);
    if(FAILED(hr)) {
        return NULL;
    }

    hr = spNodeList->get_length(&count);
    if(FAILED(hr)) {
        return false;
    }

    for(long i=0; i<count; i++) {

        IXMLDOMNode* pSubNode = NULL;
        std::wstring wsSubNodeName;
        hr = spNodeList->get_item(i, &pSubNode);
        if(FAILED(hr)) {
            return false;
        }

        if(NODE_ELEMENT != XmlUtil::GetNodeType(pSubNode)) {
            pSubNode->Release();
            pSubNode = NULL;
            continue;
        }

        wsSubNodeName = XmlUtil::GetNodeName(pSubNode);
        if(0 == _wcsicmp(wsSubNodeName.c_str(), name)) {
            *ppelem = pSubNode;
            pSubNode = NULL;
            return true;
        }

        // Not match? release
        pSubNode->Release();
        pSubNode = NULL;
    }

    return false;
}

void XmlUtil::CreateElement(_In_ IXMLDOMDocument* doc, _In_ LPCWSTR name, _Out_ IXMLDOMElement** ppelem)
{
    HRESULT hr = S_OK;
    CComBSTR bstrName(name);
    hr = doc->createElement(bstrName, ppelem);
    if(FAILED(hr)) {
        *ppelem = NULL;
        throw WIN32ERROR2(hr);
    }
}

void XmlUtil::AppendChildElement(_In_ IXMLDOMDocument* doc, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMElement> spNewElement;
    XmlUtil::CreateElement(doc, name, &spNewElement);
    hr = doc->appendChild(spNewElement, ppelem);
    if(FAILED(hr)) {
        *ppelem = NULL;
        throw WIN32ERROR2(hr);
    }
}

void XmlUtil::AppendChildElement(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ LPCWSTR name, _Out_ IXMLDOMNode** ppelem)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMElement> spNewElement;
    XmlUtil::CreateElement(doc, name, &spNewElement);
    hr = node->appendChild(spNewElement, ppelem);
    if(FAILED(hr)) {
        *ppelem = NULL;
        throw WIN32ERROR2(hr);
    }
}

void XmlUtil::CreateComment(_In_ IXMLDOMDocument* doc, _In_ LPCWSTR comment, _Out_opt_ IXMLDOMComment** ppcomment)
{
    HRESULT hr = S_OK;
    CComBSTR bstrData(comment);
    IXMLDOMComment* pComment = NULL;
    hr = doc->createComment(bstrData, &pComment);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    if(NULL == ppcomment) {
        pComment->Release();
        pComment = NULL;
    }
    else {
        *ppcomment = pComment;
        pComment = NULL;
    }
}

void XmlUtil::AppendChildComment(_In_ IXMLDOMDocument* doc, _In_ LPCWSTR comment, _Out_opt_ IXMLDOMNode** ppcomment)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMComment> spNewElement;
    IXMLDOMNode* pNode = NULL;
    XmlUtil::CreateComment(doc, comment, &spNewElement);
    hr = doc->appendChild(spNewElement, &pNode);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    if(NULL == ppcomment) {
        pNode->Release();
        pNode = NULL;
    }
    else {
        *ppcomment = pNode;
        pNode = NULL;
    }
}

void XmlUtil::AppendChildComment(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ LPCWSTR comment, _Out_opt_ IXMLDOMNode** ppcomment)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMComment> spNewElement;
    IXMLDOMNode* pNode = NULL;
    XmlUtil::CreateComment(doc, comment, &spNewElement);
    hr = node->appendChild(spNewElement, &pNode);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    if(NULL == ppcomment) {
        pNode->Release();
        pNode = NULL;
    }
    else {
        *ppcomment = pNode;
        pNode = NULL;
    }
}

bool XmlUtil::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ std::wstring& value) throw()
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNamedNodeMap> spAttrMap;
    bool bRet = false;

    value = L"";

    try {
        hr = node->get_attributes(&spAttrMap);
        if(SUCCEEDED(hr)) {

            CComBSTR bstrName(name.c_str());
            CComPtr<IXMLDOMNode> spAttr;
            hr = spAttrMap->getNamedItem(bstrName, &spAttr);
            if(SUCCEEDED(hr) && NULL != spAttr.p) {
                value = GetNodeValue(spAttr);
                bRet = true;
            }
        }
    }
    catch(...) {
        bRet = false;
    }

    return bRet;
}

bool XmlUtil::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ int* value) throw()
{
    std::wstring svalue;

    *value = -1;
    if(!GetNodeAttribute(node, name, svalue)) {
        return false;
    }

    return nudf::string::ToInt<wchar_t>(svalue, value);
}

bool XmlUtil::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ __int64* value) throw()
{
    std::wstring svalue;
    *value = -1;
    if(!GetNodeAttribute(node, name, svalue)) {
        return false;
    }

    return nudf::string::ToInt64<wchar_t>(svalue, value);
}

bool XmlUtil::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ unsigned int* value) throw()
{
    std::wstring svalue;
    *value = 0xffffffff;
    if(!GetNodeAttribute(node, name, svalue)) {
        return false;
    }

    return nudf::string::ToUint<wchar_t>(svalue, value);
}

bool XmlUtil::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ unsigned __int64* value) throw()
{
    std::wstring svalue;
    *value = 0xffffffffffffffff;
    if(!GetNodeAttribute(node, name, svalue)) {
        return false;
    }

    return nudf::string::ToUint64<wchar_t>(svalue, value);
}

bool XmlUtil::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ float* value) throw()
{
    std::wstring svalue;
    *value = -1;
    if(!GetNodeAttribute(node, name, svalue)) {
        return false;
    }

    return nudf::string::ToFloat<wchar_t>(svalue, value);
}

bool XmlUtil::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ SYSTEMTIME* value, _Out_opt_ bool* utc) throw()
{
    std::wstring svalue;
    
    memset(value, 0, sizeof(SYSTEMTIME));
    if(!GetNodeAttribute(node, name, svalue)) {
        return false;
    }

    return nudf::string::ToSystemTime<wchar_t>(svalue, value, utc);
}

bool XmlUtil::GetNodeAttribute(_In_ IXMLDOMNode* node, _In_ const std::wstring& name, _Out_ FILETIME* value, _Out_opt_ bool* utc) throw()
{
    std::wstring svalue;
    
    memset(value, 0, sizeof(FILETIME));
    if(!GetNodeAttribute(node, name, svalue)) {
        return false;
    }

    return nudf::string::ToSystemTime<wchar_t>(svalue, value, utc);
}

void XmlUtil::SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const std::wstring& value)
{
    HRESULT hr = S_OK;

    if(NULL == doc || NULL==node || name.empty()) {
        throw WIN32ERROR2(ERROR_INVALID_PARAMETER);
    }
    
    try {
    
        CComPtr<IXMLDOMNamedNodeMap> spAttrMap;

        hr = node->get_attributes(&spAttrMap);
        if(SUCCEEDED(hr)) {

            CComBSTR bstrName(name.c_str());
            CComPtr<IXMLDOMNode> spAttr;
            CComVariant varValue(value.c_str());
            hr = spAttrMap->getNamedItem(bstrName, &spAttr);
            if(SUCCEEDED(hr) && NULL != spAttr.p) {
                // already exist?
                hr = spAttr->put_nodeValue(varValue);
                if(FAILED(hr)) {
                    throw WIN32ERROR2(hr);
                }
            }
            else {
                // Not exist
                CComPtr<IXMLDOMAttribute> spNewAttr;
                hr = doc->createAttribute(bstrName, &spNewAttr);
                if(SUCCEEDED(hr) && NULL != spNewAttr.p) {
                    hr = spAttrMap->setNamedItem(spNewAttr, &spAttr);
                    if(SUCCEEDED(hr) && NULL != spAttr.p) {
                        hr = spAttr->put_nodeValue(varValue);
                        if(FAILED(hr)) {
                            throw WIN32ERROR2(hr);
                        }
                    }
                }
            }
        }
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void XmlUtil::SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ int value)
{
    std::wstring wsv = nudf::string::FromInt<wchar_t>(value);
    SetNodeAttribute(doc, node, name, wsv);
}

void XmlUtil::SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ __int64 value)
{
    std::wstring wsv = nudf::string::FromInt64<wchar_t>(value);
    SetNodeAttribute(doc, node, name, wsv);
}

void XmlUtil::SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ unsigned int value)
{
    std::wstring wsv = nudf::string::FromUint<wchar_t>(value);
    SetNodeAttribute(doc, node, name, wsv);
}

void XmlUtil::SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ unsigned __int64 value)
{
    std::wstring wsv = nudf::string::FromUint64<wchar_t>(value);
    SetNodeAttribute(doc, node, name, wsv);
}

void XmlUtil::SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ float value)
{
    wchar_t wsv[64] = {0};
    swprintf_s(wsv, 64, L"%.3f", value);
    SetNodeAttribute(doc, node, name, wsv);
}

void XmlUtil::SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const SYSTEMTIME* value, _In_ bool utc)
{
    std::wstring wsv = nudf::string::FromSystemTime<wchar_t>(value, utc);
    SetNodeAttribute(doc, node, name, wsv);
}

void XmlUtil::SetNodeAttribute(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* node, _In_ const std::wstring& name, _In_ const FILETIME& value, _In_ bool utc)
{
    std::wstring wsv = nudf::string::FromSystemTime<wchar_t>(&value, utc);
    SetNodeAttribute(doc, node, name, wsv);
}