#include "stdafx.h"
#include "NLVisualLabelingPA.h"
#include "ExcelDocLabeling.h"
#include "Utility.h"

#if defined(MSO2K3)

#pragma implementation_key(5761)
inline Excel::_excelApplicationPtr Excel::excelShapes::GetApplication ( ) {
	struct _excelApplication * _result = 0;
	_com_dispatch_method(this, 0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return _excelApplicationPtr(_result, false);
}

#pragma implementation_key(5762)
inline enum Excel::XlCreator Excel::excelShapes::GetCreator ( ) {
	enum XlCreator _result;
	_com_dispatch_method(this, 0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5763)
inline IDispatchPtr Excel::excelShapes::GetParent ( ) {
	IDispatch * _result = 0;
	_com_dispatch_method(this, 0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return IDispatchPtr(_result, false);
}

#pragma implementation_key(5764)
inline long Excel::excelShapes::GetCount ( ) {
	long _result = 0;
	_com_dispatch_method(this, 0x76, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5765)
inline Excel::excelShapePtr Excel::excelShapes::Item ( const _variant_t & Index ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0xaa, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x000c", &Index);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5766)
inline Excel::excelShapePtr Excel::excelShapes::_Default ( const _variant_t & Index ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x0, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x000c", &Index);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5767)
inline IUnknownPtr Excel::excelShapes::Get_NewEnum ( ) {
	IUnknown * _result = 0;
	_com_dispatch_method(this, 0xfffffffc, DISPATCH_PROPERTYGET, VT_UNKNOWN, (void*)&_result, NULL);
	return IUnknownPtr(_result, false);
}

#pragma implementation_key(5768)
inline Excel::excelShapePtr Excel::excelShapes::AddCallout ( enum Office::MsoCalloutType Type, float Left, float Top, float Width, float Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6b1, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0004\x0004\x0004\x0004", Type, Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5769)
inline Excel::excelShapePtr Excel::excelShapes::AddConnector ( enum Office::MsoConnectorType Type, float BeginX, float BeginY, float EndX, float EndY ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6b2, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0004\x0004\x0004\x0004", Type, BeginX, BeginY, EndX, EndY);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5770)
inline Excel::excelShapePtr Excel::excelShapes::AddCurve ( const _variant_t & SafeArrayOfPoints ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6b7, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x000c", &SafeArrayOfPoints);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5771)
inline Excel::excelShapePtr Excel::excelShapes::AddLabel ( enum Office::MsoTextOrientation Orientation, float Left, float Top, float Width, float Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6b9, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0004\x0004\x0004\x0004", Orientation, Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5772)
inline Excel::excelShapePtr Excel::excelShapes::AddLine ( float BeginX, float BeginY, float EndX, float EndY ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6ba, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0004\x0004\x0004\x0004", BeginX, BeginY, EndX, EndY);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5773)
inline Excel::excelShapePtr Excel::excelShapes::AddPicture ( _bstr_t Filename, enum Office::MsoTriState LinkToFile, enum Office::MsoTriState SaveWithDocument, float Left, float Top, float Width, float Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6bb, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0008\x0003\x0003\x0004\x0004\x0004\x0004", (BSTR)Filename, LinkToFile, SaveWithDocument, Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5774)
inline Excel::excelShapePtr Excel::excelShapes::AddPolyline ( const _variant_t & SafeArrayOfPoints ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6be, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x000c", &SafeArrayOfPoints);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5775)
inline Excel::excelShapePtr Excel::excelShapes::AddShape ( enum Office::MsoAutoShapeType Type, float Left, float Top, float Width, float Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6bf, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0004\x0004\x0004\x0004", Type, Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5776)
inline Excel::excelShapePtr Excel::excelShapes::AddTextEffect ( enum Office::MsoPresetTextEffect PresetTextEffect, _bstr_t Text, _bstr_t FontName, float FontSize, enum Office::MsoTriState FontBold, enum Office::MsoTriState FontItalic, float Left, float Top ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6c0, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0008\x0008\x0004\x0003\x0003\x0004\x0004", PresetTextEffect, (BSTR)Text, (BSTR)FontName, FontSize, FontBold, FontItalic, Left, Top);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5777)
inline Excel::excelShapePtr Excel::excelShapes::AddTextbox ( enum Office::MsoTextOrientation Orientation, float Left, float Top, float Width, float Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6c6, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0004\x0004\x0004\x0004", Orientation, Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5778)
inline Excel::excelFreeformBuilderPtr Excel::excelShapes::BuildFreeform ( enum Office::MsoEditingType EditingType, float X1, float Y1 ) {
	struct excelFreeformBuilder * _result = 0;
	_com_dispatch_method(this, 0x6c7, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0004\x0004", EditingType, X1, Y1);
	return excelFreeformBuilderPtr(_result, false);
}

#pragma implementation_key(5779)
inline Excel::excelShapeRangePtr Excel::excelShapes::GetRange ( const _variant_t & Index ) {
	struct excelShapeRange * _result = 0;
	_com_dispatch_method(this, 0xc5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, 
		L"\x000c", &Index);
	return excelShapeRangePtr(_result, false);
}

#pragma implementation_key(5780)
inline HRESULT Excel::excelShapes::SelectAll ( ) {
	return _com_dispatch_method(this, 0x6c9, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5781)
inline Excel::excelShapePtr Excel::excelShapes::AddFormControl ( enum XlFormControl Type, long Left, long Top, long Width, long Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6ca, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0003\x0003\x0003\x0003", Type, Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5782)
inline Excel::excelShapePtr Excel::excelShapes::AddOLEObject ( const _variant_t & ClassType, const _variant_t & Filename, const _variant_t & Link, const _variant_t & DisplayAsIcon, const _variant_t & IconFileName, const _variant_t & IconIndex, const _variant_t & IconLabel, const _variant_t & Left, const _variant_t & Top, const _variant_t & Width, const _variant_t & Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6cb, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x080c\x080c\x080c\x080c\x080c\x080c\x080c\x080c\x080c\x080c\x080c", &ClassType, &Filename, &Link, &DisplayAsIcon, &IconFileName, &IconIndex, &IconLabel, &Left, &Top, &Width, &Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5783)
inline Excel::excelShapePtr Excel::excelShapes::AddDiagram ( enum Office::MsoDiagramType Type, float Left, float Top, float Width, float Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x880, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0004\x0004\x0004\x0004", Type, Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5784)
inline Excel::excelShapePtr Excel::excelShapes::AddCanvas ( float Left, float Top, float Width, float Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x881, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0004\x0004\x0004\x0004", Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

//
// dispinterface Shape wrapper method implementations
//

#pragma implementation_key(5672)
inline Excel::_excelApplicationPtr Excel::excelShape::GetApplication ( ) {
	struct _excelApplication * _result = 0;
	_com_dispatch_method(this, 0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return _excelApplicationPtr(_result, false);
}

#pragma implementation_key(5673)
inline enum Excel::XlCreator Excel::excelShape::GetCreator ( ) {
	enum XlCreator _result;
	_com_dispatch_method(this, 0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5674)
inline IDispatchPtr Excel::excelShape::GetParent ( ) {
	IDispatch * _result = 0;
	_com_dispatch_method(this, 0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return IDispatchPtr(_result, false);
}

#pragma implementation_key(5675)
inline HRESULT Excel::excelShape::Apply ( ) {
	return _com_dispatch_method(this, 0x68b, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5676)
inline HRESULT Excel::excelShape::Delete ( ) {
	return _com_dispatch_method(this, 0x75, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5677)
inline Excel::excelShapePtr Excel::excelShape::Duplicate ( ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x40f, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, NULL);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5678)
inline HRESULT Excel::excelShape::Flip ( enum Office::MsoFlipCmd FlipCmd ) {
	return _com_dispatch_method(this, 0x68c, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0003", FlipCmd);
}

#pragma implementation_key(5679)
inline HRESULT Excel::excelShape::IncrementLeft ( float Increment ) {
	return _com_dispatch_method(this, 0x68e, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5680)
inline HRESULT Excel::excelShape::IncrementRotation ( float Increment ) {
	return _com_dispatch_method(this, 0x690, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5681)
inline HRESULT Excel::excelShape::IncrementTop ( float Increment ) {
	return _com_dispatch_method(this, 0x691, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5682)
inline HRESULT Excel::excelShape::PickUp ( ) {
	return _com_dispatch_method(this, 0x692, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5683)
inline HRESULT Excel::excelShape::RerouteConnections ( ) {
	return _com_dispatch_method(this, 0x693, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5684)
inline HRESULT Excel::excelShape::ScaleHeight ( float Factor, enum Office::MsoTriState RelativeToOriginalSize, const _variant_t & Scale ) {
	return _com_dispatch_method(this, 0x694, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0004\x0003\x080c", Factor, RelativeToOriginalSize, &Scale);
}

#pragma implementation_key(5685)
inline HRESULT Excel::excelShape::ScaleWidth ( float Factor, enum Office::MsoTriState RelativeToOriginalSize, const _variant_t & Scale ) {
	return _com_dispatch_method(this, 0x698, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0004\x0003\x080c", Factor, RelativeToOriginalSize, &Scale);
}

#pragma implementation_key(5686)
inline HRESULT Excel::excelShape::Select ( const _variant_t & Replace ) {
	return _com_dispatch_method(this, 0xeb, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x080c", &Replace);
}


#pragma implementation_key(5687)
inline HRESULT Excel::excelShape::SetShapesDefaultProperties ( ) {
	return _com_dispatch_method(this, 0x699, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5688)
inline Excel::excelShapeRangePtr Excel::excelShape::Ungroup ( ) {
	struct excelShapeRange * _result = 0;
	_com_dispatch_method(this, 0xf4, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, NULL);
	return excelShapeRangePtr(_result, false);
}

#pragma implementation_key(5689)
inline HRESULT Excel::excelShape::ZOrder ( enum Office::MsoZOrderCmd ZOrderCmd ) {
	return _com_dispatch_method(this, 0x26e, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0003", ZOrderCmd);
}

#pragma implementation_key(5690)
inline Excel::excelAdjustmentsPtr Excel::excelShape::GetAdjustments ( ) {
	struct excelAdjustments * _result = 0;
	_com_dispatch_method(this, 0x69b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelAdjustmentsPtr(_result, false);
}

#pragma implementation_key(5691)
inline Excel::excelTextFramePtr Excel::excelShape::GetTextFrame ( ) {
	struct excelTextFrame * _result = 0;
	_com_dispatch_method(this, 0x69c, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelTextFramePtr(_result, false);
}

#pragma implementation_key(5692)
inline enum Office::MsoAutoShapeType Excel::excelShape::GetAutoShapeType ( ) {
	enum Office::MsoAutoShapeType _result;
	_com_dispatch_method(this, 0x69d, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5693)
inline void Excel::excelShape::PutAutoShapeType ( enum Office::MsoAutoShapeType _arg1 ) {
	_com_dispatch_method(this, 0x69d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5694)
inline Excel::excelCalloutFormatPtr Excel::excelShape::GetCallout ( ) {
	struct excelCalloutFormat * _result = 0;
	_com_dispatch_method(this, 0x69e, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelCalloutFormatPtr(_result, false);
}

#pragma implementation_key(5695)
inline long Excel::excelShape::GetConnectionSiteCount ( ) {
	long _result = 0;
	_com_dispatch_method(this, 0x69f, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5696)
inline enum Office::MsoTriState Excel::excelShape::GetConnector ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x6a0, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5697)
inline Excel::excelConnectorFormatPtr Excel::excelShape::GetConnectorFormat ( ) {
	struct excelConnectorFormat * _result = 0;
	_com_dispatch_method(this, 0x6a1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelConnectorFormatPtr(_result, false);
}

#pragma implementation_key(5698)
inline Excel::excelFillFormatPtr Excel::excelShape::GetFill ( ) {
	struct excelFillFormat * _result = 0;
	_com_dispatch_method(this, 0x67f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelFillFormatPtr(_result, false);
}

#pragma implementation_key(5699)
inline Excel::excelGroupShapesPtr Excel::excelShape::GetGroupItems ( ) {
	struct excelGroupShapes * _result = 0;
	_com_dispatch_method(this, 0x6a2, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelGroupShapesPtr(_result, false);
}

#pragma implementation_key(5700)
inline float Excel::excelShape::GetHeight ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x7b, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5701)
inline void Excel::excelShape::PutHeight ( float _arg1 ) {
	_com_dispatch_method(this, 0x7b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5702)
inline enum Office::MsoTriState Excel::excelShape::GetHorizontalFlip ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x6a3, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5703)
inline float Excel::excelShape::GetLeft ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x7f, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5704)
inline void Excel::excelShape::PutLeft ( float _arg1 ) {
	_com_dispatch_method(this, 0x7f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5705)
inline Excel::excelLineFormatPtr Excel::excelShape::GetLine ( ) {
	struct excelLineFormat * _result = 0;
	_com_dispatch_method(this, 0x331, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelLineFormatPtr(_result, false);
}

#pragma implementation_key(5706)
inline enum Office::MsoTriState Excel::excelShape::GetLockAspectRatio ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x6a4, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5707)
inline void Excel::excelShape::PutLockAspectRatio ( enum Office::MsoTriState _arg1 ) {
	_com_dispatch_method(this, 0x6a4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5708)
inline _bstr_t Excel::excelShape::GetName ( ) {
	BSTR _result = 0;
	_com_dispatch_method(this, 0x6e, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&_result, NULL);
	return _bstr_t(_result, false);
}

#pragma implementation_key(5709)
inline void Excel::excelShape::PutName ( _bstr_t _arg1 ) {
	_com_dispatch_method(this, 0x6e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0008", (BSTR)_arg1);
}

#pragma implementation_key(5710)
inline Excel::excelShapeNodesPtr Excel::excelShape::GetNodes ( ) {
	struct excelShapeNodes * _result = 0;
	_com_dispatch_method(this, 0x6a5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelShapeNodesPtr(_result, false);
}

#pragma implementation_key(5711)
inline float Excel::excelShape::GetRotation ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x3b, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5712)
inline void Excel::excelShape::PutRotation ( float _arg1 ) {
	_com_dispatch_method(this, 0x3b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5713)
inline Excel::excelPictureFormatPtr Excel::excelShape::GetPictureFormat ( ) {
	struct excelPictureFormat * _result = 0;
	_com_dispatch_method(this, 0x65f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelPictureFormatPtr(_result, false);
}

#pragma implementation_key(5714)
inline Excel::excelShadowFormatPtr Excel::excelShape::GetShadow ( ) {
	struct excelShadowFormat * _result = 0;
	_com_dispatch_method(this, 0x67, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelShadowFormatPtr(_result, false);
}

#pragma implementation_key(5715)
inline Excel::excelTextEffectFormatPtr Excel::excelShape::GetTextEffect ( ) {
	struct excelTextEffectFormat * _result = 0;
	_com_dispatch_method(this, 0x6a6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelTextEffectFormatPtr(_result, false);
}

#pragma implementation_key(5716)
inline Excel::excelThreeDFormatPtr Excel::excelShape::GetThreeD ( ) {
	struct excelThreeDFormat * _result = 0;
	_com_dispatch_method(this, 0x6a7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelThreeDFormatPtr(_result, false);
}

#pragma implementation_key(5717)
inline float Excel::excelShape::GetTop ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x7e, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5718)
inline void Excel::excelShape::PutTop ( float _arg1 ) {
	_com_dispatch_method(this, 0x7e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5719)
inline enum Office::MsoShapeType Excel::excelShape::GetType ( ) {
	enum Office::MsoShapeType _result;
	_com_dispatch_method(this, 0x6c, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5720)
inline enum Office::MsoTriState Excel::excelShape::GetVerticalFlip ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x6a8, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5721)
inline _variant_t Excel::excelShape::GetVertices ( ) {
	VARIANT _result;
	VariantInit(&_result);
	_com_dispatch_method(this, 0x26d, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&_result, NULL);
	return _variant_t(_result, false);
}

#pragma implementation_key(5722)
inline enum Office::MsoTriState Excel::excelShape::GetVisible ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x22e, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5723)
inline void Excel::excelShape::PutVisible ( enum Office::MsoTriState _arg1 ) {
	_com_dispatch_method(this, 0x22e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5724)
inline float Excel::excelShape::GetWidth ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x7a, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5725)
inline void Excel::excelShape::PutWidth ( float _arg1 ) {
	_com_dispatch_method(this, 0x7a, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5726)
inline long Excel::excelShape::GetZOrderPosition ( ) {
	long _result = 0;
	_com_dispatch_method(this, 0x6a9, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5727)
inline Excel::excelHyperlinkPtr Excel::excelShape::GetHyperlink ( ) {
	struct excelHyperlink * _result = 0;
	_com_dispatch_method(this, 0x6aa, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelHyperlinkPtr(_result, false);
}

#pragma implementation_key(5728)
inline enum Office::MsoBlackWhiteMode Excel::excelShape::GetBlackWhiteMode ( ) {
	enum Office::MsoBlackWhiteMode _result;
	_com_dispatch_method(this, 0x6ab, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5729)
inline void Excel::excelShape::PutBlackWhiteMode ( enum Office::MsoBlackWhiteMode _arg1 ) {
	_com_dispatch_method(this, 0x6ab, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5730)
inline IDispatchPtr Excel::excelShape::GetDrawingObject ( ) {
	IDispatch * _result = 0;
	_com_dispatch_method(this, 0x6ac, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return IDispatchPtr(_result, false);
}

#pragma implementation_key(5731)
inline _bstr_t Excel::excelShape::GetOnAction ( ) {
	BSTR _result = 0;
	_com_dispatch_method(this, 0x254, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&_result, NULL);
	return _bstr_t(_result, false);
}

#pragma implementation_key(5732)
inline void Excel::excelShape::PutOnAction ( _bstr_t _arg1 ) {
	_com_dispatch_method(this, 0x254, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0008", (BSTR)_arg1);
}

#pragma implementation_key(5733)
inline VARIANT_BOOL Excel::excelShape::GetLocked ( ) {
	VARIANT_BOOL _result = 0;
	_com_dispatch_method(this, 0x10d, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5734)
inline void Excel::excelShape::PutLocked ( VARIANT_BOOL _arg1 ) {
	_com_dispatch_method(this, 0x10d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x000b", _arg1);
}

#pragma implementation_key(5735)
inline Excel::RangePtr Excel::excelShape::GetTopLeftCell ( ) {
	struct Range * _result = 0;
	_com_dispatch_method(this, 0x26c, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return RangePtr(_result, false);
}

#pragma implementation_key(5736)
inline Excel::RangePtr Excel::excelShape::GetBottomRightCell ( ) {
	struct Range * _result = 0;
	_com_dispatch_method(this, 0x267, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return RangePtr(_result, false);
}

#pragma implementation_key(5737)
inline enum Excel::XlPlacement Excel::excelShape::GetPlacement ( ) {
	enum XlPlacement _result;
	_com_dispatch_method(this, 0x269, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5738)
inline void Excel::excelShape::PutPlacement ( enum XlPlacement _arg1 ) {
	_com_dispatch_method(this, 0x269, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5739)
inline HRESULT Excel::excelShape::Copy ( ) {
	return _com_dispatch_method(this, 0x227, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5740)
inline HRESULT Excel::excelShape::Cut ( ) {
	return _com_dispatch_method(this, 0x235, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5741)
inline HRESULT Excel::excelShape::CopyPicture ( const _variant_t & Appearance, const _variant_t & Format ) {
	return _com_dispatch_method(this, 0xd5, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x080c\x080c", &Appearance, &Format);
}

#pragma implementation_key(5742)
inline Excel::ControlFormatPtr Excel::excelShape::GetControlFormat ( ) {
	struct ControlFormat * _result = 0;
	_com_dispatch_method(this, 0x6ad, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return ControlFormatPtr(_result, false);
}

#pragma implementation_key(5743)
inline Excel::excelLinkFormatPtr Excel::excelShape::GetLinkFormat ( ) {
	struct excelLinkFormat * _result = 0;
	_com_dispatch_method(this, 0x6ae, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelLinkFormatPtr(_result, false);
}

#pragma implementation_key(5744)
inline Excel::excelOLEFormatPtr Excel::excelShape::GetOLEFormat ( ) {
	struct excelOLEFormat * _result = 0;
	_com_dispatch_method(this, 0x6af, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelOLEFormatPtr(_result, false);
}

#pragma implementation_key(5745)
inline enum Excel::XlFormControl Excel::excelShape::GetFormControlType ( ) {
	enum XlFormControl _result;
	_com_dispatch_method(this, 0x6b0, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5746)
inline _bstr_t Excel::excelShape::GetAlternativeText ( ) {
	BSTR _result = 0;
	_com_dispatch_method(this, 0x763, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&_result, NULL);
	return _bstr_t(_result, false);
}

#pragma implementation_key(5747)
inline void Excel::excelShape::PutAlternativeText ( _bstr_t _arg1 ) {
	_com_dispatch_method(this, 0x763, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0008", (BSTR)_arg1);
}

#pragma implementation_key(5748)
inline Office::ScriptPtr Excel::excelShape::GetScript ( ) {
	struct Office::Script * _result = 0;
	_com_dispatch_method(this, 0x764, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return Office::ScriptPtr(_result, false);
}

#pragma implementation_key(5749)
inline Excel::excelDiagramNodePtr Excel::excelShape::GetDiagramNode ( ) {
	struct excelDiagramNode * _result = 0;
	_com_dispatch_method(this, 0x875, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelDiagramNodePtr(_result, false);
}

#pragma implementation_key(5750)
inline enum Office::MsoTriState Excel::excelShape::GetHasDiagramNode ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x876, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5751)
inline Excel::excelDiagramPtr Excel::excelShape::GetDiagram ( ) {
	struct excelDiagram * _result = 0;
	_com_dispatch_method(this, 0x877, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelDiagramPtr(_result, false);
}

#pragma implementation_key(5752)
inline enum Office::MsoTriState Excel::excelShape::GetHasDiagram ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x878, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5753)
inline enum Office::MsoTriState Excel::excelShape::GetChild ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x879, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5754)
inline Excel::excelShapePtr Excel::excelShape::GetParentGroup ( ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x87a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5755)
inline Office::CanvasShapesPtr Excel::excelShape::GetCanvasItems ( ) {
	struct Office::CanvasShapes * _result = 0;
	_com_dispatch_method(this, 0x87b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return Office::CanvasShapesPtr(_result, false);
}

#pragma implementation_key(5756)
inline long Excel::excelShape::GetID ( ) {
	long _result = 0;
	_com_dispatch_method(this, 0x23a, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5757)
inline HRESULT Excel::excelShape::CanvasCropLeft ( float Increment ) {
	return _com_dispatch_method(this, 0x87c, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5758)
inline HRESULT Excel::excelShape::CanvasCropTop ( float Increment ) {
	return _com_dispatch_method(this, 0x87d, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5759)
inline HRESULT Excel::excelShape::CanvasCropRight ( float Increment ) {
	return _com_dispatch_method(this, 0x87e, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5760)
inline HRESULT Excel::excelShape::CanvasCropBottom ( float Increment ) {
	return _com_dispatch_method(this, 0x87f, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}
#elif defined(MSO2K7)

#pragma implementation_key(5763)
inline IDispatchPtr Excel::excelShapes::GetParent ( ) {
	IDispatch * _result = 0;
	_com_dispatch_method(this, 0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return IDispatchPtr(_result, false);
}

#pragma implementation_key(5764)
inline int Excel::excelShapes::GetCount ( ) {
	int _result = 0;
	_com_dispatch_method(this, 0x76, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5765)
inline Excel::excelShapePtr Excel::excelShapes::Item ( const _variant_t & Index ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0xaa, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x000c", &Index);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5767)
inline IUnknownPtr Excel::excelShapes::Get_NewEnum ( ) {
	IUnknown * _result = 0;
	_com_dispatch_method(this, 0xfffffffc, DISPATCH_PROPERTYGET, VT_UNKNOWN, (void*)&_result, NULL);
	return IUnknownPtr(_result, false);
}

#pragma implementation_key(5770)
inline Excel::excelShapePtr Excel::excelShapes::AddCurve ( const _variant_t & SafeArrayOfPoints ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6b7, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x000c", &SafeArrayOfPoints);
	return excelShapePtr(_result, false);
}


#pragma implementation_key(5772)
inline Excel::excelShapePtr Excel::excelShapes::AddLine ( float BeginX, float BeginY, float EndX, float EndY ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6ba, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0004\x0004\x0004\x0004", BeginX, BeginY, EndX, EndY);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5773)
inline Excel::excelShapePtr Excel::excelShapes::AddPicture (
	_bstr_t FileName,
	enum MsoTriState LinkToFile,
	enum MsoTriState SaveWithDocument,
	float Left,
	float Top,
	float Width,
	float Height )
{
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6bb, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0008\x0003\x0003\x0004\x0004\x0004\x0004", (BSTR)FileName, LinkToFile, SaveWithDocument, Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5774)
inline Excel::excelShapePtr Excel::excelShapes::AddPolyline ( const _variant_t & SafeArrayOfPoints ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6be, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x000c", &SafeArrayOfPoints);
	return excelShapePtr(_result, false);
}


#pragma implementation_key(5776)
inline Excel::excelShapePtr Excel::excelShapes::AddTextEffect(
	Office::MsoPresetTextEffect PresetTextEffect,
	_bstr_t Text,
	_bstr_t FontName,
	float FontSize,
	Office::MsoTriState FontBold,
	Office::MsoTriState FontItalic,
	float Left,
	float Top )
{
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6c0, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0003\x0008\x0008\x0004\x0003\x0003\x0004\x0004", PresetTextEffect, (BSTR)Text, (BSTR)FontName, FontSize, FontBold, FontItalic, Left, Top);
	return excelShapePtr(_result, false);
}


#pragma implementation_key(5780)
inline HRESULT Excel::excelShapes::SelectAll ( ) {
	return _com_dispatch_method(this, 0x6c9, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}



#pragma implementation_key(5784)
inline Excel::excelShapePtr Excel::excelShapes::AddCanvas ( float Left, float Top, float Width, float Height ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x881, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0004\x0004\x0004\x0004", Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

//
// dispinterface Shape wrapper method implementations
//

#pragma implementation_key(5672)
inline Excel::_excelApplicationPtr Excel::excelShape::GetApplication ( ) {
	struct _excelApplication * _result = 0;
	_com_dispatch_method(this, 0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return _excelApplicationPtr(_result, false);
}

#pragma implementation_key(5673)
inline enum Excel::XlCreator Excel::excelShape::GetCreator ( ) {
	enum XlCreator _result;
	_com_dispatch_method(this, 0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5674)
inline IDispatchPtr Excel::excelShape::GetParent ( ) {
	IDispatch * _result = 0;
	_com_dispatch_method(this, 0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return IDispatchPtr(_result, false);
}

#pragma implementation_key(5675)
inline HRESULT Excel::excelShape::Apply ( ) {
	return _com_dispatch_method(this, 0x68b, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5676)
inline HRESULT Excel::excelShape::Delete ( ) {
	return _com_dispatch_method(this, 0x75, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5677)
inline Excel::excelShapePtr Excel::excelShape::Duplicate ( ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x40f, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, NULL);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5678)
inline HRESULT Excel::excelShape::Flip ( enum Office::MsoFlipCmd FlipCmd ) {
	return _com_dispatch_method(this, 0x68c, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0003", FlipCmd);
}

#pragma implementation_key(5679)
inline HRESULT Excel::excelShape::IncrementLeft ( float Increment ) {
	return _com_dispatch_method(this, 0x68e, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5680)
inline HRESULT Excel::excelShape::IncrementRotation ( float Increment ) {
	return _com_dispatch_method(this, 0x690, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5681)
inline HRESULT Excel::excelShape::IncrementTop ( float Increment ) {
	return _com_dispatch_method(this, 0x691, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5682)
inline HRESULT Excel::excelShape::PickUp ( ) {
	return _com_dispatch_method(this, 0x692, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5683)
inline HRESULT Excel::excelShape::RerouteConnections ( ) {
	return _com_dispatch_method(this, 0x693, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5684)
inline HRESULT Excel::excelShape::ScaleHeight ( float Factor, enum Office::MsoTriState RelativeToOriginalSize, const _variant_t & Scale ) {
	return _com_dispatch_method(this, 0x694, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0004\x0003\x080c", Factor, RelativeToOriginalSize, &Scale);
}

#pragma implementation_key(5685)
inline HRESULT Excel::excelShape::ScaleWidth ( float Factor, enum Office::MsoTriState RelativeToOriginalSize, const _variant_t & Scale ) {
	return _com_dispatch_method(this, 0x698, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0004\x0003\x080c", Factor, RelativeToOriginalSize, &Scale);
}

#pragma implementation_key(5686)
inline HRESULT Excel::excelShape::Select ( const _variant_t & Replace ) {
	return _com_dispatch_method(this, 0xeb, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x080c", &Replace);
}

#pragma implementation_key(5687)
inline HRESULT Excel::excelShape::SetShapesDefaultProperties ( ) {
	return _com_dispatch_method(this, 0x699, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5688)
inline Excel::excelShapeRangePtr Excel::excelShape::Ungroup ( ) {
	struct excelShapeRange * _result = 0;
	_com_dispatch_method(this, 0xf4, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, NULL);
	return excelShapeRangePtr(_result, false);
}

#pragma implementation_key(5689)
inline HRESULT Excel::excelShape::ZOrder ( enum Office::MsoZOrderCmd ZOrderCmd ) {
	return _com_dispatch_method(this, 0x26e, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0003", ZOrderCmd);
}

#pragma implementation_key(5690)
inline Excel::excelAdjustmentsPtr Excel::excelShape::GetAdjustments ( ) {
	struct excelAdjustments * _result = 0;
	_com_dispatch_method(this, 0x69b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelAdjustmentsPtr(_result, false);
}

#pragma implementation_key(5691)
inline Excel::excelTextFramePtr Excel::excelShape::GetTextFrame ( ) {
	struct excelTextFrame * _result = 0;
	_com_dispatch_method(this, 0x69c, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelTextFramePtr(_result, false);
}

#pragma implementation_key(5692)
inline enum Office::MsoAutoShapeType Excel::excelShape::GetAutoShapeType ( ) {
	enum Office::MsoAutoShapeType _result;
	_com_dispatch_method(this, 0x69d, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5693)
inline void Excel::excelShape::PutAutoShapeType ( enum Office::MsoAutoShapeType _arg1 ) {
	_com_dispatch_method(this, 0x69d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5694)
inline Excel::excelCalloutFormatPtr Excel::excelShape::GetCallout ( ) {
	struct excelCalloutFormat * _result = 0;
	_com_dispatch_method(this, 0x69e, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelCalloutFormatPtr(_result, false);
}

#pragma implementation_key(5695)
inline long Excel::excelShape::GetConnectionSiteCount ( ) {
	long _result = 0;
	_com_dispatch_method(this, 0x69f, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5696)
inline enum Office::MsoTriState Excel::excelShape::GetConnector ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x6a0, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5697)
inline Excel::excelConnectorFormatPtr Excel::excelShape::GetConnectorFormat ( ) {
	struct excelConnectorFormat * _result = 0;
	_com_dispatch_method(this, 0x6a1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelConnectorFormatPtr(_result, false);
}

#pragma implementation_key(5698)
inline Excel::excelFillFormatPtr Excel::excelShape::GetFill ( ) {
	struct excelFillFormat * _result = 0;
	_com_dispatch_method(this, 0x67f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelFillFormatPtr(_result, false);
}

#pragma implementation_key(5699)
inline Excel::excelGroupShapesPtr Excel::excelShape::GetGroupItems ( ) {
	struct excelGroupShapes * _result = 0;
	_com_dispatch_method(this, 0x6a2, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelGroupShapesPtr(_result, false);
}

#pragma implementation_key(5700)
inline float Excel::excelShape::GetHeight ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x7b, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5701)
inline void Excel::excelShape::PutHeight ( float _arg1 ) {
	_com_dispatch_method(this, 0x7b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5702)
inline enum Office::MsoTriState Excel::excelShape::GetHorizontalFlip ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x6a3, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5703)
inline float Excel::excelShape::GetLeft ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x7f, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5704)
inline void Excel::excelShape::PutLeft ( float _arg1 ) {
	_com_dispatch_method(this, 0x7f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5705)
inline Excel::excelLineFormatPtr Excel::excelShape::GetLine ( ) {
	struct excelLineFormat * _result = 0;
	_com_dispatch_method(this, 0x331, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelLineFormatPtr(_result, false);
}

#pragma implementation_key(5706)
inline enum Office::MsoTriState Excel::excelShape::GetLockAspectRatio ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x6a4, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5707)
inline void Excel::excelShape::PutLockAspectRatio ( enum Office::MsoTriState _arg1 ) {
	_com_dispatch_method(this, 0x6a4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5708)
inline _bstr_t Excel::excelShape::GetName ( ) {
	BSTR _result = 0;
	_com_dispatch_method(this, 0x6e, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&_result, NULL);
	return _bstr_t(_result, false);
}

#pragma implementation_key(5709)
inline void Excel::excelShape::PutName ( _bstr_t _arg1 ) {
	_com_dispatch_method(this, 0x6e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0008", (BSTR)_arg1);
}

#pragma implementation_key(5710)
inline Excel::excelShapeNodesPtr Excel::excelShape::GetNodes ( ) {
	struct excelShapeNodes * _result = 0;
	_com_dispatch_method(this, 0x6a5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelShapeNodesPtr(_result, false);
}

#pragma implementation_key(5711)
inline float Excel::excelShape::GetRotation ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x3b, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5712)
inline void Excel::excelShape::PutRotation ( float _arg1 ) {
	_com_dispatch_method(this, 0x3b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5713)
inline Excel::excelPictureFormatPtr Excel::excelShape::GetPictureFormat ( ) {
	struct excelPictureFormat * _result = 0;
	_com_dispatch_method(this, 0x65f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelPictureFormatPtr(_result, false);
}

#pragma implementation_key(5714)
inline Excel::excelShadowFormatPtr Excel::excelShape::GetShadow ( ) {
	struct excelShadowFormat * _result = 0;
	_com_dispatch_method(this, 0x67, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelShadowFormatPtr(_result, false);
}

#pragma implementation_key(5715)
inline Excel::excelTextEffectFormatPtr Excel::excelShape::GetTextEffect ( ) {
	struct excelTextEffectFormat * _result = 0;
	_com_dispatch_method(this, 0x6a6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelTextEffectFormatPtr(_result, false);
}

#pragma implementation_key(5716)
inline Excel::excelThreeDFormatPtr Excel::excelShape::GetThreeD ( ) {
	struct excelThreeDFormat * _result = 0;
	_com_dispatch_method(this, 0x6a7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelThreeDFormatPtr(_result, false);
}

#pragma implementation_key(5717)
inline float Excel::excelShape::GetTop ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x7e, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5718)
inline void Excel::excelShape::PutTop ( float _arg1 ) {
	_com_dispatch_method(this, 0x7e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5719)
inline enum Office::MsoShapeType Excel::excelShape::GetType ( ) {
	enum Office::MsoShapeType _result;
	_com_dispatch_method(this, 0x6c, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5720)
inline enum Office::MsoTriState Excel::excelShape::GetVerticalFlip ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x6a8, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5721)
inline _variant_t Excel::excelShape::GetVertices ( ) {
	VARIANT _result;
	VariantInit(&_result);
	_com_dispatch_method(this, 0x26d, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&_result, NULL);
	return _variant_t(_result, false);
}

#pragma implementation_key(5722)
inline enum Office::MsoTriState Excel::excelShape::GetVisible ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x22e, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5723)
inline void Excel::excelShape::PutVisible ( enum Office::MsoTriState _arg1 ) {
	_com_dispatch_method(this, 0x22e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5724)
inline float Excel::excelShape::GetWidth ( ) {
	float _result = 0;
	_com_dispatch_method(this, 0x7a, DISPATCH_PROPERTYGET, VT_R4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5725)
inline void Excel::excelShape::PutWidth ( float _arg1 ) {
	_com_dispatch_method(this, 0x7a, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0004", _arg1);
}

#pragma implementation_key(5726)
inline long Excel::excelShape::GetZOrderPosition ( ) {
	long _result = 0;
	_com_dispatch_method(this, 0x6a9, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5727)
inline Excel::excelHyperlinkPtr Excel::excelShape::GetHyperlink ( ) {
	struct excelHyperlink * _result = 0;
	_com_dispatch_method(this, 0x6aa, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelHyperlinkPtr(_result, false);
}

#pragma implementation_key(5728)
inline enum Office::MsoBlackWhiteMode Excel::excelShape::GetBlackWhiteMode ( ) {
	enum Office::MsoBlackWhiteMode _result;
	_com_dispatch_method(this, 0x6ab, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5729)
inline void Excel::excelShape::PutBlackWhiteMode ( enum Office::MsoBlackWhiteMode _arg1 ) {
	_com_dispatch_method(this, 0x6ab, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5730)
inline IDispatchPtr Excel::excelShape::GetDrawingObject ( ) {
	IDispatch * _result = 0;
	_com_dispatch_method(this, 0x6ac, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return IDispatchPtr(_result, false);
}

#pragma implementation_key(5731)
inline _bstr_t Excel::excelShape::GetOnAction ( ) {
	BSTR _result = 0;
	_com_dispatch_method(this, 0x254, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&_result, NULL);
	return _bstr_t(_result, false);
}

#pragma implementation_key(5732)
inline void Excel::excelShape::PutOnAction ( _bstr_t _arg1 ) {
	_com_dispatch_method(this, 0x254, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0008", (BSTR)_arg1);
}

#pragma implementation_key(5733)
inline VARIANT_BOOL Excel::excelShape::GetLocked ( ) {
	VARIANT_BOOL _result = 0;
	_com_dispatch_method(this, 0x10d, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5734)
inline void Excel::excelShape::PutLocked ( VARIANT_BOOL _arg1 ) {
	_com_dispatch_method(this, 0x10d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x000b", _arg1);
}

#pragma implementation_key(5735)
inline Excel::RangePtr Excel::excelShape::GetTopLeftCell ( ) {
	struct Range * _result = 0;
	_com_dispatch_method(this, 0x26c, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return RangePtr(_result, false);
}

#pragma implementation_key(5736)
inline Excel::RangePtr Excel::excelShape::GetBottomRightCell ( ) {
	struct Range * _result = 0;
	_com_dispatch_method(this, 0x267, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return RangePtr(_result, false);
}

#pragma implementation_key(5737)
inline enum Excel::XlPlacement Excel::excelShape::GetPlacement ( ) {
	enum XlPlacement _result;
	_com_dispatch_method(this, 0x269, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5738)
inline void Excel::excelShape::PutPlacement ( enum XlPlacement _arg1 ) {
	_com_dispatch_method(this, 0x269, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
	L"\x0003", _arg1);
}

#pragma implementation_key(5739)
inline HRESULT Excel::excelShape::Copy ( ) {
	return _com_dispatch_method(this, 0x227, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5740)
inline HRESULT Excel::excelShape::Cut ( ) {
	return _com_dispatch_method(this, 0x235, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

#pragma implementation_key(5741)
inline HRESULT Excel::excelShape::CopyPicture ( const _variant_t & Appearance, const _variant_t & Format ) {
	return _com_dispatch_method(this, 0xd5, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x080c\x080c", &Appearance, &Format);
}

#pragma implementation_key(5742)
inline Excel::ControlFormatPtr Excel::excelShape::GetControlFormat ( ) {
	struct ControlFormat * _result = 0;
	_com_dispatch_method(this, 0x6ad, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return ControlFormatPtr(_result, false);
}

#pragma implementation_key(5743)
inline Excel::excelLinkFormatPtr Excel::excelShape::GetLinkFormat ( ) {
	struct excelLinkFormat * _result = 0;
	_com_dispatch_method(this, 0x6ae, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelLinkFormatPtr(_result, false);
}

#pragma implementation_key(5744)
inline Excel::excelOLEFormatPtr Excel::excelShape::GetOLEFormat ( ) {
	struct excelOLEFormat * _result = 0;
	_com_dispatch_method(this, 0x6af, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelOLEFormatPtr(_result, false);
}

#pragma implementation_key(5745)
inline enum Excel::XlFormControl Excel::excelShape::GetFormControlType ( ) {
	enum XlFormControl _result;
	_com_dispatch_method(this, 0x6b0, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5746)
inline _bstr_t Excel::excelShape::GetAlternativeText ( ) {
	BSTR _result = 0;
	_com_dispatch_method(this, 0x763, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&_result, NULL);
	return _bstr_t(_result, false);
}

#pragma implementation_key(5747)
inline void Excel::excelShape::PutAlternativeText ( _bstr_t _arg1 ) {
	_com_dispatch_method(this, 0x763, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, 
		L"\x0008", (BSTR)_arg1);
}

#pragma implementation_key(5748)
inline Office::ScriptPtr Excel::excelShape::GetScript ( ) {
	struct Office::Script * _result = 0;
	_com_dispatch_method(this, 0x764, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return Office::ScriptPtr(_result, false);
}

#pragma implementation_key(5749)
inline Excel::excelDiagramNodePtr Excel::excelShape::GetDiagramNode ( ) {
	struct excelDiagramNode * _result = 0;
	_com_dispatch_method(this, 0x875, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelDiagramNodePtr(_result, false);
}

#pragma implementation_key(5750)
inline enum Office::MsoTriState Excel::excelShape::GetHasDiagramNode ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x876, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5751)
inline Excel::excelDiagramPtr Excel::excelShape::GetDiagram ( ) {
	struct excelDiagram * _result = 0;
	_com_dispatch_method(this, 0x877, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelDiagramPtr(_result, false);
}

#pragma implementation_key(5752)
inline enum Office::MsoTriState Excel::excelShape::GetHasDiagram ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x878, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5753)
inline enum Office::MsoTriState Excel::excelShape::GetChild ( ) {
	enum Office::MsoTriState _result;
	_com_dispatch_method(this, 0x879, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5754)
inline Excel::excelShapePtr Excel::excelShape::GetParentGroup ( ) {
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x87a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5755)
inline Office::CanvasShapesPtr Excel::excelShape::GetCanvasItems ( ) {
	struct Office::CanvasShapes * _result = 0;
	_com_dispatch_method(this, 0x87b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&_result, NULL);
	return Office::CanvasShapesPtr(_result, false);
}

#pragma implementation_key(5756)
inline long Excel::excelShape::GetID ( ) {
	long _result = 0;
	_com_dispatch_method(this, 0x23a, DISPATCH_PROPERTYGET, VT_I4, (void*)&_result, NULL);
	return _result;
}

#pragma implementation_key(5757)
inline HRESULT Excel::excelShape::CanvasCropLeft ( float Increment ) {
	return _com_dispatch_method(this, 0x87c, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5758)
inline HRESULT Excel::excelShape::CanvasCropTop ( float Increment ) {
	return _com_dispatch_method(this, 0x87d, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5759)
inline HRESULT Excel::excelShape::CanvasCropRight ( float Increment ) {
	return _com_dispatch_method(this, 0x87e, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}

#pragma implementation_key(5760)
inline HRESULT Excel::excelShape::CanvasCropBottom ( float Increment ) {
	return _com_dispatch_method(this, 0x87f, DISPATCH_METHOD, VT_EMPTY, NULL, 
		L"\x0004", Increment);
}
#else
 ERROR UNSUPPORT_VERSION
#endif

//Set Excel watermark
bool CExcelLabeling::SetWatermarkText(IDispatch* pDoc,const MARKUP& theMarkupInfo)
{
	CComQIPtr<Excel::_Workbook> workbook(pDoc); 
	HRESULT hr = S_OK;
	Excel::SheetsPtr sheets ;
	hr = workbook->get_Sheets( &sheets ) ;
	if( FAILED( hr ) )
	{
		return false;
	}
	LONG icount = 0 ;
	hr = sheets->get_Count( &icount ) ;
	if( FAILED( hr )) 
	{
		return false;
	}  
	for( INT i=0; i<icount; i ++ )
	{
		CComPtr<IDispatch> wkPtr;
		hr = sheets->get_Item( _variant_t(i+1),&wkPtr) ;
		if( FAILED( hr ))
		{
			return false;
		}
		Excel::_WorksheetPtr wkSheet ;
		wkPtr->QueryInterface( Excel::IID__Worksheet ,(VOID**)&wkSheet) ;	

		if( wkSheet == NULL )
		{
			return false;
		}

		Excel::excelShapesPtr shapes   ;
		hr = wkSheet->get_excelShapes( &shapes) ;
		if( FAILED( hr )) 
		{
			return false ;
		}
		Excel::excelShapePtr shapePtr ;
		_bstr_t text ( theMarkupInfo.strInputText.c_str()) ;
		_bstr_t fontName ( theMarkupInfo.strFont.c_str()) ;
		//		IDispatch *ptrShape ;
		shapePtr = shapes->AddTextEffect(Office::msoTextEffect1,text, fontName,	
			(float)theMarkupInfo.fSize,Office::msoFalse,Office::msoFalse,243.75, 223.5);
		if(shapePtr != NULL)
		{
			_bstr_t szName(theMarkupInfo.strTagName.c_str());
			shapePtr->PutName(szName);
			shapePtr->Select(VARIANT_TRUE);
			shapePtr->PutRotation(theMarkupInfo.fLayout);

			Excel::excelFillFormatPtr pFill;
			pFill = shapePtr->GetFill();
			Excel::excelColorFormatPtr pForeColor=NULL;
			pFill->put_Visible(msoTrue);
			pFill->Solid();
			pFill->get_ForeColor (&pForeColor);
			pForeColor->put_excelRGB(theMarkupInfo.lColor);
			pFill->put_Transparency(theMarkupInfo.fSemi);
		}
		if(i == 0)
		{
			//ActiveRange(wkPtr,L"A1");
		}
	}
	return true;
}

bool CExcelLabeling::SetHeaderText(IDispatch* pDoc,const MARKUP& theInfo)
{
	/*CComQIPtr<Excel::_Workbook> workbook(pDoc); 
	HRESULT hr = S_OK;
	Excel::SheetsPtr sheets ;
	hr = workbook->get_Sheets( &sheets ) ;
	if( FAILED( hr ) )
	{
		return false;
	}
	LONG icount = 0 ;
	hr = sheets->get_Count( &icount ) ;
	if( FAILED( hr )) 
	{
		return false;
	}  
	for( INT i=0; i<icount; i ++ )
	{
		IDispatch* wkPtr;
		hr = sheets->get_Item( _variant_t(i+1),&wkPtr) ;
		if( FAILED( hr ))
		{
			continue;
		}
		Excel::_WorksheetPtr wkSheet ;
		hr = wkPtr->QueryInterface( Excel::IID__Worksheet ,(VOID**)&wkSheet) ;	
		if( FAILED(hr) )
		{
			continue;
		}

		Excel::excelPageSetupPtr pPageSetup;
		hr = wkSheet->get_excelPageSetup(&pPageSetup);
		if( FAILED(hr) )
		{
			continue;
		}

		pPageSetup->AddRef();
		if(_wcsicmp(theInfo.strJustify.c_str(),L"Left") == 0)
		{
			
			pPageSetup->Invoke()
		}

		wkPtr->Release();
	}*/

	bool bRet = false;
	CComVariant varResult;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pDoc,L"Worksheets",0);
	if(SUCCEEDED(hr))
	{
		CComPtr<IDispatch> pWorkSheets = varResult.pdispVal;
		CComVariant varTrue(TRUE);
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Count",0);
		if(SUCCEEDED(hr))
		{
			long lCount = varResult.lVal;
			for(int i=lCount;i>0;i--)
			{
				CComVariant varIndex(i);
				//DP((L"Sheet Index:%d\n",i));
				varResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Item",1,varIndex);
				if(SUCCEEDED(hr) && varResult.pdispVal != NULL)
				{
					CComPtr<IDispatch> pSheet = varResult.pdispVal;
					varResult.Clear();
					hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheet,L"PageSetup",0);
					if(FAILED(hr) || varResult.pdispVal == NULL)	continue;

					CComPtr<IDispatch> pPageSetup= varResult.pdispVal;
					varResult.Clear();

					if(_wcsicmp(theInfo.strJustify.c_str(),L"Left") == 0)
					{
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"LeftHeader",0);
						if(FAILED(hr) || varResult.bstrVal == NULL)	continue;
						std::wstring orig_text(varResult.bstrVal);
						FormatText(orig_text, theInfo);
						//orig_text = FormatHeaderFoosterText(orig_text, theInfo.strInputText, 0);
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"LeftHeader",1,CComVariant(orig_text.c_str()));
					}
					else if(_wcsicmp(theInfo.strJustify.c_str(),L"Center") == 0)
					{
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"CenterHeader",0);
						if(FAILED(hr) || varResult.bstrVal == NULL)	continue;
						std::wstring orig_text(varResult.bstrVal);
						FormatText(orig_text, theInfo);
						//orig_text = FormatHeaderFoosterText(orig_text, theInfo.strInputText, 0);
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"CenterHeader",1,CComVariant(orig_text.c_str()));
					}
					else if(_wcsicmp(theInfo.strJustify.c_str(),L"Right") == 0)
					{
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"RightHeader",0);
						if(FAILED(hr) || varResult.bstrVal == NULL)	continue;
						std::wstring orig_text(varResult.bstrVal);
						FormatText(orig_text, theInfo);
						//orig_text = FormatHeaderFoosterText(orig_text, theInfo.strInputText, 0);
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"RightHeader",1,CComVariant(orig_text.c_str()));							
					}
				}
			}
		}
	}
	return bRet;
}

bool CExcelLabeling::SetFooterText(IDispatch* pDoc,const MARKUP& theInfo)
{
	bool bRet = false;
	CComVariant varResult;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pDoc,L"Worksheets",0);
	if(SUCCEEDED(hr))
	{
		CComPtr<IDispatch> pWorkSheets = varResult.pdispVal;
		CComVariant varTrue(TRUE);
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Count",0);
		if(SUCCEEDED(hr))
		{
			long lCount = varResult.lVal;
			for(int i=lCount;i>0;i--)
			{
				CComVariant varIndex(i);
				//DP((L"Sheet Index:%d\n",i));
				varResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Item",1,varIndex);
				if(SUCCEEDED(hr) && varResult.pdispVal != NULL)
				{
					CComPtr<IDispatch> pSheet = varResult.pdispVal;
					varResult.Clear();
					hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheet,L"PageSetup",0);
					if(FAILED(hr) || varResult.pdispVal == NULL)	continue;

					CComPtr<IDispatch> pPageSetup= varResult.pdispVal;
					varResult.Clear();

					if(_wcsicmp(theInfo.strJustify.c_str(),L"Left") == 0)
					{
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"LeftFooter",0);
						if(FAILED(hr) || varResult.bstrVal == NULL)	continue;
						std::wstring orig_text(varResult.bstrVal);
						FormatText(orig_text, theInfo);
						//orig_text = FormatHeaderFoosterText(orig_text, theInfo.strInputText, 1);
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"LeftFooter",1,CComVariant(orig_text.c_str()));
					}
					else if(_wcsicmp(theInfo.strJustify.c_str(),L"Center") == 0)
					{
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"CenterFooter",0);
						if(FAILED(hr) || varResult.bstrVal == NULL)	continue;
						std::wstring orig_text(varResult.bstrVal);
						FormatText(orig_text, theInfo);
						//orig_text = FormatHeaderFoosterText(orig_text, theInfo.strInputText, 1);
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"CenterFooter",1,CComVariant(orig_text.c_str()));
					}
					else if(_wcsicmp(theInfo.strJustify.c_str(),L"Right") == 0)
					{
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"RightFooter",0);
						if(FAILED(hr) || varResult.bstrVal == NULL)	continue;
						std::wstring orig_text(varResult.bstrVal);
						FormatText(orig_text, theInfo);
						//orig_text = FormatHeaderFoosterText(orig_text, theInfo.strInputText, 1);
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"RightFooter",1,CComVariant(orig_text.c_str()));							
					}

				}
			}
		}
	}
	return bRet;
}

//Remove Excel watermark
bool CExcelLabeling::RemoveWatermarkText(IDispatch* pDoc,const wchar_t* strTagName)
{
	CComQIPtr<Excel::_Workbook> workbook(pDoc); 
	HRESULT hr = S_OK;
	Excel::SheetsPtr sheets ;
	hr = workbook->get_Worksheets(   &sheets ) ;
	if( FAILED( hr ) )
	{
		return false ;
	}

	LONG icount = 0 ;
	hr = sheets->get_Count( &icount ) ;
	if( FAILED( hr )) 
	{
		return false ;
	}  
	for( INT i=0; i<icount; i ++ )
	{
		CComPtr<IDispatch> wkPtr = NULL;
		hr = sheets->get_Item( _variant_t(i+1),&wkPtr) ;
		if( FAILED( hr ))
		{
			return false;
		}
		Excel::_WorksheetPtr wkSheet ;
		wkPtr->QueryInterface( Excel::IID__Worksheet ,(VOID**)&wkSheet) ;
		if( wkSheet == NULL )
		{
			return false ;
		}
		Excel::excelShapesPtr shapes   ;
		hr = wkSheet->get_excelShapes( &shapes) ;
		if( FAILED( hr )) 
		{
			return false ;
		}
		long lShapeCount = shapes->GetCount();
		for(int j=0;j<lShapeCount;j++)
		{
			CComVariant theIndex(j+1);
			Excel::excelShapePtr theShape = shapes->Item(&theIndex);
			if(theShape != NULL)
			{
				_bstr_t theName = theShape->GetName();
				if(_wcsicmp(theName,strTagName) == 0)
				{
					theShape->Select(VARIANT_TRUE);
					theShape->Delete();
				}
			}
		}
	}
	return true;
}

void CExcelLabeling::FormatText(std::wstring& orig_text, MARKUP theInfo)
{
	std::wstring theNewText(theInfo.strInputText);
	theNewText.insert(0, L"[");
	theNewText.insert(theNewText.size(), L"]");

	long lColor = theInfo.lColor;
	char theChar[30];
	_ultoa_s(lColor, theChar, 16);
	std::wstring theColorStr = boost::lexical_cast<std::wstring>(theChar);
	size_t itemp = theColorStr.size();
	while (itemp < 6)
	{
		theColorStr.insert(0, L"0");
		itemp++;
	}
	theColorStr.insert(0,L"&K");
	theNewText.insert(0, theColorStr);

	std::wstring theSizeStr = boost::lexical_cast<std::wstring>(theInfo.fSize);
	theSizeStr.insert(0, L"&");
	theNewText.insert(0, theSizeStr);

	if(_wcsicmp(theInfo.strJustify.c_str(),L"Left") == 0)
	{
		theNewText.insert(0, L"&L");
	}
	else if(_wcsicmp(theInfo.strJustify.c_str(),L"Center") == 0)
	{
		theNewText.insert(0, L"&C");
	}
	else if(_wcsicmp(theInfo.strJustify.c_str(),L"Right") == 0)
	{
		theNewText.insert(0, L"&R");
	}

	size_t nPos1 = orig_text.find(L"[");
	size_t nPos2 = orig_text.rfind(L"]");
	if( (nPos1 != wstring::npos) && (nPos2 != wstring::npos) && (nPos1 < nPos2) )
	{
		orig_text.replace(nPos1, nPos2-nPos1+1, theNewText);
	}
	else
		orig_text = theNewText;
}