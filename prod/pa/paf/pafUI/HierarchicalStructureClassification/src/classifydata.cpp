


#include <Windows.h>


#include <boost\algorithm\string.hpp>
#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\host.hpp>
#include <nudf\time.hpp>
//#include <nudf\web\json.hpp>
#include <nudf\xmlparser.hpp>
#include "classifydata.hpp"
#include <vector>

using namespace classify;


extern void get_logon_user(std::wstring& user_name, std::wstring& user_id);

//
//  class CItemValue
//
CItemValue::CItemValue() : _priority(0)
{
}

CItemValue::CItemValue(int priority, const std::wstring& data, const std::wstring& description, const std::vector<std::wstring>& subitems)
    : _priority(priority), _data(data), _description(description), _subitems(subitems)
{
}

CItemValue::~CItemValue()
{
}

//
//  class CItem
//

CItem::CItem()
: _selid(-1), _mandatory(false), _multisel(false), _hWndComboBox(NULL)
{
}

CItem::CItem(const std::wstring& id, const std::wstring& name, const std::wstring& displayname, bool mandatory, bool multisel,
	int nDefaultValue, bool bEditAble, bool bHideUI)
	: _id(id), _selid(-1), _mandatory(mandatory), _multisel(multisel), _name(name), _displayname(displayname), 
    _defaultValue(0),_editable(bEditAble), _hideUI(bHideUI), _hWndComboBox(NULL)
{
	
	SetDefaultValue(nDefaultValue);
}

CItem::~CItem()
{
}

const CItemValue* CItem::Select(int id) throw()
{
    if(-1 == id) {
        _selid = id;
    }
    else if(id >= 0 && id < (int)_values.size()) {
        if(IsMultiSelection()) {
            if(_selid == -1) {
                _selid = 0;
            }
            ULONG mask = 0x00000001;
            // id is zero-based
            if(id > 0) {
                mask = mask << id;
            }
            _selid |= mask;
        }
        else {
            _selid = id;
        }
        return &_values[id];
    }
    else {
        ; // Do nothing
    }

    return NULL;
}

void CItem::Unselect(int id) throw()
{
    if(IsMultiSelection()) {
        if(id >= 0 && id < (int)_values.size() && id < 31) {
            if(_selid != -1) {
                ULONG mask = 0x00000001;
                // id is zero-based
                if(id > 0) {
                    mask = mask << id;
                }
                _selid &= (~mask);
            }            
        }
    }
    else {
        _selid = -1;
    }
}

void CItem::SetDefaultValue(const wchar_t* wszDefaultValue)
{
	std::vector<CItemValue>& itemValues = GetValues();
	std::vector<CItemValue>::iterator itValue = itemValues.begin();
	int nDefaultValueIndex = -1;
	while (itValue != itemValues.end())
	{
		nDefaultValueIndex++;
		if (_wcsicmp(itValue->GetData().c_str(), wszDefaultValue)==0 )
		{
			SetDefaultValue(nDefaultValueIndex);
			break;
		}
		itValue++;
	}
}

void CItem::SetDefaultValue(int nDefault)
{
	if (nDefault == -1)
	{
		if (IsMultiSelection())
		{
			_defaultValue = 0; // -1 for multi-sel tag, means not select any thing
		}
		else
		{
			_defaultValue = nDefault; // -1 for single-sel tag, means select the first tag
		}
	}
	else if (nDefault >= 0)
	{
		if (IsMultiSelection())
		{
			ULONG mask = 0x00000001;
			mask = mask << nDefault;
			_defaultValue |= mask;
		}
		else
		{
			_defaultValue = nDefault;
		}
	}
	else
	{
		//do nothing
	}
	
}

const CItemValue* CItem::GetFirstSelectedValue() const throw()
{
    if(IsMultiSelection()) {
        if(_selid <= 0) {
            return NULL;
        }
        ULONG mask = 0x00000001;
        int loop = min(31, ((int)_values.size()));
        for(int i=0; i<loop; i++) {
            if(0 != ((ULONG)_selid & mask)) {
                return &_values[i];
            }
            mask = (mask << 1);
        }
    }
    else {
        if(_selid >= 0 || _selid < (int)_values.size()) {
            return &_values[_selid];
        }
    }

    return NULL;
}

int CItem::GetSelectedValues(std::vector<const CItemValue*>& values)
{
    if(IsMultiSelection()) {
        if(_selid <= 0) {
            return 0;
        }
        ULONG mask = 0x00000001;
        int loop = min(31, ((int)_values.size()));
        for(int i=0; i<loop; i++) {
            if(0 != ((ULONG)_selid & mask)) {
                values.push_back(&_values[i]);
            }
            mask = (mask << 1);
        }
    }
    else {
		if (_hWndComboBox)
		{
			::OutputDebugStringW(L"Enter GetSelectedValues have combobox\n");
			int nCurSel = ::SendMessageW(_hWndComboBox, CB_GETCURSEL, 0, 0);
			if (nCurSel == CB_ERR)
			{  //User inputed value
				wchar_t szComboBoxString[MAX_PATH + 1] = { 0 };
				::GetWindowTextW(_hWndComboBox, szComboBoxString, MAX_PATH);
				if (wcslen(szComboBoxString))
				{
					_valueUserInput.SetData(szComboBoxString);
					values.push_back(&_valueUserInput);
				}
			}
			else
			{
				if (_selid >= 0 && _selid < (int)_values.size()) {
					values.push_back(&_values[_selid]);
				}
			}
		}
		else
		{
			if (_selid >= 0 && _selid < (int)_values.size()) {
				values.push_back(&_values[_selid]);
			}
		}
    }

    return (int)values.size();
}

bool CItem::IsSelected() const throw()
{
    if(IsMultiSelection()) {
        return (_selid > 0) ? TRUE : FALSE;
    }
    else {
        return (_selid >= 0 && _selid < (int)_values.size()) ? TRUE : FALSE;
    }
}

bool CItem::IsSelected(int id) const throw()
{
    if(IsMultiSelection()) {
        if(_selid <= 0) {
            return false;
        }
        ULONG mask = 0x00000001;
        // id is zero-based
        if(id > 0) {
            mask = mask << id;
        }
        return (mask == ((ULONG)_selid & mask));
    }
    else {
        return (_selid == id);
    }
}


//
//  class CClassifyData
//

CClassifyData::CClassifyData()
{
}

CClassifyData::~CClassifyData()
{
}

void CClassifyData::init_dynamic_attributes()
{
    try {
        std::wstring user_name;
        std::wstring user_id;
        std::wstring host_name;
        std::wstring current_time;

        get_logon_user(user_name, user_id);
        if (!user_name.empty() && !user_id.empty()) {
            _dynamic_attributes[L"user.name"] = user_name;
            _dynamic_attributes[L"user.id"] = user_id;
        }

        nudf::win::CHost host;
        host_name = host.GetHostName();
        std::transform(host_name.begin(), host_name.end(), host_name.begin(), tolower);
        _dynamic_attributes[L"host.name"] = host_name;

        SYSTEMTIME st = { 0 };
        GetLocalTime(&st);
        swprintf_s(nudf::string::tempstr<wchar_t>(current_time, 256), 256, L"%04d-%02d-%02dT%02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        _dynamic_attributes[L"current_time"] = current_time;
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void CClassifyData::LoadFromXml(const std::wstring& xml, const std::wstring& group_name, const std::vector<std::pair<std::wstring, std::wstring>>& oldTags) throw()
{
	try
	{
		nudf::util::CXmlDocument doc;
		doc.LoadFromXml(xml.c_str());
		Load(doc.GetDoc(), group_name, oldTags);
	}
	catch (...)
	{
		OutputDebugStringW(L"Exception when load Xml");
	}
  
}

CItem* CClassifyData::GetItem(const wchar_t* wszName)
{
	for (int i = 0; i < (int)_data.size(); i++)
	{
		if (_wcsicmp(_data[i].GetName().c_str(), wszName)==0)
		{
			return &_data[i];
		}
	}
	return NULL;
}

void CClassifyData::Load(IXMLDOMDocument* doc, const std::wstring& group_name, const std::vector<std::pair<std::wstring, std::wstring>>& oldTag) throw()
{
    init_dynamic_attributes();
    
    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> spRoot;
        CComPtr<IXMLDOMNode>    spClassify;
        CComPtr<IXMLDOMNode>    spGroupList;
        CComPtr<IXMLDOMNode>    spTopLevel;
        CComPtr<IXMLDOMNode>    spLabels;
        std::wstring            wsRoot;

        hr = doc->get_documentElement(&spRoot);
        if(FAILED(hr)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Find <Classify>
		/*
        wsRoot = nudf::util::XmlUtil::GetNodeName(spRoot);
        if(0 == _wcsicmp(wsRoot.c_str(), L"ClassificationProfile")) {
            if(!nudf::util::XmlUtil::FindChildElement(spRoot, L"Classify", &spClassify)) {
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            }
        }
        else {
            CComPtr<IXMLDOMNode>    spClassifyProfile;
            if(!nudf::util::XmlUtil::FindChildElement(spRoot, L"ClassificationProfile", &spClassifyProfile)) {
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            }
            if(!nudf::util::XmlUtil::FindChildElement(spClassifyProfile, L"Classify", &spClassify)) {
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            }
        }
		*/
		spClassify = spRoot;

        if(nudf::util::XmlUtil::FindChildElement(spClassify, L"Profiles", &spGroupList)) {
            // Find by name
            if(!nudf::util::XmlUtil::FindChildElement(spGroupList, group_name.c_str(), &spTopLevel)) {
                // Not found? use 'Default'
                if(!nudf::util::XmlUtil::FindChildElement(spGroupList, L"Default", &spTopLevel)) {
                    throw WIN32ERROR2(ERROR_INVALID_DATA);
                }
            }
        }
        else {
            // Maybe this is old format?
            if(!nudf::util::XmlUtil::FindChildElement(spClassify, L"TopLevel", &spTopLevel)) {
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            }
        }


        // Get Label List
        if(!nudf::util::XmlUtil::FindChildElement(spClassify, L"LabelList", &spLabels)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        std::wstring wsTopLevelList = nudf::util::XmlUtil::GetNodeText(spTopLevel);
        if(!wsTopLevelList.empty()) {
            nudf::string::Split(wsTopLevelList, L',', _entry);
        }

        CComPtr<IXMLDOMNodeList>    spLabelList;
        long                        nLabels = 0;
        hr = spLabels->get_childNodes(&spLabelList);
        if(FAILED(hr)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        hr = spLabelList->get_length(&nLabels);
        if(FAILED(hr)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        for(int i=0; i<nLabels; i++) {
            
            CComPtr<IXMLDOMNode>    spLabel;
            std::wstring            wstmp;

            hr = spLabelList->get_item(i, &spLabel);
            if(FAILED(hr)) {
                break;
            }

            wstmp = nudf::util::XmlUtil::GetNodeName(spLabel);
            if(0 != _wcsicmp(L"LABEL", wstmp.c_str())) {
                continue;
            }


            std::wstring id;
            std::wstring name;
            std::wstring displayname;
            bool mandatory = false;
            bool multisel = false;

            if(!nudf::util::XmlUtil::GetNodeAttribute(spLabel, L"id", id) || id.empty()) {
                continue;
            }
            if(!nudf::util::XmlUtil::GetNodeAttribute(spLabel, L"name", name) || name.empty()) {
                continue;
            }
            if(!nudf::util::XmlUtil::GetNodeAttribute(spLabel, L"display-name", displayname) || displayname.empty()) {
                displayname = name;
            }
            if(nudf::util::XmlUtil::GetNodeAttribute(spLabel, L"mandatory", wstmp) && 0 == _wcsicmp(L"true", wstmp.c_str())) {
                mandatory = true;
            }
            if(nudf::util::XmlUtil::GetNodeAttribute(spLabel, L"multi-select", wstmp) && 0 == _wcsicmp(L"true", wstmp.c_str())) {
                multisel = true;
            }

			int defaultValue = -1;
			if (nudf::util::XmlUtil::GetNodeAttribute(spLabel, L"default-value", wstmp))
			{
				defaultValue = _wtoi(wstmp.c_str());
			}

			bool bEditAble = true;
			if (nudf::util::XmlUtil::GetNodeAttribute(spLabel, L"usereditable", wstmp) && 0 == _wcsicmp(L"false", wstmp.c_str())) {
				bEditAble = false;
			}

			bool bHidenUI = false;
			if (nudf::util::XmlUtil::GetNodeAttribute(spLabel, L"hidden", wstmp) && 0 == _wcsicmp(L"true", wstmp.c_str())) {
				bHidenUI = true;
			}

            CItem item(id, name, displayname, mandatory, multisel, defaultValue, bEditAble, bHidenUI);

            CComPtr<IXMLDOMNodeList>    spValueList;
            long                        nValues = 0;
            hr = spLabel->get_childNodes(&spValueList);
            if(FAILED(hr)) {
                continue;
            }
            hr = spValueList->get_length(&nValues);
            if(FAILED(hr) || 0 == nValues) {
                continue;
            }

            for(int j=0; j<nValues; j++) {

                CComPtr<IXMLDOMNode>    spValue;
                std::wstring            value;
                std::wstring            subitems;
                std::wstring            description;
                int                     priority = 0;
                std::vector<std::wstring>        vSubValues;

                hr = spValueList->get_item(j, &spValue);
                if(FAILED(hr) || NULL == spValue.p) {
                    break;
                }

                wstmp = nudf::util::XmlUtil::GetNodeName(spValue);
                if(0 != _wcsicmp(L"VALUE", wstmp.c_str())) {
                    continue;
                }

                if(!nudf::util::XmlUtil::GetNodeAttribute(spValue, L"value", value) || value.empty()) {
                    continue;
                }
                if(!nudf::util::XmlUtil::GetNodeAttribute(spValue, L"priority", &priority) || priority < 0) {
                    priority = 0;
                }
                if(nudf::util::XmlUtil::GetNodeAttribute(spValue, L"sub-label", subitems) && !subitems.empty()) {
                    nudf::string::Split(subitems, L',', vSubValues);
                }
                nudf::util::XmlUtil::GetNodeAttribute(spValue, L"description", description);
                // expand the value if it is necessary
                value = expand_value(value);
                item.AddValue(CItemValue(priority, value, description, vSubValues));
            }

//             if(!multisel) {
//                 int default_id = -1;
//                 if(nudf::util::XmlUtil::GetNodeAttribute(spLabel, L"default-value", &default_id) && default_id >= 0 && default_id < (int)item.GetValues().size()) {
//                     item.Select(default_id);
//                 }
//             }

			//if document contains this tag already, we set the old value to the default one.
			if (bEditAble && (!bHidenUI) )
			{
				std::vector<std::pair<std::wstring, std::wstring>>::const_iterator itTag = oldTag.begin();
				while (itTag != oldTag.end())
				{
					if (_wcsicmp(itTag->first.c_str(), name.c_str()) == 0)
					{
						item.SetDefaultValue(itTag->second.c_str());

						if (!multisel)
						{
							break;
						}
					}
					itTag++;
				}
			}

            // Add this item
            _data.push_back(item);
        }
    }
    catch(const nudf::CException& e) {
        _entry.clear();
        _data.clear();
        UNREFERENCED_PARAMETER(e);
    }
}

std::wstring CClassifyData::expand_value(const std::wstring& value) const
{
    std::wstring expanded_value;

    if (boost::algorithm::starts_with(value, L"$(") && boost::algorithm::ends_with(value, L")")) {
        expanded_value = value.substr(2);
        expanded_value = expanded_value.substr(0, expanded_value.length()-1);
        std::transform(expanded_value.begin(), expanded_value.end(), expanded_value.begin(), tolower);
		std::map<std::wstring, std::wstring>::const_iterator pos = _dynamic_attributes.find(expanded_value);
        if (pos != _dynamic_attributes.end()) {
            expanded_value = (*pos).second;
        }
    }
    else {
        expanded_value = value;
    }

	return expanded_value;//std::move(expanded_value);
}

void CClassifyData::LoadFromJson(const std::wstring& ws, const std::wstring& groupname) throw()
{
#if 1
#pragma  message("not implement LoadFromJson")
#else
    init_dynamic_attributes();

    try {

        NX::web::json::value v = NX::web::json::value::parse(ws);

        if (!v.has_field(L"profiles")) {
            throw std::exception("element \"profiles\" not exists");
        }
        if (!v.has_field(L"labels")) {
            throw std::exception("element \"labels\" not exists");
        }
        if(!v[L"profiles"].is_object()) {
            throw std::exception("wrong type of element \"profiles\" (not object)");
        }
        if(!v[L"labels"].is_object()) {
            throw std::exception("wrong type of element \"labels\" (not object)");
        }

        std::wstring group_name = groupname;
        std::transform(group_name.begin(), group_name.end(), group_name.begin(), tolower);
        if (!v[L"profiles"].has_field(group_name)) {
            throw std::exception("group profile not exists");
        }

        NX::web::json::array& top_labels = v[L"profiles"][group_name].as_array();
        std::for_each(top_labels.begin(), top_labels.end(), [&](const NX::web::json::value& lb) {
            wchar_t* pend = NULL;
            _entry.push_back(lb.as_string());
        });

        NX::web::json::object& labels = v[L"labels"].as_object();
        std::for_each(labels.begin(), labels.end(), [&](const std::pair<std::wstring,NX::web::json::value>& lb) {
            const NX::web::json::object& label = lb.second.as_object();
            std::wstring label_id = lb.second.at(L"id").as_string();
            std::wstring label_name = lb.second.at(L"name").as_string();
            std::wstring label_display_name  = lb.second.at(L"display-name").as_string();
            int          label_default_value = lb.second.at(L"default-value").as_integer();
            bool         label_multi_select  = lb.second.at(L"multi-select").as_bool();
            bool         label_mandatory     = lb.second.at(L"mandatory").as_bool();
			bool         label_editable = lb.second.at(L"usereditable").as_bool();
			bool         label_hideUI = lb.second.at(L"hidden").as_bool();

            CItem item(label_id, label_name, label_display_name, label_mandatory, label_multi_select, label_default_value, label_editable, label_hideUI);
            const NX::web::json::array& label_values = lb.second.at(L"values").as_array();
            for (auto it = label_values.begin(); it != label_values.end(); ++it) {
                std::wstring v_value = expand_value((*it).at(L"value").as_string());
                std::wstring v_sub_labels = (*it).at(L"sub-label").as_string();
                std::wstring v_description = (*it).at(L"description").as_string();
                int          v_priority = (*it).at(L"priority").as_integer();
                std::vector<std::wstring> sub_label_vector;
                nudf::string::Split<wchar_t>(v_sub_labels, L',', sub_label_vector);
                item.AddValue(CItemValue(v_priority, v_value, v_description, sub_label_vector));
            }
            if (!label_multi_select) {
                if (label_default_value >= 0 && label_default_value < (int)item.GetValues().size()) {
                    item.Select(label_default_value);
                }
            }
            _data.push_back(item);
        });
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
#endif
}


std::wstring  CClassifyData::GetTagValueFromKey(const std::vector<std::pair<std::wstring, std::wstring>>& vTags, const wchar_t* wszKey)
{
	std::vector<std::pair<std::wstring, std::wstring>>::const_iterator itTag = vTags.begin();
	while (itTag != vTags.end())
	{
		if (_wcsicmp(itTag->first.c_str(), wszKey)==0 )
		{
			return itTag->second;
		}

		itTag++;
	}
	return L"";
}
