


#ifndef __LABEL_INFO_HPP__
#define __LABEL_INFO_HPP__


#include <string>
#include <vector>
#include <memory>

//#include <nudf\exception.hpp>
#include <nudf\xmlparser.hpp>

class CLabelValue;

class CLabel
{
public:
    CLabel();
    CLabel(const std::wstring& name, const std::wstring& display_name, int default_value, bool mandatory, bool multisel, const std::wstring& description);
    ~CLabel();

    bool Load(_In_ IXMLDOMNode* pNode, _In_opt_ CLabel* parent);
    const CLabelValue* GetValue(int id) const throw();
    const CLabelValue* GetValue(const std::wstring& value) const throw();
    const CLabelValue* GetSelectedValue() const throw();
    int SelectValue(int index) throw();
    int UnselectValue(int index) throw();

    void GetClassificationTags(_Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags);

    bool ToXmlObject(_In_ IXMLDOMDocument* doc, _Out_ IXMLDOMElement** ppNode);

    inline CLabel* GetParent() const throw() {return _parent;}
    inline int GetDefaultValueId() const throw() {return _default_value;}
    inline int GetSelectedValueId() const throw() {return _selected_value;}
    inline bool IsMandatory() const throw() {return _mandatory;}
    inline bool IsMultiSelect() const throw() {return _multisel;}
    inline const std::wstring& GetName() const throw() {return _name;}
    inline const std::wstring& GetDisplayName() const throw() {return _display;}
    inline const std::wstring& GetDescription() const throw() {return _description;}
    inline const std::vector<std::tr1::shared_ptr<CLabelValue>>& GetValues() const throw() {return _values;}
    inline std::vector<std::tr1::shared_ptr<CLabelValue>>& GetValues() throw() {return _values;}
    inline bool IsFirstPage() const throw() {return (NULL==_parent)?true:false;}
    
private:
    CLabel* _parent;
    int     _default_value;
    int     _selected_value;
    bool    _mandatory;
    bool    _multisel;
    std::wstring _name;
    std::wstring _display;
    std::wstring _description;
    std::vector<std::tr1::shared_ptr<CLabelValue>> _values;
};

class CLabelValue
{
public:
    CLabelValue();
    CLabelValue(_In_ const std::wstring& value);
    ~CLabelValue();
    
    inline const std::wstring& GetValue() const throw() {return _value;}
    inline const std::tr1::shared_ptr<CLabel>& GetSubLabel() const throw() {return _sublabel;}
    inline std::tr1::shared_ptr<CLabel>& GetSubLabel() throw() {return _sublabel;}
    inline bool HasSubLabels() const throw() {return (NULL != _sublabel.get())?true:false;}

    
    bool Load(_In_ IXMLDOMNode* pNode, _In_ CLabel* parent);
    bool ToXmlObject(_In_ IXMLDOMDocument* doc, _Out_ IXMLDOMElement** ppNode);

private:
    std::wstring _value;
    std::tr1::shared_ptr<CLabel> _sublabel;
};


class CLabelConf :  public nudf::util::CXmlDocument
{
public:
    CLabelConf();
    virtual ~CLabelConf();
    
    virtual void Create();
    virtual void LoadFromFile(_In_ LPCWSTR file);
    virtual void LoadFromXml(_In_ LPCWSTR xml);
    virtual void Close() throw();

    bool AddLabelGroup(_In_ IXMLDOMElement* pElem, _In_ const std::wstring& lang, _In_ bool is_default=false);
    bool FindLabelGroup(_In_ const std::wstring& lang, _Out_ IXMLDOMNode** ppNode);
    CLabel* LoadLabels(_In_ const std::wstring& lang);
    bool FindLang(_In_ const std::wstring& lang);

    inline const std::wstring& GetDefaultLanguage() const throw() {return _default_lang;}

protected:
    void Init(_In_ IXMLDOMElement* pRoot);

private:
    std::vector<std::wstring>   _langs;
    std::wstring                _default_lang;
};


#endif