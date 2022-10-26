
#include <Windows.h>
#include <assert.h>


#include <nudf\exception.hpp>
#include <nudf\xmlparser.hpp>

#include "labelinfo.hpp"



#define LABELCONF_ROOT_NAME         L"CLASSIFICATION"
#define LABELCONF_LANGATTR_NAME     L"default-lang"

#define LABEL_NODE_NAME             L"LABEL"
#define LABEL_ATTRNAME_NAME         L"name"
#define LABEL_ATTRNAME_DISPLAYNAME  L"display-name"
#define LABEL_ATTRNAME_MANDATORY    L"mandatory"
#define LABEL_ATTRNAME_MULTISEL     L"multi-select"
#define LABEL_ATTRNAME_DEFAULTVALUE L"default-value"
#define LABEL_ATTRNAME_DESCRIPTION  L"description"

#define LABELVALUE_NODE_NAME        L"VALUE"
#define LABELVALUE_ATTRNAME_VALUE   L"value"


//
//  class CLabel
//

CLabel::CLabel() : _parent(NULL), _default_value(0), _selected_value(0), _mandatory(true), _multisel(false)
{
}

CLabel::CLabel(const std::wstring& name, const std::wstring& display_name, int default_value, bool mandatory, bool multisel, const std::wstring& description)
    : _parent(NULL), _name(name), _display(display_name),
      _default_value(default_value), _selected_value(0), _mandatory(mandatory), _multisel(multisel),
      _description(description)
{
}

CLabel::~CLabel()
{
}

bool CLabel::Load(_In_ IXMLDOMNode* pNode, _In_opt_ CLabel* parent)
{
    std::wstring wsattr;

    _parent = parent;

    //
    //  Get Attributes
    //
    if(!nudf::util::XmlUtil::GetNodeAttribute(pNode, LABEL_ATTRNAME_NAME, _name)) {
        return false;
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(pNode, LABEL_ATTRNAME_DISPLAYNAME, _display)) {
        return false;
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(pNode, LABEL_ATTRNAME_MANDATORY, wsattr)) {
        _mandatory = true;
    }
    else {
        _mandatory = (0 == _wcsicmp(wsattr.c_str(), L"false")) ? false : true;
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(pNode, LABEL_ATTRNAME_MULTISEL, wsattr)) {
        _multisel = false;
    }
    else {
        _multisel = (0 == _wcsicmp(wsattr.c_str(), L"true")) ? true : false;
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(pNode, LABEL_ATTRNAME_DEFAULTVALUE, &_default_value)) {
        _default_value = 0;
    }
    _selected_value = _default_value;
    if(!nudf::util::XmlUtil::GetNodeAttribute(pNode, LABEL_ATTRNAME_DESCRIPTION, _description)) {
        _description = L"";
    }

    //
    //  Get Values
    //

    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMNodeList> spNodeList;
        LONG count = 0;

        hr = pNode->get_childNodes(&spNodeList);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        hr = spNodeList->get_length(&count);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        for(long i=0; i<count; i++) {

            CComPtr<IXMLDOMNode> spSubNode;
            std::wstring         wsSubNodeName;

            hr = spNodeList->get_item(i, &spSubNode);
            if(FAILED(hr)) {
                break;
            }

            if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spSubNode)) {
                continue;
            }

            wsSubNodeName = nudf::util::XmlUtil::GetNodeName(spSubNode);
            if(0 != _wcsicmp(wsSubNodeName.c_str(), LABELVALUE_NODE_NAME)) {
                continue;
            }

            std::tr1::shared_ptr<CLabelValue> spLabelValue(new CLabelValue());
            if(spLabelValue->Load(spSubNode, this)) {
                _values.push_back(spLabelValue);
            }
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }

    return _values.empty() ? false : true;
}

const CLabelValue* CLabel::GetValue(int id) const throw()
{
    if((int)GetValues().size() <= id) {
        return NULL;
    }
    return GetValues()[id].get();
}

const CLabelValue* CLabel::GetValue(const std::wstring& value) const throw()
{
    for(std::vector<std::tr1::shared_ptr<CLabelValue>>::const_iterator it=_values.begin(); it!=_values.end(); ++it) {
        if(0 == _wcsicmp(value.c_str(), (*it)->GetValue().c_str())) {
            return (*it).get();
        }
    }
    return NULL;
}

const CLabelValue* CLabel::GetSelectedValue() const throw()
{
    return GetValue(_selected_value);
}

int CLabel::SelectValue(int index) throw()
{
    if(index >= (int)_values.size()) {
        return _selected_value;
    }
    if(IsMultiSelect()) {
        _selected_value |= (0x00000001 << index);
    }
    else {
        _selected_value = index;
    }

    return _selected_value;
}

int CLabel::UnselectValue(int index) throw()
{
    if(index >= (int)_values.size()) {
        return _selected_value;
    }
    if(IsMultiSelect()) {
        _selected_value &= (~(0x00000001 << index));
    }
    return _selected_value;
}

void CLabel::GetClassificationTags(_Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags)
{
    const CLabel* label = this;
    do {

        if(label->IsMultiSelect()) {
            // This must be the last one
            for(int i=0; i<(int)label->GetValues().size(); i++) {
                if(0 != (label->GetSelectedValueId() & (0x00000001 << i))) {
                    // This one is selected
                    tags.push_back(std::pair<std::wstring,std::wstring>(label->GetName(), label->GetValues()[i]->GetValue()));
                }
            }
            label = NULL;
        }
        else {
            const CLabelValue* pLabelValue = label->GetSelectedValue();
            if(NULL == pLabelValue) {
                break;
            }
            tags.push_back(std::pair<std::wstring,std::wstring>(label->GetName(), pLabelValue->GetValue()));
            // Move to Next
            label = pLabelValue->GetSubLabel().get();
        }

    } while (NULL != label);
}

bool CLabel::ToXmlObject(_In_ IXMLDOMDocument* doc, _Out_ IXMLDOMElement** ppNode)
{
    HRESULT hr = S_OK;
    IXMLDOMElement* pElem = NULL;

    *ppNode = NULL;

    hr = doc->createElement(LABEL_NODE_NAME, &pElem);
    if(FAILED(hr)) {
        return false;
    }

    nudf::util::XmlUtil::SetNodeAttribute(doc, pElem, LABEL_ATTRNAME_DEFAULTVALUE, _default_value);
    nudf::util::XmlUtil::SetNodeAttribute(doc, pElem, LABEL_ATTRNAME_MANDATORY, _mandatory ? L"true" : L"false");
    nudf::util::XmlUtil::SetNodeAttribute(doc, pElem, LABEL_ATTRNAME_MULTISEL, _multisel ? L"true" : L"false");
    nudf::util::XmlUtil::SetNodeAttribute(doc, pElem, LABEL_ATTRNAME_NAME, _name);
    nudf::util::XmlUtil::SetNodeAttribute(doc, pElem, LABEL_ATTRNAME_DISPLAYNAME, _display);
    nudf::util::XmlUtil::SetNodeAttribute(doc, pElem, LABEL_ATTRNAME_DESCRIPTION, _description);

    for(std::vector<std::tr1::shared_ptr<CLabelValue>>::const_iterator it=_values.begin(); it!=_values.end(); ++it) {
        CComPtr<IXMLDOMElement> spValue;
        CComPtr<IXMLDOMNode>    spChildValue;

        if(!(*it)->ToXmlObject(doc, &spValue)) {
            pElem->Release();
            pElem = NULL;
            return false;
        }

        hr = pElem->appendChild(spValue, &spChildValue);
        if(FAILED(hr)) {
            pElem->Release();
            pElem = NULL;
            return false;
        }
    }

    *ppNode = pElem;
    return true;
}

//
//  class CLabelValue
//

CLabelValue::CLabelValue()
{
}

CLabelValue::CLabelValue(_In_ const std::wstring& value) : _value(value)
{
}

CLabelValue::~CLabelValue()
{
}

bool CLabelValue::Load(_In_ IXMLDOMNode* pNode, _In_ CLabel* parent)
{
    //
    //  Get Attributes
    //
    if(!nudf::util::XmlUtil::GetNodeAttribute(pNode, LABELVALUE_ATTRNAME_VALUE, _value)) {
        return false;
    }

    //
    //  Get Sub-labels
    //
    try {
        
        std::tr1::shared_ptr<CLabel> spLabel(new CLabel());
        CComPtr<IXMLDOMNode> spLabelNode;
        
        if(nudf::util::XmlUtil::FindChildElement(pNode, LABEL_NODE_NAME, &spLabelNode)) {
            if(spLabel->Load(spLabelNode, parent)) {
                _sublabel = spLabel;
            }
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }

    return true;
}

bool CLabelValue::ToXmlObject(_In_ IXMLDOMDocument* doc, _Out_ IXMLDOMElement** ppNode)
{
    HRESULT hr = S_OK;
    IXMLDOMElement* pElem = NULL;

    *ppNode = NULL;

    hr = doc->createElement(LABELVALUE_NODE_NAME, &pElem);
    if(FAILED(hr)) {
        return false;
    }

    nudf::util::XmlUtil::SetNodeAttribute(doc, pElem, LABELVALUE_ATTRNAME_VALUE, _value);

    if(_sublabel.get() != NULL) {

        CComPtr<IXMLDOMElement> spLabel;
        CComPtr<IXMLDOMNode>    spChildLabel;

        if(!_sublabel->ToXmlObject(doc, &spLabel)) {
            pElem->Release();
            pElem = NULL;
            return false;
        }

        hr = pElem->appendChild(spLabel, &spChildLabel);
        if(FAILED(hr)) {
            pElem->Release();
            pElem = NULL;
            return false;
        }
    }

    *ppNode = pElem;
    return true;
}


//
//  class CLabelConf
//

CLabelConf::CLabelConf()
{
}

CLabelConf::~CLabelConf()
{
}
  
void CLabelConf::Create()
{
    try {
    
        // Create Doc
        nudf::util::CXmlDocument::Create();
        // Creater Root
        CComPtr<IXMLDOMNode> spRoot;
        AppendChildElement(LABELCONF_ROOT_NAME, &spRoot);
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CLabelConf::LoadFromFile(_In_ LPCWSTR file)
{
    //HRESULT hr = S_OK;
    CComPtr<IXMLDOMElement> spRoot;
    std::wstring root_name;

    nudf::util::CXmlDocument::LoadFromFile(file);

    if(!GetDocRoot(&spRoot)) {
        Close();
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(spRoot);
    if(0 != _wcsicmp(LABELCONF_ROOT_NAME, root_name.c_str())) {
        Close();
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    try {
        Init(spRoot);
    }
    catch(const nudf::CException& e) {
        Close();
        throw e;
    }
}

void CLabelConf::LoadFromXml(_In_ LPCWSTR xml)
{
    //HRESULT hr = S_OK;
    CComPtr<IXMLDOMElement> spRoot;
    std::wstring root_name;

    nudf::util::CXmlDocument::LoadFromXml(xml);

    if(!GetDocRoot(&spRoot)) {
        Close();
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(spRoot);
    if(0 != _wcsicmp(LABELCONF_ROOT_NAME, root_name.c_str())) {
        Close();
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    try {
        Init(spRoot);
    }
    catch(const nudf::CException& e) {
        Close();
        throw e;
    }
}

void CLabelConf::Init(_In_ IXMLDOMElement* pRoot)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList> spNodeList;
    LONG count = 0;
    bool default_lang_exist = false;

    if(!nudf::util::XmlUtil::GetNodeAttribute(pRoot, LABELCONF_LANGATTR_NAME, _default_lang)) {
        _default_lang = L"en-US";
    }

    hr = pRoot->get_childNodes(&spNodeList);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = spNodeList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    for(long i=0; i<count; i++) {

        CComPtr<IXMLDOMNode> spSubNode;
        std::wstring         wsSubNodeName;

        hr = spNodeList->get_item(i, &spSubNode);
        if(FAILED(hr)) {
            break;
        }

        if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spSubNode)) {
            continue;
        }

        wsSubNodeName = nudf::util::XmlUtil::GetNodeName(spSubNode);
        _langs.push_back(wsSubNodeName);
        if(!default_lang_exist && 0 == _wcsicmp(wsSubNodeName.c_str(), _default_lang.c_str())) {
            default_lang_exist = true;
        }
    }

    if(!default_lang_exist) {
        _default_lang = L"";
        if(_langs.size() != 0) {
            _default_lang = _langs[0];
        }
    }
}

void CLabelConf::Close() throw()
{
    _langs.clear();
}

bool CLabelConf::AddLabelGroup(_In_ IXMLDOMElement* pElem, _In_ const std::wstring& lang, _In_ bool is_default)
{
    bool bRet = false;
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMElement> spRoot;
    CComPtr<IXMLDOMNode> spLabels;
    CComPtr<IXMLDOMNode> spOldLabels;

    GetDocRoot(&spRoot);
    if(NULL == spRoot) {
        return false;
    }

    try {

        if(FindLabelGroup(lang, &spLabels)) {

            hr = spLabels->replaceChild(pElem, spLabels, &spOldLabels);
            if(SUCCEEDED(hr)) {
                bRet = true;
                if(is_default) {
                    SetNodeAttribute(spRoot, LABELCONF_LANGATTR_NAME, lang);
                }
            }
        }
        else {
            CComPtr<IXMLDOMNode> spLangNode;
            CComPtr<IXMLDOMNode> spNewLabels;

            nudf::util::XmlUtil::AppendChildElement(GetDoc(), spRoot, lang.c_str(), &spLangNode);
            hr = spLangNode->appendChild(pElem, &spNewLabels);
            if(SUCCEEDED(hr)) {
                bRet = true;
                _langs.push_back(lang);
                if(is_default) {
                    SetNodeAttribute(spRoot, LABELCONF_LANGATTR_NAME, lang);
                }
            }
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        bRet = false;
    }

    return bRet;
}

bool CLabelConf::FindLang(_In_ const std::wstring& lang)
{
    for(std::vector<std::wstring>::const_iterator it=_langs.begin(); it!=_langs.end(); ++it) {
        if(0 == _wcsicmp((*it).c_str(), lang.c_str())) {
            return true;
        }
    }
    return false;
}

bool CLabelConf::FindLabelGroup(_In_ const std::wstring& lang, _Out_ IXMLDOMNode** ppNode)
{
    //HRESULT hr = S_OK;
    CComPtr<IXMLDOMElement> spRoot;
    CComPtr<IXMLDOMNode> spLangNode;
    std::wstring target_lang(lang);

    *ppNode = NULL;
    
    if(!GetDocRoot(&spRoot)) {
        return false;
    }

    if(_langs.empty()) {
        return false;
    }

    if(target_lang.empty()) {
        target_lang = _default_lang;
    }

    if(!nudf::util::XmlUtil::FindChildElement(spRoot, target_lang.c_str(), &spLangNode)) {
        return false;
    }

    if(!nudf::util::XmlUtil::FindChildElement(spLangNode, LABEL_NODE_NAME, ppNode)) {
        *ppNode = NULL;
        return false;
    }

    return true;
}

CLabel* CLabelConf::LoadLabels(_In_ const std::wstring& lang)
{
    //HRESULT hr = S_OK;
    CComPtr<IXMLDOMNode> spLabels;
    CLabel* labels = NULL;
    
    if(!FindLabelGroup(lang, &spLabels)) {
        return NULL;
    }

    labels = new CLabel();
    if(!labels->Load(spLabels, NULL)) {
        delete labels;
        labels = NULL;
    }

    return labels;
}
