========================================================================
    STATIC LIBRARY : odhd Project Overview
========================================================================

AppWizard has created this odhd library project for you. 

This file contains a summary of what you will find in each of the files that
make up your odhd application.


odhd.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    Application Wizard.


/////////////////////////////////////////////////////////////////////////////

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named odhd.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////


================================================
Support Office 2010
================================================
Current code use "#ifdef MSO2K7" to support Office 2007.
Idealy we should create new CPP file for 2010 too.
But because of the time issue, we decide to use the code for Office 2007 to handle Office 2010.
The change is to use "#ifndef MSO2K3" instead of "#ifdef MSO2K7".
This change may cause following problems:
1. ODHD may not work properly for Office 2010 because Office 2010's object model may be different with Office 2007's.
2. It makes the code hard to maintain.
(Detail information can be got from bugzilla: http://bugs/show_bug.cgi?id=15311)

This should be changed in next release.

