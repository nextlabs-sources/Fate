#!perl
#
# DESCRIPTION
# This script prepare an installer assembly directory for building an installer.
#
# IMPORTANT:
#	1. "Script Files" folder must be in the same directory as *.ism file. Otherwise, you will get
#		an error message like this:
#			ISDEV : error -7132: An error occurred streaming ISSetup.dll support file
#			c:\nightly\current\D_SiriusR2\install\oe\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\
#			desktop-enforcer-5.5.0.0-10001-dev-20110321063014\Script Files\Setup.inx
#		And you will not see these messages at the beginning:
#			Compiling...
#			Setup.rul
#			c:\nightly\current\D_SiriusR2\install\oe\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\script files\Setup.rul(90)
#				: warning W7503: 'ProcessEnd' : function defined but never called
#			c:\nightly\current\D_SiriusR2\install\oe\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\script files\Setup.rul(90)
#				: warning W7503: 'ProcessRunning' : function defined but never called
#			Linking...
#			Setup.inx - 0 error(s), 2 warning(s)
#			ISDEV : warning -4371: There were warnings compiling InstallScript


BEGIN { push @INC, "../scripts"}

use strict;
use warnings;

use Getopt::Long;
use File::Copy::Recursive qw(dircopy);
use InstallHelper;


print "NextLabs Installer Assembly Preparation Script\n";


#
# Global variables
#

my	$buildType = "";
my	$buildNum = "";
my	$ismTemplateFileName = "";
my	$versionStr = "";
my	$majorVer = 0;
my	$minorVer = 0;
my	$msiFilePrefix = "outlook-enforcer";


#
# Process parameters
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: prepareAssembly.pl --buildType=<type> --buildNum=<#> --version=<string>\n";
	print "         --template=<file>\n";
	print "  buildNum        A build number. Can be any numerical or string value.\n";
	print "  buildType       Specify a build type (e.g., release, pcv, nightly or dev)\n";
	print "  template        Name of an InstallShield build script (.ism file).\n";
	print "  version         Version string of format major.minor (e.g., 5.5)\n";
	print "\nEnvironment Variables:\n";
	print "  NLBUILDROOT     Source tree root (e.g., c:/nightly/current/D_SiriusR2).\n";
}

# -----------------------------------------------------------------------------
# Parse command line arguments

sub parseCommandLine()
{
	#
	# Parse arguments
	#

	# GetOptions() key specification:
	#	option			Given as --option of not at all (value set to 0 or 1)
	#	option!			May be given as --option or --nooption (value set to 0 or 1)
	#	option=s		Mandatory string parameter: --option=somestring
	#	option:s		Optional string parameter: --option or --option=somestring
	#	option=i		Mandatory integer parameter: --option=35
	#	option:i		Optional integer parameter: --option or --option=35
	#	option=f		Mandatory floating point parameter: --option=3.14
	#	option:f		Optional floating point parameter: --option or --option=3.14

	my	$help = 0;

	if (!GetOptions(
			'buildNum=s' => \$buildNum,				# --buildNum
			'buildType=s' => \$buildType,			# --buildType
			'help' => \$help,						# --help
			'template=s' => \$ismTemplateFileName,	# --template
			'version=s' => \$versionStr				# --version
		))
	{
		exit(1);
	}

	#
	# Help
	#

	if ($help == 1)
	{
		&printHelp();
		exit;
	}

	#
	# Check for errors
	#

	if ($buildType eq '')
	{
		print "Missing build type\n";
		exit(1);
	}

	if ($buildType ne "release" && $buildType ne "pcv" && $buildType ne "nightly" && $buildType ne "dev")
	{
		print "Invalid build type $buildType (expected release, pcv, nightly or dev)\n";
		exit(1);
	}

	if ($buildNum eq '')
	{
		print "Missing build number\n";
		exit(1);
	}

	if ($ismTemplateFileName eq '')
	{
		print "Missing ISM template file name\n";
		exit(1);
	}

	if ($versionStr eq '')
	{
		print "Missing version string\n";
		exit(1);
	}

	if ($versionStr !~ /^(\d+)\.(\d+)$/)
	{
		print "Invalid verison string (expects format 5.5)\n";
		exit(1);
	}

	$majorVer = $1;
	$minorVer = $2;

	if ($majorVer < 1 || $majorVer > 100)
	{
		print "Invalid major verison # (expects 1-100)\n";
		exit(1);
	}

	if ($minorVer < 0 || $minorVer > 100)
	{
		print "Invalid minor verison # (expects 1-100)\n";
		exit(1);
	}
}

my	$argCount = scalar(@ARGV);

if ($argCount < 2 || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")
{
	printUsage;
	exit 1;
}

&parseCommandLine();

# Print parameters
print "Parameters:\n";
print "  Build Type          = $buildType\n";
print "  Build #             = $buildNum\n";
print "  Version String      = $versionStr\n";
print "  Major               = $majorVer\n";
print "  Minor               = $minorVer\n";
print "  Template File Name  = $ismTemplateFileName\n";


#
# Environment
#

my	$buildRootDir = $ENV{NLBUILDROOT};
my	$buildRootPath = $buildRootDir;

$buildRootPath =~ s/:$/:\//;

if (! defined $buildRootDir || $buildRootDir eq "")
{
	die "### ERROR: Environment variable NLBUILDROOT is missing.\n";
}


if (! -d $buildRootPath)
{
	die "### ERROR: $buildRootPath (i.e., NLBUILDROOT) does not exists.\n";
}

# Print environment
print "Environment Variables:\n";
print "  NLBUILDROOT     = $buildRootDir\n";



#
# Construct names
#

print "INFO: Construct ISM and MSI names\n";

# Construct names
my	$ismFileBaseName = "";
my	$msiFileName32 = "";
my	$msiFileName64 = "";

if ($buildType eq "release")
{
	($ismFileBaseName, $msiFileName32, $msiFileName64) = InstallHelper::makeInstallerNames_Release($msiFilePrefix, $versionStr);
}
else
{
	($ismFileBaseName, $msiFileName32, $msiFileName64) = InstallHelper::makeInstallerNames_PreRelease($msiFilePrefix, $versionStr, $buildNum, $buildType);
}

# Print names
print "Assembly Names:\n";
print "  ISM file basename      = $ismFileBaseName\n";
print "  MSI file name (32-bit) = $msiFileName32\n";
print "  MSI file name (64-bit) = $msiFileName64\n";


#
# Prepare assembly directory
#

print "INFO: Preparing assembly directory\n";

my	$installDir = "$buildRootDir/install/oe";
my	$assemblyDir = "$installDir/build/data";

if (-d $assemblyDir)
{
	InstallHelper::removeAssemblyDirectoryContent($assemblyDir);
}
else
{
	InstallHelper::createAssemblyDirectory($assemblyDir);
}


#
# Copy build binaries
#

print "INFO: Copying build binaries\n";

my	$srcBin32Dir = "$buildRootDir/bin/release_win_x86";
my	$srcBin64Dir = "$buildRootDir/bin/release_win_x64";
my	$destBin32Dir = "$assemblyDir/release_win_x86";
my	$destBin64Dir = "$assemblyDir/release_win_x64";
my	$releaseOnly = 0;

if ($buildType eq "release")
{
	$releaseOnly = 1;
}

my	@bin32FileList = (
		["adaptercomm32.dll",			$versionStr, "x86", $releaseOnly],
		["adaptermanager32.dll",		$versionStr, "x86", $releaseOnly],
		["approvaladapter32.dll",		$versionStr, "x86", $releaseOnly],
		["CEOffice32.dll",				$versionStr, "x86", $releaseOnly],
		["CE_Explorer32.dll",			$versionStr, "x86", $releaseOnly],
		["CE_AdobePEPTrm32.dll",		$versionStr, "x86", $releaseOnly],
		["diagnostic32.dll",			$versionStr, "x86", $releaseOnly],
		["edpmanager.exe",				$versionStr, "x86", $releaseOnly],
		["edpmdlg32.dll",				$versionStr, "x86", $releaseOnly],
		["edpmgrutility32.dll",			$versionStr, "x86", $releaseOnly],
		["enhancement32.dll",			$versionStr, "x86", $releaseOnly],
		["InjectExp32.dll",				$versionStr, "x86", $releaseOnly],
		["InstallOEX64.exe",			$versionStr, "x86", $releaseOnly],
		["ipcstub32.dll",				$versionStr, "x86", $releaseOnly],
		["mso2016PEP32.dll",			"", "x86", 0],
		["mso2010PEP32.dll",			"", "x86", 0],
		["mso2013PEP32.dll",			"", "x86", 0],
		["NlRegisterPlugins32.exe",		$versionStr, "x86", $releaseOnly],
		["nlQuench.exe",				$versionStr, "x86", $releaseOnly],
		["notification32.dll",			$versionStr, "x86", $releaseOnly],
		["odhd2k332.dll",				"0.0.0.0", "Win32", 0],
		["odhd2k732.dll",				"0.0.0.0", "Win32", 0],
		["odhd201032.dll",				"0.0.0.0", "Win32", 0],
		["odhd201332.dll",				"0.0.0.0", "Win32", 0],
		["OEService32.dll",				$versionStr, "x86", $releaseOnly],
		["PCStatus32.dll",				$versionStr, "x86", $releaseOnly],
		["PluginInstallerSDK32.dll",	$versionStr, "x86", $releaseOnly],
		["WindowBlocker32.dll",			$versionStr, "x86", $releaseOnly],
		["pgp_adapter32.dll",			$versionStr, "x86", $releaseOnly]
	);
my	@bin64FileList = (
		["adaptercomm.dll",				$versionStr, "x64", $releaseOnly],
		["adaptermanager.dll",			$versionStr, "x64", $releaseOnly],
		["approvaladapter.dll",			$versionStr, "x64", $releaseOnly],
		["CEOffice.dll",				$versionStr, "x64", $releaseOnly],
		["CE_Explorer.dll",				$versionStr, "x64", $releaseOnly],
		["diagnostic.dll",				$versionStr, "x64", $releaseOnly],
		["edpmanager.exe",				$versionStr, "x64", $releaseOnly],
		["edpmdlg.dll",					$versionStr, "x64", $releaseOnly],
		["edpmgrutility.dll",			$versionStr, "x64", $releaseOnly],
		["enhancement.dll",				$versionStr, "x64", $releaseOnly],
		["InjectExp.dll",				$versionStr, "x64", $releaseOnly],
		["ipcstub.dll",					$versionStr, "x64", $releaseOnly],
		["mso2010PEP.dll",				"", "x64", 0],
		["mso2016PEP.dll",				"", "x64", 0],
		["mso2013PEP.dll",				"", "x64", 0],
		["NlRegisterPlugins.exe",		$versionStr, "x64", $releaseOnly],
		["notification.dll",			$versionStr, "x64", $releaseOnly],
		["odhd2k3.dll",					$versionStr, "x64", 0],
		["odhd2k7.dll",					$versionStr, "x64", 0],
		["odhd2010.dll",				$versionStr, "x64", 0],
		["odhd2013.dll",				$versionStr, "x64", 0],
		["OEService.dll",				$versionStr, "x64", $releaseOnly],
		["PCStatus.dll",				$versionStr, "x64", $releaseOnly],
		["WindowBlocker.dll",			$versionStr, "x64", $releaseOnly]
);

InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin32Dir, $destBin32Dir, \@bin32FileList);
InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin64Dir, $destBin64Dir, \@bin64FileList);

InstallHelper::copyVersionArchitectureAndReleaseRequired("$srcBin32Dir/CE_Reader32.dll", "$destBin32Dir/CE_Reader32.api", $versionStr, "x86", $releaseOnly);
InstallHelper::copyVersionArchitectureAndReleaseRequired("$srcBin32Dir/CE_Acrobat32.dll", "$destBin32Dir/CE_Acrobat32.api", $versionStr, "x86", $releaseOnly);

#
# Copy external libraries
#
# Notes: gdiplus.dll does not seem to be used in OE at all. Put it out in OE 5.5 installer.
# It can be found in S:\build\externals\microsoft\gdiplus\5.1.3102.5581\dist\asms\10\msft\windows\gdiplus.

print "INFO: Copying external libraries\n";

my	$msoPepDir = "$buildRootDir/prod/pep/endpoint/oe/msoPEP";


#
# Copy externals from xlib directory
#

print "INFO: Copying external libraries\n";

my $xlibX86Dir = "$buildRootDir/xlib/release_win_x86";
my $xlibX64Dir = "$buildRootDir/xlib/release_win_x64";

# 32-bit binaries
InstallHelper::copyVersionRequired("$xlibX86Dir/atl90.dll",		"$destBin32Dir/atl90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/msvcm90.dll",		"$destBin32Dir/msvcm90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/msvcp90.dll",		"$destBin32Dir/msvcp90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/msvcr90.dll",		"$destBin32Dir/msvcr90.dll",	"9.0.21022.8");

InstallHelper::copyRequired("$xlibX86Dir/boost_regex-vc90-mt-1_43.dll",					"$destBin32Dir/boost_regex-vc90-mt-1_43.dll");

InstallHelper::copyRequired("$xlibX86Dir/Microsoft.VC90.ATL.manifest",		"$destBin32Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$xlibX86Dir/Microsoft.VC90.CRT.manifest",		"$destBin32Dir/Microsoft.VC90.CRT.manifest");

InstallHelper::copyRequired("$xlibX86Dir/DocumentFormat.OpenXml.dll",					"$destBin32Dir/DocumentFormat.OpenXml.dll");

# 64-bit binaries
InstallHelper::copyVersionRequired("$xlibX64Dir/atl90.dll",		"$destBin64Dir/atl90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/msvcm90.dll",		"$destBin64Dir/msvcm90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/msvcp90.dll",		"$destBin64Dir/msvcp90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/msvcr90.dll",		"$destBin64Dir/msvcr90.dll",	"9.0.21022.8");

InstallHelper::copyRequired("$xlibX64Dir/boost_regex-vc90-mt-1_43.dll",					"$destBin64Dir/boost_regex-vc90-mt-1_43.dll");

InstallHelper::copyRequired("$xlibX64Dir/Microsoft.VC90.ATL.manifest",		"$destBin64Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$xlibX64Dir/Microsoft.VC90.CRT.manifest",		"$destBin64Dir/Microsoft.VC90.CRT.manifest");

# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
my	$to32Dir = "$assemblyDir/main/dependencies/main/lib/release_win_x86";
my	$to64Dir = "$assemblyDir/main/dependencies/main/lib/release_win_x64";

InstallHelper::copyRequired("$destBin32Dir/atl90.dll",						"$to32Dir/atl90.dll");
InstallHelper::copyRequired("$destBin32Dir/msvcm90.dll",					"$to32Dir/msvcm90.dll");
InstallHelper::copyRequired("$destBin32Dir/msvcp90.dll",					"$to32Dir/msvcp90.dll");
InstallHelper::copyRequired("$destBin32Dir/msvcr90.dll",					"$to32Dir/msvcr90.dll");

InstallHelper::copyRequired("$destBin32Dir/Microsoft.VC90.ATL.manifest",	"$to32Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$destBin32Dir/Microsoft.VC90.CRT.manifest",	"$to32Dir/Microsoft.VC90.CRT.manifest");

InstallHelper::copyRequired("$destBin64Dir/atl90.dll",						"$to64Dir/atl90.dll");
InstallHelper::copyRequired("$destBin64Dir/msvcm90.dll",					"$to64Dir/msvcm90.dll");
InstallHelper::copyRequired("$destBin64Dir/msvcp90.dll",					"$to64Dir/msvcp90.dll");
InstallHelper::copyRequired("$destBin64Dir/msvcr90.dll",					"$to64Dir/msvcr90.dll");

InstallHelper::copyRequired("$destBin64Dir/Microsoft.VC90.ATL.manifest",	"$to64Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$destBin64Dir/Microsoft.VC90.CRT.manifest",	"$to64Dir/Microsoft.VC90.CRT.manifest");

# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****


#
#  Prepare support files
#
# Notes: license.cfg will not be installed by the installer. It will be updated manually.

# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup desired structure
my	$fromDir = "$installDir/src/resource";
my	$toDir = "$installDir/resource";

InstallHelper::copyRequired("$fromDir/README.txt",									"$toDir/README.txt");
InstallHelper::copyRequired("$fromDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$toDir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$fromDir/ce-32.ico",									"$toDir/ce-32.ico");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****

print "INFO: Copying support files\n";

my	$oeConfigDir = "$buildRootDir/prod/pep/endpoint/oe/configuration";
my	$wdeConfigDir = "$buildRootDir/prod/pep/endpoint/wde/configuration";
my	$oeApprovalAdapterDir = "$buildRootDir/prod/pep/endpoint/oe/msoPEP/approvaladapter";
my	$oeStripAttachmentDir = "$buildRootDir/prod/pep/endpoint/oe/msoPEP/StripAttachment";
my	$oeServiceDir = "$buildRootDir/prod/pep/endpoint/oe/msoPEP/sendto/OEService";
my	$srcResDir = "$installDir/resource";
my	$destResDir = "$assemblyDir/resource";
my	$destRes32Dir = "$assemblyDir/resource32";
my	$destRes64Dir = "$assemblyDir/resource64";

# Common
InstallHelper::copyRequired("$oeConfigDir/license.cfg",					"$destResDir/license.cfg");
InstallHelper::copyRequired("$oeConfigDir/TamperResistance.cfg",		"$destResDir/OutlookEnforcer_TamperResistance.cfg");

InstallHelper::copyRequired("$srcResDir/README.txt",									"$destResDir/README.txt");
InstallHelper::copyRequired("$srcResDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$destResDir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$srcResDir/ce-32.ico",										"$destResDir/ce-32.ico");

# 32-bit
InstallHelper::copyRequired("$wdeConfigDir/dialog32.ini",					"$destRes32Dir/dialog.ini");
InstallHelper::copyRequired("$wdeConfigDir/menu_l32.ini",					"$destRes32Dir/menu_l.ini");
InstallHelper::copyRequired("$wdeConfigDir/menu_r32.ini",					"$destRes32Dir/menu_r.ini");
InstallHelper::copyRequired("$wdeConfigDir/notify32.ini",					"$destRes32Dir/notify.ini");
InstallHelper::copyRequired("$wdeConfigDir/status_plugin32.ini",			"$destRes32Dir/status_plugin.ini");
InstallHelper::copyRequired("$oeApprovalAdapterDir/approvaladapter.ini",	"$destRes32Dir/approvaladapter.ini");
InstallHelper::copyRequired("$oeServiceDir/nl_OE_plugin32.cfg",				"$destRes32Dir/nl_OE_plugin32.cfg");
InstallHelper::copyRequired("$oeStripAttachmentDir/fsadapter/fsadapter.ini",				"$destRes32Dir/fsadapter.ini");
InstallHelper::copyRequired("$oeStripAttachmentDir/ftpadapter/ftpadapter.ini",				"$destRes32Dir/ftpadapter.ini");
InstallHelper::copyRequired("$oeConfigDir/UserAttributesClient.properties",				"$destRes32Dir/UserAttributesClient.properties");
InstallHelper::copyRequired("$oeConfigDir/UserAttributesClient.jar",				"$destRes32Dir/UserAttributesClient.jar");
InstallHelper::copyRequired("$oeConfigDir/OutlookEnforcer.ini",				"$destRes32Dir/OutlookEnforcer.ini");

# 64-bit
InstallHelper::copyRequired("$wdeConfigDir/dialog.ini",						"$destRes64Dir/dialog.ini");
InstallHelper::copyRequired("$wdeConfigDir/menu_l.ini",						"$destRes64Dir/menu_l.ini");
InstallHelper::copyRequired("$wdeConfigDir/menu_r.ini",						"$destRes64Dir/menu_r.ini");
InstallHelper::copyRequired("$wdeConfigDir/notify.ini",						"$destRes64Dir/notify.ini");
InstallHelper::copyRequired("$wdeConfigDir/status_plugin.ini",				"$destRes64Dir/status_plugin.ini");
InstallHelper::copyRequired("$oeServiceDir/nl_OE_plugin.cfg",				"$destRes64Dir/nl_OE_plugin.cfg");
InstallHelper::copyRequired("$oeStripAttachmentDir/fsadapter/fsadapter.ini",				"$destRes64Dir/fsadapter.ini");
InstallHelper::copyRequired("$oeStripAttachmentDir/ftpadapter/ftpadapter.ini",				"$destRes64Dir/ftpadapter.ini");
InstallHelper::copyRequired("$oeConfigDir/UserAttributesClient.properties",				"$destRes64Dir/UserAttributesClient.properties");
InstallHelper::copyRequired("$oeConfigDir/UserAttributesClient.jar",				"$destRes64Dir/UserAttributesClient.jar");
InstallHelper::copyRequired("$oeConfigDir/OutlookEnforcer.ini",				"$destRes64Dir/OutlookEnforcer.ini");

# Update readme file
InstallHelper::updateReadMeFile("$destResDir/README.txt", $majorVer, $minorVer, $buildNum);


# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup old structure
my	$toOldRes32Dir = "$assemblyDir/main/resource";

#InstallHelper::copyRequired("$destResDir/license.cfg",									"$toOldRes32Dir/license.cfg");
InstallHelper::copyRequired("$destResDir/OutlookEnforcer_TamperResistance.cfg",			"$toOldRes32Dir/OutlookEnforcer_TamperResistance.cfg");
InstallHelper::copyRequired("$destResDir/README.txt",									"$toOldRes32Dir/README.txt");
InstallHelper::copyRequired("$destResDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$toOldRes32Dir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$destResDir/ce-32.ico",									"$toOldRes32Dir/ce-32.ico");

InstallHelper::copyRequired("$destRes32Dir/approvaladapter.ini",		"$toOldRes32Dir/approvaladapter.ini");
InstallHelper::copyRequired("$destRes32Dir/nl_OE_plugin32.cfg",			"$toOldRes32Dir/nl_OE_plugin32.cfg");

InstallHelper::copyRequired("$destRes64Dir/nl_OE_plugin.cfg",			"$toOldRes32Dir/nl_OE_plugin.cfg");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****


#
# Prepare packages
#

my	$package32Dir = "$installDir/build/package32";
my	$package64Dir = "$installDir/build/package64";

InstallHelper::copyRequired("$oeConfigDir/product.xml",					"$package32Dir/product.xml");
InstallHelper::copyRequired("$oeConfigDir/product.xml",					"$package64Dir/product.xml");


InstallHelper::copyRequired("$oeConfigDir/client-info.xml",					"$package32Dir/client-info.xml");
InstallHelper::copyRequired("$oeConfigDir/client-info.xml",					"$package64Dir/client-info.xml");

#
# Prepare InstallShield files
#

# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup desired structure
my	$ismTemplateFile = "$installDir/src/installer/$ismTemplateFileName";
my	$fromOblDir = "$installDir/src/installer/Script Files/include";
my	$fromScriptDir = "$installDir/src/installer/Script Files";
my	$toScriptDir = "$installDir/Script Files";

InstallHelper::copyRequired("$ismTemplateFile",							"$installDir/$ismTemplateFileName");
InstallHelper::copyRequired("$fromOblDir/CommonInstallScript.obl",		"$installDir/CommonInstallScript.obl");
InstallHelper::copyRequired("$fromOblDir/PluginInstallerSDK.obl",		"$installDir/PluginInstallerSDK.obl");

#dircopy($fromScriptDir, $toScriptDir) || die "### ERROR: Failed to copy resource from $fromScriptDir to $toScriptDir.\n";
if (system("cp -fR \"$fromScriptDir\" \"$installDir\""))
{
	die "### ERROR: Failed to copy resource from $fromScriptDir to $toScriptDir.\n";
}
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****

print "INFO: Copying InstallShield files\n";

my	$srcScriptDir = "$installDir/Script Files";
my	$destScriptDir = "$assemblyDir/Script Files";

#InstallHelper::copyRequired("$installDir/$ismTemplateFileName",		"$assemblyDir/$ismTemplateFileName");
InstallHelper::copyRequired("$installDir/CommonInstallScript.obl",		"$assemblyDir/CommonInstallScript.obl");
InstallHelper::copyRequired("$installDir/PluginInstallerSDK.obl",		"$assemblyDir/PluginInstallerSDK.obl");

#dircopy($srcScriptDir, $destScriptDir) || die "### ERROR: Failed to copy resource from $srcScriptDir to $destScriptDir.\n";
if (system("cp -fR \"$srcScriptDir\" \"$assemblyDir\""))
{
	die "### ERROR: Failed to copy resource from $srcScriptDir to $destScriptDir.\n";
}

# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup old structure
my	$toOblDir = "$assemblyDir/main/installer/obl";
my	$toLibDir = "$assemblyDir/main/installer/lib";
my	$toMiscDir = "$assemblyDir/main/installer/misc_external";

InstallHelper::copyRequired("$assemblyDir/CommonInstallScript.obl",		"$toOblDir/CommonInstallScript.obl");
InstallHelper::copyRequired("$assemblyDir/PluginInstallerSDK.obl",		"$toOblDir/PluginInstallerSDK.obl");

InstallHelper::copyRequired("$destBin32Dir/PluginInstallerSDK32.dll",	"$destBin32Dir/PluginInstallerSDK.dll");
InstallHelper::copyRequired("$destBin32Dir/nlQuench.exe",				"$toMiscDir/nlQuench.exe");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****


#
# Modify installer build script
#

print "INFO: modify installer script\n";

my	$templateFile = "$installDir/$ismTemplateFileName";
#my	$newIsmFile = "$assemblyDir/${ismFileBaseName}.ism";
my	$newIsmFile = "$assemblyDir/$ismTemplateFileName";

InstallHelper::constructIsmFile($templateFile, $newIsmFile, $msiFileName32, $msiFileName64, $versionStr, $buildNum);
