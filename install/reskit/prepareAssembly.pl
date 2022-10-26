#!perl
#
# DESCRIPTION
# This script prepare a resource kit assembly directory for building a resource kit.


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
my	$versionStr = "";
my	$majorVer = 0;
my	$minorVer = 0;
my	$maintenanceVer = 0;
my	$patchVer = 0;


#
# Process parameters
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: prepareAssembly.pl --buildType=<type> --buildNum=<#> --version=<string>\n";
	print "  buildNum        A build number. Can be any numerical or string value.\n";
	print "  buildType       Specify a build type (e.g., release, pcv, nightly or dev)\n";
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
# Prepare assembly directory
#

print "INFO: Preparing assembly directory\n";

my	$installDir = "$buildRootDir/install/reskit";
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
my	$destBin32Dir = "$assemblyDir/x86";
my	$destBin64Dir = "$assemblyDir/x64";
my	$destInstall32Dir = "$assemblyDir/installer";
my	$destInstallCleanupDir = "$assemblyDir/InstallCleanup";
my	$releaseOnly = 0;

if ($buildType eq "release")
{
	$releaseOnly = 1;
}

my	@bin32FileList = (
		["celog32.dll",					$versionStr, "x86", $releaseOnly],
		["NlRegisterPlugins32.exe",		$versionStr, "x86", $releaseOnly],
		["nl_sysenc_lib32.dll",			$versionStr, "x86", $releaseOnly],
		["nltag.exe",					$versionStr, "x86", $releaseOnly],
		["PDFLibTool.exe",				$versionStr, "x86", $releaseOnly],
		["pdflib32.dll",				$versionStr, "x86", $releaseOnly],
		["tag_office2k732.dll",			$versionStr, "x86", $releaseOnly],
		["resattrlib32.dll",			$versionStr, "x86", $releaseOnly],
		["resattrmgr32.dll",			$versionStr, "x86", $releaseOnly],
		["TagFiles.exe",				$versionStr, "x86", $releaseOnly]
	);
my	@bin64FileList = (
		["celog.dll",					$versionStr, "x64", $releaseOnly],
#		["NlRegisterPlugins.exe",		$versionStr, "x64", $releaseOnly],		# No need to provide 64-bit version right now
		["nl_sysenc_lib.dll",			$versionStr, "x64", $releaseOnly],
		["nltag.exe",					$versionStr, "x64", $releaseOnly],
		["PDFLibTool.exe",				$versionStr, "x64", $releaseOnly],
		["pdflib.dll",					$versionStr, "x64", $releaseOnly],
		["tag_office2k7.dll",			$versionStr, "x64", $releaseOnly],
		["resattrlib.dll",				$versionStr, "x64", $releaseOnly],
		["resattrmgr.dll",				$versionStr, "x64", $releaseOnly],
		["TagFiles.exe",				$versionStr, "x64", $releaseOnly]
);
my	@install32FileList = (
		["EnforcerGPOPrep.exe",			$versionStr, "x86", $releaseOnly],
		["PolicyControllerGPOPrep.exe",	$versionStr, "x86", $releaseOnly]
	);

InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin32Dir, $destBin32Dir, \@bin32FileList);
InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin64Dir, $destBin64Dir, \@bin64FileList);
InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin32Dir, $destInstall32Dir, \@install32FileList);

InstallHelper::copyVersionRequired("$srcBin32Dir/NlUninstallCleanup.exe", "$destInstallCleanupDir/x86/NlUninstallCleanup.exe", $versionStr);
InstallHelper::copyVersionRequired("$srcBin64Dir/NlUninstallCleanup.exe", "$destInstallCleanupDir/x64/NlUninstallCleanup.exe", $versionStr);


#
# Copy external libraries
#
# Notes: Podofo library uses zlib and freetype. For 32-bit Podofo.dll, it dynamically links freetype6.dll
# and zlib1.dll. For 64-bit Podofo.dll, it statically links freetype and dynamically links zlibwapi.dll.

print "INFO: Copying external libraries\n";

my $xlibX86Dir = "$buildRootDir/xlib/release_win_x86";
my $xlibX64Dir = "$buildRootDir/xlib/release_win_x64";

# 32-bit binaries
InstallHelper::copyVersionRequired("$xlibX86Dir/atl90.dll",		"$destBin32Dir/atl90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/msvcm90.dll",		"$destBin32Dir/msvcm90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/msvcp90.dll",		"$destBin32Dir/msvcp90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/msvcr90.dll",		"$destBin32Dir/msvcr90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/mfc90.dll",		"$destBin32Dir/mfc90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/mfc90u.dll",		"$destBin32Dir/mfc90u.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/mfcm90.dll",		"$destBin32Dir/mfcm90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/mfcm90u.dll",		"$destBin32Dir/mfcm90u.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/vcomp90.dll",	"$destBin32Dir/vcomp90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX86Dir/freetype6.dll",				"$destBin32Dir/freetype6.dll",	"2.3.5.2742");
InstallHelper::copyRequired("$xlibX86Dir/libtiff.dll",				"$destBin32Dir/libtiff.dll");
InstallHelper::copyVersionRequired("$xlibX86Dir/PoDoFoLib.dll",	"$destBin32Dir/PoDoFoLib.dll",	"0.8.1.0");
InstallHelper::copyVersionRequired("$xlibX86Dir/zlib1.dll",							"$destBin32Dir/zlib1.dll",		"1.2.3.0");

InstallHelper::copyRequired("$xlibX86Dir/Microsoft.VC90.ATL.manifest",		"$destBin32Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$xlibX86Dir/Microsoft.VC90.CRT.manifest",		"$destBin32Dir/Microsoft.VC90.CRT.manifest");
InstallHelper::copyRequired("$xlibX86Dir/Microsoft.VC90.MFC.manifest",		"$destBin32Dir/Microsoft.VC90.MFC.manifest");
InstallHelper::copyRequired("$xlibX86Dir/Microsoft.VC90.OpenMP.manifest",	"$destBin32Dir/Microsoft.VC90.OpenMP.manifest");

# 64-bit binaries
InstallHelper::copyVersionRequired("$xlibX64Dir/atl90.dll",		"$destBin64Dir/atl90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/msvcm90.dll",		"$destBin64Dir/msvcm90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/msvcp90.dll",		"$destBin64Dir/msvcp90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/msvcr90.dll",		"$destBin64Dir/msvcr90.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/mfc90.dll",		"$destBin64Dir/mfc90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/mfc90u.dll",		"$destBin64Dir/mfc90u.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/mfcm90.dll",		"$destBin64Dir/mfcm90.dll",		"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/mfcm90u.dll",		"$destBin64Dir/mfcm90u.dll",	"9.0.21022.8");
InstallHelper::copyVersionRequired("$xlibX64Dir/vcomp90.dll",	"$destBin64Dir/vcomp90.dll",	"9.0.21022.8");
InstallHelper::copyRequired("$xlibX64Dir/libtiff.dll",		"$destBin64Dir/libtiff.dll");
InstallHelper::copyVersionRequired("$xlibX64Dir/PoDoFoLib.dll",	"$destBin64Dir/PoDoFoLib.dll",	"0.8.1.0");
InstallHelper::copyVersionRequired("$xlibX64Dir/zlibwapi.dll",			"$destBin64Dir/zlibwapi.dll",	"1.2.5.0");

InstallHelper::copyRequired("$xlibX64Dir/Microsoft.VC90.ATL.manifest",		"$destBin64Dir/Microsoft.VC90.ATL.manifest");
InstallHelper::copyRequired("$xlibX64Dir/Microsoft.VC90.CRT.manifest",		"$destBin64Dir/Microsoft.VC90.CRT.manifest");
InstallHelper::copyRequired("$xlibX64Dir/Microsoft.VC90.MFC.manifest",		"$destBin64Dir/Microsoft.VC90.MFC.manifest");
InstallHelper::copyRequired("$xlibX64Dir/Microsoft.VC90.OpenMP.manifest",	"$destBin64Dir/Microsoft.VC90.OpenMP.manifest");


#
#  Prepare support files
#

print "INFO: Copying support files\n";

my	$srcSignToolDir = "$buildRootDir/build/signtool";
my	$srcUninstallCleanupConfigDir = "$buildRootDir/install/support/NlUninstallCleanup/configuration";
my	$srcDeployToolDir = "$buildRootDir/install/deploytools";
my	$destInstallCheckDir= "$assemblyDir/InstallCheck";

# Common
InstallHelper::copyRequired("$installDir/ReadMe.txt",				"$assemblyDir/ReadMe.txt");

# Install
InstallHelper::copyRequired("$srcSignToolDir/NextLabsPub.cer",		"$destInstall32Dir/NextLabsPub.cer");

# Install cleanup
InstallHelper::copyRequired("$srcUninstallCleanupConfigDir/NlUninstallCleanup_x86.cfg",		"$destInstallCleanupDir/x86/NlUninstallCleanup_x86.cfg");
InstallHelper::copyRequired("$srcUninstallCleanupConfigDir/NlUninstallCleanup_x64.cfg",		"$destInstallCleanupDir/x64/NlUninstallCleanup_x64.cfg");
InstallHelper::copyRequired("$srcUninstallCleanupConfigDir/InstallCleanup.bat",				"$destInstallCleanupDir/InstallCleanup.bat");

# Install check
InstallHelper::copyRequired("$srcDeployToolDir/NlInstallCheck.bat",			"$destInstallCheckDir/NlInstallCheck.bat");
InstallHelper::copyRequired("$srcDeployToolDir/NlUninstallCheck.bat",		"$destInstallCheckDir/NlUninstallCheck.bat");
InstallHelper::copyRequired("$srcDeployToolDir/NlSystemCheck.bat",			"$destInstallCheckDir/NlSystemCheck.bat");
