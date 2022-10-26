#pragma  once

#define OBLIGATION_VIEW_OVERLAY	L"VIEW_OVERLAY"
#define OBLIGATION_PRINT_OVERLAY L"PRINT_OVERLAY"
#define OBLIGATION_DOC_LABEL	L"DOCUMENT_LABEL"
#define OBLIGATION_EMAIL_LABEL	L"EMAIL_LABEL"

namespace NM_VLObligation
{
	// generic
	static const wchar_t* str_text_format =L"%classification%userId%userName%gmtTime%localTime%fileName%filePath%hostName%PolicyName";
	static const wchar_t* str_text_name = L"Text(Confidential\\n%hostName, %userName)";
	static const wchar_t* str_classification_map_name = L"Classification Mapping (ITAR=ITAR;;IP=Confidential)";
	static const wchar_t* str_transparency_name = L"Transparency";
	static const wchar_t* str_font_name=L"Font Name";
	static const wchar_t* str_font_size1_name = L"Font Size (Line 1 , in pt)";	// in pt
	static const wchar_t* str_font_size2_name = L"Font Size (Line 2 , in pt)";	// in pt
	static const wchar_t* str_font_bold_name = L"Font Bold";
	static const wchar_t* str_font_color_name = L"Font Color";
	static const wchar_t* str_policy_name = L"Policy Name";

	static const wchar_t* str_date_format_name = L"Date Format";
	static const wchar_t* str_time_format_name = L"Time Format";

	// for view
	static const wchar_t* str_placement_view_print_name = L"Placement";	// repeat or center
	static const wchar_t* str_left_margin_view_name = L"Left Margin (in pixels, for repeat only)";
	static const wchar_t* str_top_margin_view_name	=L"Top Margin (in pixels, for repeat only)";
	static const wchar_t* str_hor_space_view_name = L"Horizontal Spacing (in pixels, for repeat only)";
	static const wchar_t* str_ver_space_view_name=L"Vertical Spacing (in pixels, for repeat only)";
	// for print
	static const wchar_t* str_left_margins_print_name=L"Left and Right Margins (in inches, for repeat only)";
	static const wchar_t* str_top_margins_print_name=L"Top and Bottom Margin (in inches, for repeat only)";
	static const wchar_t* str_hor_space_print_name = L"Horizontal Spacing (in inches, for repeat only)";
	static const wchar_t* str_ver_space_print_name=L"Vertical Spacing (in inches, for repeat only)";

	// for Doc VL
	static const wchar_t* str_text_type_name = L"Text Type";
	static const wchar_t* str_conflict_onheaderfooter_name = L"On Header/ Footer Conflict";
	static const wchar_t* str_conflict_onwatermark_name = L"Watermark conflict";
	static const wchar_t* str_watermark_orig_name = L"Watermark Orientation";

	// for Email
	static const wchar_t* str_text_location_name = L"Text location";

	typedef struct _VisualLabelingInfo
	{
	public:
		wchar_t strText[512];		// visual labeling E.g: %userName\n%userID
		wchar_t strTextValue[1024];	//
		wchar_t strClassificationMap[512];
		DWORD	dwTransparency;
		wchar_t strFontName[64];
		DWORD   dwFontSize1;
		DWORD   dwFontSize2;
		bool	bFontBold;
		DWORD   dwFontColor;
		wchar_t strPlacement[64];
		DWORD	dwLeftMargin;		// pixels or inches
		DWORD   dwTopMargin;		// pixels or inches
		DWORD	dwHorSpace;			// pixels or inches
		DWORD	dwVerSpace;			// pixels or inches
		wchar_t strPolicyName[512];
		wchar_t strFilePath[2048];		//full path

		wchar_t strDateFormat[128];
		wchar_t strTimeFormat[128];

		// for persisted
		wchar_t strType[64];			// header/footer/watermark
		wchar_t strConfHFtooer[64];		//Replace, prepend, append, skip
		wchar_t strConfWatermark[64];	//Replace, skip
		wchar_t strWatermarkOrig[64];	//Horizontal or Diagonal

		// for email
		wchar_t strLocation[128];		//Subject;BodyBegin;BodyEnd

	}VisualLabelingInfo,IVisualLabelingInfo;
};
