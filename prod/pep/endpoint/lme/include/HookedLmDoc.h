#pragma once

class CHookedLmDoc:public CHookBase
{
    INSTANCE_DECLARE( CHookedLmDoc );
public:

    void Hook( void* pLmDoc );

public:

    typedef HRESULT (__stdcall* Func_Save )( IDocument* pDoc );
    typedef HRESULT (__stdcall* Func_Close )( IDocument* pDoc,
        /*[in]*/ VARIANT_BOOL SaveChanges );
    typedef HRESULT (__stdcall* Func_SaveAs )( IDocument* pDoc,
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum MiFILE_FORMAT FileFormat,
        /*[in]*/ enum MiCOMP_LEVEL CompLevel );
    typedef HRESULT (__stdcall* Func_get_Images )( IDocument* pDoc,
    /*[out,retval]*/ struct IImages * * pVal );
    typedef HRESULT (__stdcall* Func_get_BuiltInDocumentProperties )( IDocument* pDoc,
        /*[out,retval]*/ IDispatch * * pVal );
    typedef HRESULT (__stdcall* Func_get_CustomDocumentProperties )( IDocument* pDoc,
        /*[out,retval]*/ IDispatch * * pVal );
    typedef HRESULT (__stdcall* Func_Create )( IDocument* pDoc,
        /*[in]*/ BSTR FileOpen );
    typedef HRESULT (__stdcall* Func_get_Dirty )( IDocument* pDoc,
        /*[out,retval]*/ VARIANT_BOOL * pVal );
    typedef HRESULT (__stdcall* Func_OCR )( IDocument* pDoc,
        /*[in]*/ enum MiLANGUAGES LangId,
        /*[in]*/ VARIANT_BOOL OCROrientImage,
        /*[in]*/ VARIANT_BOOL OCRStraightenImage );
    typedef HRESULT (__stdcall* Func_PrintOut )( IDocument* pDoc,
        /*[in]*/ long From,
        /*[in]*/ long To,
        /*[in]*/ long Copies,
        /*[in]*/ BSTR PrinterName,
        /*[in]*/ BSTR PrintToFileName,
        /*[in]*/ VARIANT_BOOL PrintAnnotation,
        /*[in]*/ enum MiPRINT_FITMODES FitMode );

    static HRESULT __stdcall Save ( IDocument* pDoc );
    static HRESULT __stdcall Close ( IDocument* pDoc,
        /*[in]*/ VARIANT_BOOL SaveChanges );
    static HRESULT __stdcall SaveAs ( IDocument* pDoc,
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum MiFILE_FORMAT FileFormat,
        /*[in]*/ enum MiCOMP_LEVEL CompLevel );
    static HRESULT __stdcall get_Images ( IDocument* pDoc,
    /*[out,retval]*/ struct IImages * * pVal );
    static HRESULT __stdcall get_BuiltInDocumentProperties ( IDocument* pDoc,
        /*[out,retval]*/ IDispatch * * pVal );
    static HRESULT __stdcall get_CustomDocumentProperties ( IDocument* pDoc,
        /*[out,retval]*/ IDispatch * * pVal );
    static HRESULT __stdcall Create ( IDocument* pDoc,
        /*[in]*/ BSTR FileOpen );
    static HRESULT __stdcall get_Dirty ( IDocument* pDoc,
        /*[out,retval]*/ VARIANT_BOOL * pVal );
    static HRESULT __stdcall OCR ( IDocument* pDoc,
        /*[in]*/ enum MiLANGUAGES LangId,
        /*[in]*/ VARIANT_BOOL OCROrientImage,
        /*[in]*/ VARIANT_BOOL OCRStraightenImage );
    static HRESULT __stdcall PrintOut ( IDocument* pDoc,
        /*[in]*/ long From,
        /*[in]*/ long To,
        /*[in]*/ long Copies,
        /*[in]*/ BSTR PrinterName,
        /*[in]*/ BSTR PrintToFileName,
        /*[in]*/ VARIANT_BOOL PrintAnnotation,
        /*[in]*/ enum MiPRINT_FITMODES FitMode );    
};

class CHookedDocConverter:public CHookBase
{
    INSTANCE_DECLARE( CHookedDocConverter );
public:

    void Hook( void* pLmDocConverter );

public:

    typedef HRESULT (__stdcall* Func_RequiresMODIConversion )( IDocConverter* pDocConverter,
        /*[in]*/ BSTR bstrDocPath,
        /*[out]*/ VARIANT_BOOL * pbRequiresConversion ) ;

    typedef HRESULT (__stdcall* Func_ConvertDocToLmp )( IDocConverter* pDocConverter,
        /*[in]*/ short audVersion,
        /*[in]*/ BSTR bstrDocPath,
        /*[in]*/ BSTR bstrOriginalDocExtension,
        /*[out]*/ BSTR * pbstrLmpPath ) ;

    typedef HRESULT (__stdcall* Func_GetErrorCode )( IDocConverter* pDocConverter,
        /*[in,out]*/ long * pErrorCode ) ;

    typedef HRESULT (__stdcall* Func_GetMODIPrinterName )(
        /*[in,out]*/ BSTR * pbstrMODIPrinterName ) ;

    static HRESULT __stdcall Hooked_RequiresMODIConversion ( IDocConverter* pDocConverter,
        /*[in]*/ BSTR bstrDocPath,
        /*[out]*/ VARIANT_BOOL * pbRequiresConversion ) ;

    static HRESULT __stdcall Hooked_ConvertDocToLmp ( IDocConverter* pDocConverter,
        /*[in]*/ short audVersion,
        /*[in]*/ BSTR bstrDocPath,
        /*[in]*/ BSTR bstrOriginalDocExtension,
        /*[out]*/ BSTR * pbstrLmpPath ) ;

    static HRESULT __stdcall Hooked_GetErrorCode ( IDocConverter* pDocConverter,
        /*[in,out]*/ long * pErrorCode ) ;

    static HRESULT __stdcall Hooked_GetMODIPrinterName ( IDocConverter* pDocConverter,
        /*[in,out]*/ BSTR * pbstrMODIPrinterName ) ;





	   static HRESULT __stdcall my_RequiresMODIConversion ( IDocConverter* pDocConverter,
        /*[in]*/ BSTR bstrDocPath,
        /*[out]*/ VARIANT_BOOL * pbRequiresConversion ) ;

    static HRESULT __stdcall my_ConvertDocToLmp ( IDocConverter* pDocConverter,
        /*[in]*/ short audVersion,
        /*[in]*/ BSTR bstrDocPath,
        /*[in]*/ BSTR bstrOriginalDocExtension,
        /*[out]*/ BSTR * pbstrLmpPath ) ;

    static HRESULT __stdcall my_GetErrorCode ( IDocConverter* pDocConverter,
        /*[in,out]*/ long * pErrorCode ) ;

    static HRESULT __stdcall my_GetMODIPrinterName ( IDocConverter* pDocConverter,
        /*[in,out]*/ BSTR * pbstrMODIPrinterName ) ;
};