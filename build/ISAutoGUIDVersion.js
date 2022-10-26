
//InstallShield 2014
//ISWiAuto21.dll
//
//Update PRODUCT CODE GUID and Product version 
//

var Args;
var ISFileName;
var ISWiProject;
var newGUID;
var newVersion;

Args = WScript.Arguments;
var count=Args.Count();
if  (count!=2)
{
	WScript.Echo('Usage: cscript ISAutoGUIDVersion.js  [Project filename] [Product version]'  );
	WScript.Quit(1);
}
ISFileName = Args.Item(0);
newVersion = Args.Item(1);
	
try
{
	try
	{
		ISWiProject  = WScript.CreateObject("ISWiAuto21.ISWiProject");
	}
	catch (e )
	{
		WScript.Echo("Create ISwiProject Object failed." );
		WScript.Quit(1);
	}
			
	ISWiProject.OpenProject(ISFileName);
	newGUID = ISWiProject.GenerateGuid();	
	ISWiProject.ProductCode = newGUID;
	ISWiProject.ProductVersion = newVersion;
		
	if (ISWiProject.SaveProject() != 0)
	    throw ("Failed to save new GUID and Version in project file.");
			
	ISWiProject.CloseProject();
}
catch (e)
{
	WScript.Echo("Failed to open IS project, error: " + e);
	WScript.Quit(1);
}

WScript.Echo("Update " + ISFileName + " Success.\nProductCode:" + newGUID + " Version:" + newVersion);
