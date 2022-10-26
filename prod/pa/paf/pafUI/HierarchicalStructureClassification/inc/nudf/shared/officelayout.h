#ifndef __NUDF_SHARE_OFFICE_LAYOUT_H__ 
#define __NUDF_SHARE_OFFICE_LAYOUT_H__ 



#define OFFICE_LAYOUT_XML   \
 "<customUI xmlns=\"http://schemas.microsoft.com/office/2009/07/customui\" onLoad=\"OnLoad\" loadImage=\"LoadImage\"> \
 	<ribbon> \
 		<tabs> \
 			<tab id=\"ProtectionTab\" label=\"NextLabs\"> \
 				<group id=\"ProtectionGroup\" label=\"Rights Management\"> \
 					<button id=\"Button_Protect_Id\" visible=\"true\" size=\"large\" getLabel=\"GetProtectButtonLable\" image=\"Protect\" onAction=\"ProtectUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Classify document\"/> \
 					<button id=\"Button_CheckRights_Id\" visible=\"true\" size=\"large\" label=\"Permissions\" image=\"CheckPermission\" onAction=\"CheckPermissionUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Check premissions\"/> \
 				</group> \
 			</tab> \
 		</tabs> \
 	</ribbon> \
 </customUI>"




#define EXCEL_LAYOUT_XML_14 \
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \
 <customUI xmlns=\"http://schemas.microsoft.com/office/2009/07/customui\" onLoad=\"OnLoad\" loadImage=\"LoadImage\"> \
 	<commands>\
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSave\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSaveAs\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileOpen\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FilePrintQuick\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInfo\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabRecent\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileClose\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabNew\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPrint\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabShare\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ApplicationOptionsDialog\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"AdvancedFileProperties\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"UpgradeDocument\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSendAsAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsPdfEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsXpsEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileInternetFax\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabHome\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabFormulas\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabReview\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"SheetMoveOrCopy\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenshotInsertGallery\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenClipping\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectctInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Paste\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Cut\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Copy\"/> \
	</commands> \
 	<ribbon> \
 		<tabs> \
 			<tab id=\"ProtectionTab\" label=\"NextLabs\"> \
 				<group id=\"ProtectionGroup\" label=\"Rights Management\"> \
 					<button id=\"Button_Protect_Id\" visible=\"true\" size=\"large\" getLabel=\"GetProtectButtonLable\" image=\"Protect\" onAction=\"ProtectUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Classify document\"/> \
 					<button id=\"Button_CheckRights_Id\" visible=\"true\" size=\"large\" label=\"Permissions\" image=\"CheckPermission\" onAction=\"CheckPermissionUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Check premissions\"/> \
 				</group> \
 			</tab> \
 		</tabs> \
 	</ribbon> \
 </customUI>"



#define WORD_LAYOUT_XML_14 \
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \
 <customUI xmlns=\"http://schemas.microsoft.com/office/2009/07/customui\" onLoad=\"OnLoad\" loadImage=\"LoadImage\"> \
 	<commands>\
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSave\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSaveAs\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileOpen\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FilePrintQuick\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInfo\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabRecent\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileClose\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabNew\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPrint\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabShare\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ApplicationOptionsDialog\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"AdvancedFileProperties\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"UpgradeDocument\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSendAsAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsPdfEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsXpsEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileInternetFax\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabHome\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPageLayoutWord\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabReferences\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabMailings\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabReviewWord\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabView\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenshotInsertGallery\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenClipping\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectInsertMenu\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectctInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Paste\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Cut\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Copy\"/> \
	</commands> \
 	<ribbon> \
 		<tabs> \
 			<tab id=\"ProtectionTab\" label=\"NextLabs\"> \
 				<group id=\"ProtectionGroup\" label=\"Rights Management\"> \
 					<button id=\"Button_Protect_Id\" visible=\"true\" size=\"large\" getLabel=\"GetProtectButtonLable\" image=\"Protect\" onAction=\"ProtectUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Classify document\"/> \
 					<button id=\"Button_CheckRights_Id\" visible=\"true\" size=\"large\" label=\"Permissions\" image=\"CheckPermission\" onAction=\"CheckPermissionUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Check premissions\"/> \
 				</group> \
 			</tab> \
 		</tabs> \
 	</ribbon> \
 </customUI>"



#define POWERPNT_LAYOUT_XML_14 \
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \
 <customUI xmlns=\"http://schemas.microsoft.com/office/2009/07/customui\" onLoad=\"OnLoad\" loadImage=\"LoadImage\"> \
 	<commands>\
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSave\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSaveAs\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileOpen\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FilePrintQuick\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInfo\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabRecent\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileClose\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabNew\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPrint\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabShare\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ApplicationOptionsDialog\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"AdvancedFileProperties\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"UpgradeDocument\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSendAsAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsPdfEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsXpsEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileInternetFax\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabHome\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabDesign\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabTransitions\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabAnimations\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabSlideShow\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabReview\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabDeveloper\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabView\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenshotInsertGallery\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenClipping\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectInsertMenu\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectctInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Paste\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Cut\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Copy\"/> \
	</commands> \
 	<ribbon> \
 		<tabs> \
 			<tab id=\"ProtectionTab\" label=\"NextLabs\"> \
 				<group id=\"ProtectionGroup\" label=\"Rights Management\"> \
 					<button id=\"Button_Protect_Id\" visible=\"true\" size=\"large\" getLabel=\"GetProtectButtonLable\" image=\"Protect\" onAction=\"ProtectUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Classify document\"/> \
 					<button id=\"Button_CheckRights_Id\" visible=\"true\" size=\"large\" label=\"Permissions\" image=\"CheckPermission\" onAction=\"CheckPermissionUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Check premissions\"/> \
 				</group> \
 			</tab> \
 		</tabs> \
 	</ribbon> \
 </customUI>"



#define EXCEL_LAYOUT_XML_15 \
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \
 <customUI xmlns=\"http://schemas.microsoft.com/office/2009/07/customui\" onLoad=\"OnLoad\" loadImage=\"LoadImage\"> \
 	<commands>\
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInfo\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabOfficeStart\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabRecent\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileClose\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSave\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FilePrintQuick\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabSave\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPrint\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabShare\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPublish\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ApplicationOptionsDialog\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"AdvancedFileProperties\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"UpgradeDocument\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSendAsAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsPdfEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsXpsEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileInternetFax\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabHome\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabFormulas\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabReview\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabData\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"SheetMoveOrCopy\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenshotInsertGallery\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenClipping\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectctInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Paste\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Cut\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Copy\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"CopyAsPicture\"/> \
	</commands> \
 	<ribbon> \
 		<tabs> \
 			<tab id=\"ProtectionTab\" label=\"NextLabs\"> \
 				<group id=\"ProtectionGroup\" label=\"Rights Management\"> \
 					<button id=\"Button_Protect_Id\" visible=\"true\" size=\"large\" getLabel=\"GetProtectButtonLable\" image=\"Protect\" onAction=\"ProtectUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Classify document\"/> \
 					<button id=\"Button_CheckRights_Id\" visible=\"true\" size=\"large\" label=\"Permissions\" image=\"CheckPermission\" onAction=\"CheckPermissionUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Check premissions\"/> \
 				</group> \
 			</tab> \
 		</tabs> \
 	</ribbon> \
 </customUI>"



#define WORD_LAYOUT_XML_15 \
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \
 <customUI xmlns=\"http://schemas.microsoft.com/office/2009/07/customui\" onLoad=\"OnLoad\" loadImage=\"LoadImage\"> \
 	<commands>\
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInfo\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabOfficeStart\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabRecent\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileClose\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSave\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FilePrintQuick\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabSave\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPrint\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabShare\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPublish\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ApplicationOptionsDialog\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"AdvancedFileProperties\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"UpgradeDocument\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSendAsAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsPdfEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsXpsEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileInternetFax\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabHome\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabWordDesign\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPageLayoutWord\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabReferences\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabMailings\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabReviewWord\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenshotInsertGallery\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenClipping\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectInsertMenu\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectctInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Paste\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Cut\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Copy\"/> \
	</commands> \
 	<ribbon> \
 		<tabs> \
 			<tab id=\"ProtectionTab\" label=\"NextLabs\"> \
 				<group id=\"ProtectionGroup\" label=\"Rights Management\"> \
 					<button id=\"Button_Protect_Id\" visible=\"true\" size=\"large\" getLabel=\"GetProtectButtonLable\" image=\"Protect\" onAction=\"ProtectUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Classify document\"/> \
 					<button id=\"Button_CheckRights_Id\" visible=\"true\" size=\"large\" label=\"Permissions\" image=\"CheckPermission\" onAction=\"CheckPermissionUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Check premissions\"/> \
 				</group> \
 			</tab> \
 		</tabs> \
 	</ribbon> \
 </customUI>"



#define POWERPNT_LAYOUT_XML_15 \
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \
 <customUI xmlns=\"http://schemas.microsoft.com/office/2009/07/customui\" onLoad=\"OnLoad\" loadImage=\"LoadImage\"> \
 	<commands>\
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInfo\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabOfficeStart\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabRecent\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileClose\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSave\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FilePrintQuick\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabSave\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPrint\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabShare\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabPublish\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ApplicationOptionsDialog\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"AdvancedFileProperties\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"UpgradeDocument\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileSendAsAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsPdfEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileEmailAsXpsEmailAttachment\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"FileInternetFax\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabHome\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabDesign\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabTransitions\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabAnimations\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabSlideShow\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabReview\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabDeveloper\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"TabView\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenshotInsertGallery\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"ScreenClipping\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectInsertMenu\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"OleObjectctInsert\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Paste\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Cut\"/> \
		<command getEnabled=\"CheckMsoButtonStatus\" idMso=\"Copy\"/> \
	</commands> \
 	<ribbon> \
 		<tabs> \
 			<tab id=\"ProtectionTab\" label=\"NextLabs\"> \
 				<group id=\"ProtectionGroup\" label=\"Rights Management\"> \
 					<button id=\"Button_Protect_Id\" visible=\"true\" size=\"large\" getLabel=\"GetProtectButtonLable\" image=\"Protect\" onAction=\"ProtectUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Classify document\"/> \
 					<button id=\"Button_CheckRights_Id\" visible=\"true\" size=\"large\" label=\"Permissions\" image=\"CheckPermission\" onAction=\"CheckPermissionUI\" getEnabled=\"CheckButtonStatus\" screentip=\"Check premissions\"/> \
 				</group> \
 			</tab> \
 		</tabs> \
 	</ribbon> \
 </customUI>"







#endif