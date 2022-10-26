#pragma  once
/*
*\brief: this header is used for extern using.
*/
typedef enum _FontSize
{
	Size8  = 8,
	Size10 = 10,
	Size12 = 12,
	Size16 = 16,
	Size20 = 20, 
	Size26 = 26, 
	Size36 = 36, 
	Size48 = 48, 
	Size72 = 72,
}FONTSIZE;

typedef struct _ColorPair
{
	std::wstring strColorName;
	long		 lColorValue;
}COLORPAIR;
const COLORPAIR g_szColor[] = 
{
	{L"Black", RGB(  0,   0,   0)},
	{L"White", RGB(255, 255, 255)},
	{L"Red",   RGB(255,   0,   0)},
	{L"Green", RGB(  0, 128,   0)},
	{L"Blue",  RGB(  0,   0, 255)}
};

typedef enum _Transparency
{
	emNonTran = 0,
	emSemi    = 3,
	emTran    = 4,
}TRANSPARENCY;

typedef enum _Layout
{
	emHorizontal = 0,
	emDiagonal   = 315,
}LAYOUT;

typedef struct _Markup
{
public:
	wstring strLabelingType;
	wstring strLabelingContent;
	wstring strUsername;
	wstring strTimeStamp;
	wstring strFilepath;
	wstring strTagName;
	wstring strTagValue;
	wstring strInputText;		// user inputed text
	wstring strPic;				// the image path for watermark. only work for watermark
	wstring strFont;			// The font of the content
	float   fSize;
	long	lColor;
	// left,center,right
	wstring strJustify;		// left/center/right ,only for header/footer
	float   fSemi;		    // yes or no for watermarking
	float   fLayout;		// diagonal or horizontal for watermarking
	wstring strPosition;	// only for email, prepend or append
	wstring strPurpose;		// only for print cases to popup dialog box.
	bool    bIntialize;
	_Markup():bIntialize(false)
	{

	}
}MARKUP;

typedef enum _VL_PERSISTED_RESULT
{
	emVL_NOERROR=0,
	emVL_FILETYPEERROR=1,
	emVL_UNKNOWNERROR=255
}VL_PERSISTED_RESULT;