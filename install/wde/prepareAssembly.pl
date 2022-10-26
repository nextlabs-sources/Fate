#!perl
#
# DESCRIPTION
# This script prepare an installer assembly directory for building an installer.
#
# IMPORTANT:
#	1. "Script Files" folder must be in the same directory as *.ism file. Otherwise, you will get
#		an error message like this:
#			ISDEV : error -7132: An error occurred streaming ISSetup.dll support file
#			c:\nightly\current\D_SiriusR2\install\wde\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\
#			desktop-enforcer-5.5.0.0-10001-dev-20110321063014\Script Files\Setup.inx
#		And you will not see these messages at the beginning:
#			Compiling...
#			Setup.rul
#			c:\nightly\current\D_SiriusR2\install\wde\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\script files\Setup.rul(90)
#				: warning W7503: 'ProcessEnd' : function defined but never called
#			c:\nightly\current\D_SiriusR2\install\wde\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\script files\Setup.rul(90)
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
my	$maintenanceVer = 0;
my	$patchVer = 0;
my	$msiFilePrefix = "desktop-enforcer";


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
	print "  version         Version string of format major.minor.maintenance.patch (e.g., 5.5.1.0)\n";
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

	if ($versionStr !~ /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/)
	{
		print "Invalid verison string (expects format 5.5.0.1)\n";
		exit(1);
	}

	$majorVer = $1;
	$minorVer = $2;
	$maintenanceVer = $3;
	$patchVer = $4;

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

	if ($maintenanceVer < 0 || $maintenanceVer > 100)
	{
		print "Invalid maintenance verison # (expects 1-100)\n";
		exit(1);
	}

	if ($patchVer < 0 || $patchVer > 1000)
	{
		print "Invalid patch verison # (expects 1-1000)\n";
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
print "  Maintenance         = $maintenanceVer\n";
print "  Patch               = $patchVer\n";
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

my	$installDir = "$buildRootDir/install/wde";
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
		["basepep32.dll",					$versionStr, "x86", $releaseOnly],
		["BasePEPPlugin32.dll",				$versionStr, "x86", $releaseOnly],
		["diagnostic32.dll",				$versionStr, "x86", $releaseOnly],
		["edpmanager.exe",					$versionStr, "x86", $releaseOnly],
		["edpmdlg32.dll",					$versionStr, "x86", $releaseOnly],
		["edpmgrutility32.dll",				$versionStr, "x86", $releaseOnly],
		["enhancement32.dll",				$versionStr, "x86", $releaseOnly],
		["iePEP32.dll",						$versionStr, "x86", $releaseOnly],
		["ipcproxy32.dll",					$versionStr, "x86", $releaseOnly],
		["ipcstub32.dll",					$versionStr, "x86", $releaseOnly],
		["nlQuench.exe",					$versionStr, "x86", $releaseOnly],
		["AdobePEPTrm32.dll",				$versionStr, "x86", $releaseOnly],
		["NLVisualLabelingPA200332.dll",	$versionStr, "x86", $releaseOnly],
		["NLVisualLabelingPA200732.dll",	$versionStr, "x86", $releaseOnly],
		["NLOfficePEP32.dll",	$versionStr, "x86", $releaseOnly],
		["NLVLViewPrint32.dll",				$versionStr, "x86", $releaseOnly],
		["NlRegisterPlugins32.exe",			$versionStr, "x86", $releaseOnly],	# Need this only on 32-bit installer
		["nlsce.exe",						$versionStr, "x86", $releaseOnly],	# Need this only on 32-bit installer
		["nlscekeeper32.dll",				$versionStr, "x86", $releaseOnly],
		["notification32.dll",				$versionStr, "x86", $releaseOnly],
		["OutlookAddin32.dll",				$versionStr, "x86", $releaseOnly],
		["PCStatus32.dll",					$versionStr, "x86", $releaseOnly],
		["PluginInstallerSDK32.dll",		$versionStr, "x86", $releaseOnly],
		["WdeAddTags.exe",					$versionStr, "x86", $releaseOnly],	# Need this only on 32-bit installer
		["cbPEP32.dll",                     $versionStr, "x86", $releaseOnly]
	);
my	@bin64FileList = (
		["basepep.dll",						$versionStr, "x64", $releaseOnly],
		["BasePEPPlugin.dll",				$versionStr, "x64", $releaseOnly],
		["diagnostic.dll",					$versionStr, "x64", $releaseOnly],
		["edpmanager.exe",					$versionStr, "x64", $releaseOnly],
		["edpmdlg.dll",						$versionStr, "x64", $releaseOnly],
		["edpmgrutility.dll",				$versionStr, "x64", $releaseOnly],
		["enhancement.dll",					$versionStr, "x64", $releaseOnly],
		["iePEP.dll",						$versionStr, "x64", $releaseOnly],
		["ipcproxy.dll",					$versionStr, "x64", $releaseOnly],
		["ipcstub.dll",						$versionStr, "x64", $releaseOnly],
		["NlRegisterPlugins.exe",			$versionStr, "x64", $releaseOnly],	# Need this only on 64-bit installer
		["nlsce.exe",						$versionStr, "x64", $releaseOnly],	# Need this only on 64-bit installer
		["nlscekeeper.dll",					$versionStr, "x64", $releaseOnly],
		["NLVisualLabelingPA2003.dll",		$versionStr, "x64", $releaseOnly],
		["NLVisualLabelingPA2007.dll",		$versionStr, "x64", $releaseOnly],
		["NLOfficePEP.dll",		$versionStr, "x64", $releaseOnly],
		["NLVLViewPrint.dll",				$versionStr, "x64", $releaseOnly],
		["notification.dll",				$versionStr, "x64", $releaseOnly],
		["OutlookAddin.dll",				$versionStr, "x64", $releaseOnly],
		["PCStatus.dll",					$versionStr, "x64", $releaseOnly],
		["WdeAddTags.exe",					$versionStr, "x64", $releaseOnly],	# Need this only on 64-bit installer
		["cbPEP.dll",                       $versionStr, "x64", $releaseOnly]
);

InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin32Dir, $destBin32Dir, \@bin32FileList);
InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin64Dir, $destBin64Dir, \@bin64FileList);

InstallHelper::copyRequired("$srcBin32Dir/procdetect.sys", "$destBin32Dir/procdetect.sys");
InstallHelper::copyRequired("$srcBin64Dir/procdetect.sys", "$destBin64Dir/procdetect.sys");

InstallHelper::copyVersionArchitectureAndReleaseRequired("$srcBin32Dir/NLReaderPEP32.dll", "$destBin32Dir/NLReaderPEP32.api", $versionStr, "x86", $releaseOnly);
InstallHelper::copyVersionArchitectureAndReleaseRequired("$srcBin32Dir/NLAcrobatPEP32.dll", "$destBin32Dir/NLAcrobatPEP32.api", $versionStr, "x86", $releaseOnly);


#
# Copy external libraries
#

print "INFO: Copying external libraries\n";

my $xlibX86Dir = "$buildRootDir/xlib/release_win_x86";
my $xlibX64Dir = "$buildRootDir/xlib/release_win_x64";

# 32-bit binaries
InstallHelper::copyVersionRequired("$xlibX86Dir/atl90.dll",		"$destBin32Dir/atl90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/msvcm90.dll",		"$destBin32Dir/msvcm90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/msvcp90.dll",		"$destBin32Dir/msvcp90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/msvcr90.dll",		"$destBin32Dir/msvcr90.dll",	"9.0.21022.8");

InstallHelper::copyRequired("$xlibX86Dir/Microsoft.VC90.ATL.manifest",		"$destBin32Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$xlibX86Dir/Microsoft.VC90.CRT.manifest",		"$destBin32Dir/Microsoft.VC90.CRT.manifest");

# 64-bit binaries
InstallHelper::copyVersionRequired("$xlibX64Dir/atl90.dll",		"$destBin64Dir/atl90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/msvcm90.dll",		"$destBin64Dir/msvcm90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/msvcp90.dll",		"$destBin64Dir/msvcp90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/msvcr90.dll",		"$destBin64Dir/msvcr90.dll",	"9.0.21022.8");

InstallHelper::copyRequired("$xlibX64Dir/Microsoft.VC90.ATL.manifest",		"$destBin64Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$xlibX64Dir/Microsoft.VC90.CRT.manifest",		"$destBin64Dir/Microsoft.VC90.CRT.manifest");


# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
my	$toLib32Dir = "$assemblyDir/main/dependencies/main/lib/release_win_x86";
my	$toLib64Dir = "$assemblyDir/main/dependencies/main/lib/release_win_x64";
my	$toRes32Dir = "$assemblyDir/main/dependencies/main/resource/release_win_x86";
my	$toRes64Dir = "$assemblyDir/main/dependencies/main/resource/release_win_x64";

InstallHelper::copyRequired("$destBin32Dir/atl90.dll",						"$toLib32Dir/atl90.dll");
InstallHelper::copyRequired("$destBin32Dir/msvcm90.dll",					"$toLib32Dir/msvcm90.dll");
InstallHelper::copyRequired("$destBin32Dir/msvcp90.dll",					"$toLib32Dir/msvcp90.dll");
InstallHelper::copyRequired("$destBin32Dir/msvcr90.dll",					"$toLib32Dir/msvcr90.dll");

InstallHelper::copyRequired("$destBin32Dir/Microsoft.VC90.ATL.manifest",	"$toRes32Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$destBin32Dir/Microsoft.VC90.CRT.manifest",	"$toRes32Dir/Microsoft.VC90.CRT.manifest");

InstallHelper::copyRequired("$destBin64Dir/atl90.dll",						"$toLib64Dir/atl90.dll");
InstallHelper::copyRequired("$destBin64Dir/msvcm90.dll",					"$toLib64Dir/msvcm90.dll");
InstallHelper::copyRequired("$destBin64Dir/msvcp90.dll",					"$toLib64Dir/msvcp90.dll");
InstallHelper::copyRequired("$destBin64Dir/msvcr90.dll",					"$toLib64Dir/msvcr90.dll");

InstallHelper::copyRequired("$destBin64Dir/Microsoft.VC90.ATL.manifest",	"$toRes64Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$destBin64Dir/Microsoft.VC90.CRT.manifest",	"$toRes64Dir/Microsoft.VC90.CRT.manifest");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****


#
#  Prepare artifacts
#
# Notes: Only one copy of these files are needed on a 32-bit Windows or a 64-bit Windows.
# We will rename the *32.dll to *.dll when we generate an installer.
#		dialog.ini				[EDP Control Panel]
#		menu_l.ini				[EDP Control Panel]
#		menu_r.ini				[EDP Control Panel]
#		notify.ini				[EDP Control Panel]
#		status_plugin.ini		[EDP Control Panel]
#		wde_base_plugin.cfg
#		nl_screencap_plugin.cfg	[Screen caputure]

# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup desired structure
my	$fromDir = "$installDir/src/resource";
my	$toDir = "$installDir/artifacts";

InstallHelper::copyRequired("$fromDir/README.txt",						"$toDir/README.txt");
InstallHelper::copyRequired("$fromDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$toDir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$fromDir/ce-32.ico",						"$toDir/ce-32.ico");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****

print "INFO: Copying artifacts\n";

my	$wdeConfigDir = "$buildRootDir/prod/pep/endpoint/wde/configuration";
my	$wdeScreenCapConfigDir = "$buildRootDir/prod/pep/endpoint/wde/ScreenCapture/configuration";
my	$iepepImportDir = "$buildRootDir/prod/pep/endpoint/wde/iePEP/import";
my	$adobePEPConfigDir = "$buildRootDir/prod/pep/endpoint/adobepep/configuration";
my  $cbPEPConfigDir = "$buildRootDir/prod/pep/endpoint/wde/configuration";
my	$ScreenCaptureSCEDir = "$buildRootDir/prod/pep/endpoint/wde/ScreenCapture/SCE/";
my	$srcArtifactDir = "$installDir/artifacts";
my	$destArtifactDir = "$assemblyDir/artifacts";
my	$destArtifact32Dir = "$assemblyDir/artifacts32";
my	$destArtifact64Dir = "$assemblyDir/artifacts64";
my  $procdetectDir = "$buildRootDir/prod/pep/endpoint/wde/procdetect";

# Common
InstallHelper::copyRequired("$wdeConfigDir/license.cfg",									"$destArtifactDir/license.cfg");
InstallHelper::copyRequired("$wdeConfigDir/TamperResistance.cfg",							"$destArtifactDir/WindowsDesktopEnforcer_TamperResistance.cfg");

InstallHelper::copyRequired("$iepepImportDir/ce_deny.gif",									"$destArtifactDir/ce_deny.gif");
InstallHelper::copyRequired("$iepepImportDir/ce_deny.html",									"$destArtifactDir/ce_deny.html");

InstallHelper::copyRequired("$srcArtifactDir/README.txt",									"$destArtifactDir/README.txt");
InstallHelper::copyRequired("$srcArtifactDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$destArtifactDir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$srcArtifactDir/ce-32.ico",									"$destArtifactDir/ce-32.ico");
InstallHelper::copyRequired("$procdetectDir/procdetect.inf",								"$destArtifactDir/procdetect.inf");
InstallHelper::copyRequired("$ScreenCaptureSCEDir/SCEBackGround.bmp",						"$destArtifactDir/SCEBackGround.bmp");

# 32-bit
InstallHelper::copyRequired("$wdeConfigDir/dialog32.ini",				"$destArtifact32Dir/dialog.ini");
InstallHelper::copyRequired("$wdeConfigDir/menu_l32.ini",				"$destArtifact32Dir/menu_l.ini");
InstallHelper::copyRequired("$wdeConfigDir/menu_r32.ini",				"$destArtifact32Dir/menu_r.ini");
InstallHelper::copyRequired("$wdeConfigDir/notify32.ini",				"$destArtifact32Dir/notify.ini");
InstallHelper::copyRequired("$wdeConfigDir/status_plugin32.ini",		"$destArtifact32Dir/status_plugin.ini");
InstallHelper::copyRequired("$wdeConfigDir/wde_base_plugin32.cfg",		"$destArtifact32Dir/wde_base_plugin.cfg");
InstallHelper::copyRequired("$wdeConfigDir/injection.ini",		"$destArtifact32Dir/injection.ini");

InstallHelper::copyRequired("$wdeScreenCapConfigDir/nl_screencap_plugin32.cfg",		"$destArtifact32Dir/nl_screencap_plugin.cfg");

InstallHelper::copyRequired("$adobePEPConfigDir/acrobat.exe.ini",		"$destArtifactDir/acrobat.exe.ini");
InstallHelper::copyRequired("$adobePEPConfigDir/acroRd32.exe.ini",		"$destArtifactDir/acroRd32.exe.ini");

InstallHelper::copyRequired("$cbPEPConfigDir/CommonBrowserEnforcer_TamperResistance.cfg", "$destArtifactDir/CommonBrowserEnforcer_TamperResistance.cfg");

# 64-bit
InstallHelper::copyRequired("$wdeConfigDir/dialog.ini",					"$destArtifact64Dir/dialog.ini");
InstallHelper::copyRequired("$wdeConfigDir/menu_l.ini",					"$destArtifact64Dir/menu_l.ini");
InstallHelper::copyRequired("$wdeConfigDir/menu_r.ini",					"$destArtifact64Dir/menu_r.ini");
InstallHelper::copyRequired("$wdeConfigDir/notify.ini",					"$destArtifact64Dir/notify.ini");
InstallHelper::copyRequired("$wdeConfigDir/status_plugin.ini",			"$destArtifact64Dir/status_plugin.ini");
InstallHelper::copyRequired("$wdeConfigDir/wde_base_plugin.cfg",		"$destArtifact64Dir/wde_base_plugin.cfg");
InstallHelper::copyRequired("$wdeConfigDir/injection.ini",		"$destArtifact64Dir/injection.ini");

InstallHelper::copyRequired("$wdeScreenCapConfigDir/nl_screencap_plugin.cfg",		"$destArtifact64Dir/nl_screencap_plugin.cfg");

# Update readme file
InstallHelper::updateReadMeFile("$destArtifactDir/README.txt", $majorVer, $minorVer, $buildNum);


# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup old structure
$toRes32Dir = "$assemblyDir/main/resource";
$toRes64Dir = "$assemblyDir/main/resource64";

InstallHelper::copyRequired("$destArtifactDir/license.cfg",									"$toRes32Dir/license.cfg");
InstallHelper::copyRequired("$destArtifactDir/WindowsDesktopEnforcer_TamperResistance.cfg",	"$toRes32Dir/WindowsDesktopEnforcer_TamperResistance.cfg");
InstallHelper::copyRequired("$destArtifactDir/ce_deny.gif",									"$toRes32Dir/ce_deny.gif");
InstallHelper::copyRequired("$destArtifactDir/ce_deny.html",								"$toRes32Dir/ce_deny.html");
InstallHelper::copyRequired("$destArtifactDir/README.txt",									"$toRes32Dir/README.txt");
InstallHelper::copyRequired("$destArtifactDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$toRes32Dir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$destArtifactDir/ce-32.ico",									"$toRes32Dir/ce-32.ico");
InstallHelper::copyRequired("$destArtifactDir/CommonBrowserEnforcer_TamperResistance.cfg",	"$toRes32Dir/CommonBrowserEnforcer_TamperResistance.cfg");
InstallHelper::copyRequired("$destArtifactDir/procdetect.inf",								"$toRes32Dir/procdetect.inf");

InstallHelper::copyRequired("$destArtifact32Dir/dialog.ini",			"$toRes32Dir/dialog.ini");
InstallHelper::copyRequired("$destArtifact32Dir/menu_l.ini",			"$toRes32Dir/menu_l.ini");
InstallHelper::copyRequired("$destArtifact32Dir/menu_r.ini",			"$toRes32Dir/menu_r.ini");
InstallHelper::copyRequired("$destArtifact32Dir/notify.ini",			"$toRes32Dir/notify.ini");
InstallHelper::copyRequired("$destArtifact32Dir/status_plugin.ini",		"$toRes32Dir/status_plugin.ini");

InstallHelper::copyRequired("$destArtifactDir/acrobat.exe.ini",		"$toRes32Dir/acrobat.exe.ini");
InstallHelper::copyRequired("$destArtifactDir/acrord32.exe.ini",	"$toRes32Dir/acrord32.exe.ini");

InstallHelper::copyRequired("$destArtifactDir/CommonBrowserEnforcer_TamperResistance.cfg",	"$toRes32Dir/CommonBrowserEnforcer_TamperResistance.cfg");

InstallHelper::copyRequired("$destArtifact32Dir/wde_base_plugin.cfg",		"$toRes32Dir/wde_base_plugin.cfg");
InstallHelper::copyRequired("$destArtifact32Dir/nl_screencap_plugin.cfg",	"$toRes32Dir/nl_screencap_plugin.cfg");

InstallHelper::copyRequired("$destArtifact64Dir/dialog.ini",				"$toRes64Dir/dialog.ini");
InstallHelper::copyRequired("$destArtifact64Dir/menu_l.ini",				"$toRes64Dir/menu_l.ini");
InstallHelper::copyRequired("$destArtifact64Dir/menu_r.ini",				"$toRes64Dir/menu_r.ini");
InstallHelper::copyRequired("$destArtifact64Dir/notify.ini",				"$toRes64Dir/notify.ini");
InstallHelper::copyRequired("$destArtifact64Dir/status_plugin.ini",			"$toRes64Dir/status_plugin.ini");
InstallHelper::copyRequired("$destArtifact64Dir/wde_base_plugin.cfg",		"$toRes64Dir/wde_base_plugin.cfg");
InstallHelper::copyRequired("$destArtifact64Dir/nl_screencap_plugin.cfg",	"$toRes64Dir/nl_screencap_plugin.cfg");
InstallHelper::copyRequired("$adobePEPConfigDir/64BitMAPIBroker.exe.ini",	"$toRes64Dir/64BitMAPIBroker.exe.ini");

InstallHelper::copyRequired("$destBin32Dir/procdetect.sys",		"$toRes32Dir/procdetect.sys");
InstallHelper::copyRequired("$destBin64Dir/procdetect.sys",		"$toRes64Dir/procdetect.sys");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****


#
# Prepare packages
#

my	$package32Dir = "$installDir/build/package32";
my	$package64Dir = "$installDir/build/package64";

InstallHelper::copyRequired("$wdeConfigDir/product.xml",				"$package32Dir/product.xml");

InstallHelper::copyRequired("$wdeConfigDir/product.xml",				"$package64Dir/product.xml");


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
if (system("cp -fR \"$fromScriptDir\" \"$toScriptDir\""))
{
	die "### ERROR: Failed to copy resource from $fromScriptDir to $toScriptDir.\n";
}
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****

print "INFO: Copying InstallShield files\n";

my	$srcScriptDir = "$installDir/Script Files";
my	$destScriptDir = "$assemblyDir/Script Files";

#InstallHelper::copyRequired("$installDir/$ismTemplateFileName",			"$assemblyDir/$ismTemplateFileName");
InstallHelper::copyRequired("$installDir/CommonInstallScript.obl",		"$assemblyDir/CommonInstallScript.obl");
InstallHelper::copyRequired("$installDir/PluginInstallerSDK.obl",		"$assemblyDir/PluginInstallerSDK.obl");

#dircopy($srcScriptDir, $destScriptDir) || die "### ERROR: Failed to copy resource from $srcScriptDir to $destScriptDir.\n";
if (system("cp -fR \"$srcScriptDir\" \"$destScriptDir\""))
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

InstallHelper::copyRequired("$destBin32Dir/PluginInstallerSDK32.dll",	"$toLibDir/PluginInstallerSDK.dll");
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

#
# Create prepareAssembly.inc for scripts/Makefile.install
#

print "INFO: Create prepareAssembly.inc\n";

open FILE, ">prepareAssembly.inc" || die "Error opening build installer include file prepareAssembly.inc\n";

print FILE <<"EOT";
ISM_FILE_BASENAME=$ismFileBaseName
MSI_FILE_NAME32=$msiFileName32
MSI_FILE_NAME64=$msiFileName64
EOT


close FILE;
