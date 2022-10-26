

#ifndef __CLASSIFY_DATA_HPP__
#define __CLASSIFY_DATA_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <msxml6.h>


//#include <nudf\exception.hpp>
//#include <nudf\xmlparser.hpp>


namespace classify {

class CItemValue
{
public:
    CItemValue();
    CItemValue(int priority, const std::wstring& data, const std::wstring& description, const std::vector<std::wstring>& subitems);
    virtual ~CItemValue();

    inline int GetPriority() const throw() {return _priority;}
    inline const std::wstring& GetData() const throw() {return _data;}
    inline const std::wstring& GetDescription() const throw() {return _description;}
    inline const std::vector<std::wstring>& GetSubItems() const throw() {return _subitems;}
	inline void SetData(const wchar_t* szData) { if (szData){ _data = szData; } }
    inline CItemValue& operator = (const CItemValue& value) throw()
    {
        if(this != &value) {
            _priority = value.GetPriority();
            _data = value.GetData();
            _description = value.GetDescription();
            _subitems = value.GetSubItems();
        }
        return *this;
    }

private:
    int              _priority;
    std::wstring     _data;
    std::wstring     _description;
    std::vector<std::wstring> _subitems;
};

class CItem
{
public:
    CItem();
    CItem(const std::wstring& id, const std::wstring& name, const std::wstring& displayname, bool mandatory, bool multisel, int nDefaultValue, bool bEditAble, bool bHideUI);
    virtual ~CItem();

    const CItemValue* Select(int id) throw();
    void Unselect(int id) throw();
    const CItemValue* GetFirstSelectedValue() const throw();
    int GetSelectedValues(std::vector<const CItemValue*>& values);
    bool IsSelected() const throw();
    bool IsSelected(int id) const throw();

    inline const std::wstring& GetId() const throw() {return _id;}
    inline const std::wstring& GetName() const throw() {return _name;}
    inline const std::wstring& GetDisplayName() const throw() {return _displayname;}
    inline const std::vector<CItemValue>& GetValues() const throw() {return _values;}
    inline std::vector<CItemValue>& GetValues() throw() {return _values;}
    inline void AddValue(const CItemValue& value) throw() {_values.push_back(value);}
    inline int GetSelectId() const throw() {return _selid;}
	inline int GetDefaultValue() const throw() { return _defaultValue; }
	inline bool IsEditable()const throw() { return _editable;  }
	inline bool IsHideUI() const throw() { return _hideUI; }
    inline bool IsValid() const throw() {return (!_id.empty());}
    inline bool IsMandatory() const throw() {return _mandatory;}
    inline void SetMandatory(bool b) throw() {_mandatory = b;}
    inline bool IsMultiSelection() const throw() {return _multisel;}
	inline void SetComboBox(HWND hWnd) { _hWndComboBox = hWnd; }
	inline void SetDefaultValue(const wchar_t* wszDefaultValue);
	inline void SetDefaultValue(int nDefault);

    
    inline CItem& operator = (const CItem& item) throw()
    {
        if(this != &item) {
            _id = item.GetId();
            _name = item.GetName();
            _displayname = item.GetDisplayName();
            _values = item.GetValues();
            _mandatory = item.IsMandatory();
            _multisel = item.IsMultiSelection();
            _selid = item.GetSelectId();
			_defaultValue = item.GetDefaultValue();
			_editable = item.IsEditable();
			_hideUI = item.IsHideUI();
        }
        return *this;
    }

private:
    std::wstring            _id;
    std::wstring            _name;
    std::wstring            _displayname;
    std::vector<CItemValue> _values;
	CItemValue              _valueUserInput;
    bool                    _mandatory;
    bool                    _multisel;
    int                     _selid;
	int                     _defaultValue;
	bool                    _editable;
	bool                    _hideUI;
	HWND                    _hWndComboBox;
};

class CClassifyData
{
public:
    CClassifyData();
    virtual ~CClassifyData();

	void LoadFromXml(const std::wstring& xml, const std::wstring& group_name, const std::vector<std::pair<std::wstring, std::wstring>>& oldTag) throw();
    void LoadFromJson(const std::wstring& ws, const std::wstring& group_name) throw();

    inline bool IsEmpty() const throw() {return (_entry.empty() || _data.empty());}
    inline const std::vector<std::wstring>& GetEntryList() const throw() {return _entry;}
    inline const std::vector<CItem>& GetItemList() const throw() {return _data;}   
    inline std::vector<CItem>& GetItemList() throw() {return _data;}   
    inline CItem* GetItem(const std::wstring& id)
    {
        for(int i=0; i<(int)_data.size(); i++) {
            if(_data[i].GetId() == id) {
                return &_data[i];
            }
        }
        return NULL;
    }
	CItem* GetItem(const wchar_t* wszName);


protected:
	void Load(IXMLDOMDocument* doc, const std::wstring& group_name, const std::vector<std::pair<std::wstring, std::wstring>>& oldTag);
    std::wstring expand_value(const std::wstring& value) const;
    void init_dynamic_attributes();

	std::wstring GetTagValueFromKey(const std::vector<std::pair<std::wstring, std::wstring>>& vTags, const wchar_t* wszKey);

private:
    std::vector<std::wstring>   _entry;
    std::vector<CItem>  _data;
    std::wstring _group;
    std::map<std::wstring, std::wstring> _dynamic_attributes;
};

}   // namespace classify

#endif