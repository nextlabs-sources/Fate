


#ifndef __CLASSIFY_LIST_CONTROL_HPP__
#define __CLASSIFY_LIST_CONTROL_HPP__

#include "classifydata.hpp"
#include "dlgunit.hpp"

class CListItem
{
public:
    CListItem();
    CListItem(int level, const CDlgUnit& margin_unit, const CDlgUnit& caption_unit, const CDlgUnit& checkbox_unit, const CDlgUnit& combobox_unit);
    virtual ~CListItem();

    BOOL Create(HWND hListCtrl, UINT& uId, const RECT& rc, classify::CItem* item);
    void Destroy();
    void Show(BOOL bShow, RECT& rc);
    void SetReadOnly(bool readonly);

    inline const std::vector<std::pair<UINT,HWND>>& GetSubItems() const throw() {return _vItems;}
    inline int GetLevel() const throw() {return _level;}
    inline classify::CItem* GetClassifyData() {return _clsitem;}
    void SetTitleFont(HFONT hFont) {_hTitleFont=hFont;}

protected:
    HWND CreateStaticControl(HWND hListCtrl, UINT uId, const RECT& rc);
    HWND CreateComboBoxControl(HWND hListCtrl, UINT uId, const RECT& rc);
    HWND CreateCheckBoxControl(HWND hListCtrl, LPCWSTR wzText, UINT uId, const RECT& rc);


private:
    classify::CItem*    _clsitem;
    HWND                _hCaption;
    std::vector<std::pair<UINT,HWND>>   _vItems;
    int                 _level;
    HFONT               _hTitleFont;
    
    CDlgUnit            _margin_unit;
    CDlgUnit            _caption_unit;
    CDlgUnit            _checkbox_unit;
    CDlgUnit            _combobox_unit;
};

class CClassifyListCtrl
{
public:
    CClassifyListCtrl();
    virtual ~CClassifyListCtrl();

    void SetData(classify::CClassifyData* data) throw() {_data= data;}
    void SetReadOnly(_In_ bool readonly) throw() {_readonly=readonly;}
    void SetTitleFont(HFONT hFont) {_hTitleFont=hFont;}
    void GetResult(std::vector<std::pair<std::wstring,std::wstring>>& result);
    BOOL MandatoryCheck(_Out_ const classify::CItem** pEmptyItem);

    BOOL Init();
    void Clear();

    void Attach(_In_ HWND hWnd);
    void Detach();
    LRESULT ListProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct);
    void OnMeasureItem(MEASUREITEMSTRUCT* lpMeasureItem);

protected:
    BOOL InsertItem(int index, classify::CItem* clsItem, int nLevel);
    void RemoveItem(int index);

    BOOL FindItemBySubControlId(_In_ UINT dwCtrlId, _Out_ int* pnItem, _Out_ int* pnSubItem);
    void OnComboBoxSelectChange(UINT dwCtrlId);
    void OnCheckBoxStateChange(UINT dwCtrlId);

    // return next index
    int InsertClassifyItem(int index, int level, classify::CItem* pItem);

    CListItem* GetItem(int index);

    void PositionItem(int index);
    void PositionAllItems();

private:
    HWND    _hWnd;
    UINT    _uNextSubCtrlId;
    bool    _readonly;
    classify::CClassifyData* _data;
    std::list<std::tr1::shared_ptr<CListItem>> _itemList;


    WNDPROC _OldListProc;

    classify::CItem*  pInsertingItem;

    CDlgUnit _margin_unit;
    CDlgUnit _caption_unit;
    CDlgUnit _checkbox_unit;
    CDlgUnit _combobox_unit;

    HBRUSH  _hbrWhite;
    HFONT   _hTitleFont;
};


#endif