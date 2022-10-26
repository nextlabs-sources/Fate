#!perl
#
# DESCRIPTION
# This script prepare an installer assembly directory for building an installer.
#
# IMPORTANT:
#	1. "Script Files" folder must be in the same directory as *.ism file. Otherwise, you will get
#		an error message like this:
#			ISDEV : error -7132: An error occurred streaming ISSetup.dll support file
#			c:\nightly\current\D_SiriusR2\install\ne\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\
#			desktop-enforcer-5.5.0.0-10001-dev-20110321063014\Script Files\Setup.inx
#		And you will not see these messages at the beginning:
#			Compiling...
#			Setup.rul
#			c:\nightly\current\D_SiriusR2\install\ne\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\script files\Setup.rul(90)
#				: warning W7503: 'ProcessEnd' : function defined but never called
#			c:\nightly\current\D_SiriusR2\install\ne\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\script files\Setup.rul(90)
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
my	$msiFilePrefix = "network-enforcer";


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

my	$installDir = "$buildRootDir/install/ne";
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
		["ftpe32.dll",					$versionStr, "x86", $releaseOnly],
		["httpe32.dll",					$versionStr, "x86", $releaseOnly],
		["hpe32.dll",					$versionStr, "x86", $releaseOnly],
		["InstallSPICaller32.exe",		$versionStr, "x86", $releaseOnly],
		["nlQuench.exe",				$versionStr, "x86", $releaseOnly],
		["PluginInstallerSDK32.dll",	$versionStr, "x86", $releaseOnly],
	);
my	@bin64FileList = (
		["ftpe.dll",					$versionStr, "x64", $releaseOnly],
		["httpe.dll",					$versionStr, "x64", $releaseOnly],
		["hpe.dll",						$versionStr, "x64", $releaseOnly],
		["InstallSPICaller.exe",		$versionStr, "x64", $releaseOnly]
);

InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin32Dir, $destBin32Dir, \@bin32FileList);
InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin64Dir, $destBin64Dir, \@bin64FileList);

# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
InstallHelper::copyRequired("$destBin32Dir/hpe32.dll",				"$assemblyDir/main/installer/build_native/release_lib_win32/hpe.dll");
InstallHelper::copyRequired("$destBin32Dir/ftpe32.dll",				"$assemblyDir/main/native/win32/debug/InstallerSPI.dll");
InstallHelper::copyRequired("$destBin32Dir/ftpe32.dll",				"$assemblyDir/main/native/win32/release/InstallerSPI.dll");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****


#
#  Prepare artifacts
#
# Notes: license.cfg will not be installed by the installer. It will be updated manually.

# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup desired structure
my	$fromDir = "$installDir/src/resource";
my	$toDir = "$installDir/artifacts";

InstallHelper::copyRequired("$fromDir/README.txt",						"$toDir/README.txt");
InstallHelper::copyRequired("$fromDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$toDir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$fromDir/ce-32.ico",						"$toDir/ce-32.ico");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****

print "INFO: Copying artifacts\n";

my	$neConfigDir = "$buildRootDir/prod/pep/endpoint/ne/configuration";
my	$srcArtifactDir = "$installDir/artifacts";
my	$destArtifactDir = "$assemblyDir/artifacts";
my	$destArtifact32Dir = "$assemblyDir/artifacts32";
my	$destArtifact64Dir = "$assemblyDir/artifacts64";

# Common
InstallHelper::copyRequired("$neConfigDir/TamperResistance.cfg",		"$destArtifactDir/NetworkEnforcer_TamperResistance.cfg");

InstallHelper::copyRequired("$srcArtifactDir/README.txt",				"$destArtifactDir/README.txt");
InstallHelper::copyRequired("$srcArtifactDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$destArtifactDir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$srcArtifactDir/ce-32.ico",				"$destArtifactDir/ce-32.ico");

InstallHelper::copyRequired("$neConfigDir/ftpe.ini",					"$destArtifactDir/ftpe.ini");
InstallHelper::copyRequired("$neConfigDir/httpe.ini",					"$destArtifactDir/httpe.ini");
InstallHelper::copyRequired("$neConfigDir/hpe.ini",						"$destArtifactDir/hpe.ini");

# 32-bit
InstallHelper::copyRequired("$neConfigDir/ftpte.exe.ini",				"$destArtifact32Dir/ftpte.exe.ini");

# 64-bit
InstallHelper::copyRequired("$neConfigDir/smartftp.exe.ini",			"$destArtifact64Dir/smartftp.exe.ini");

# Update readme file
InstallHelper::updateReadMeFile("$destArtifactDir/README.txt", $majorVer, $minorVer, $maintenanceVer, $patchVer, $buildNum);


# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup old structure
my	$toRes32Dir = "$assemblyDir/main/resource";

InstallHelper::copyRequired("$destArtifactDir/NetworkEnforcer_TamperResistance.cfg",	"$toRes32Dir/NetworkEnforcer_TamperResistance.cfg");
InstallHelper::copyRequired("$destArtifactDir/README.txt",				"$toRes32Dir/README.txt");
InstallHelper::copyRequired("$destArtifactDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$toRes32Dir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$destArtifactDir/ce-32.ico",				"$assemblyDir/main/image/ce-32.ico");

InstallHelper::copyRequired("$destArtifactDir/ftpe.ini",				"$toRes32Dir/configuration/ftpe.ini");
InstallHelper::copyRequired("$destArtifactDir/httpe.ini",				"$toRes32Dir/configuration/httpe.ini");
InstallHelper::copyRequired("$destArtifactDir/hpe.ini",					"$toRes32Dir/configuration/hpe.ini");

InstallHelper::copyRequired("$neConfigDir/ftpte.exe.ini",				"$toRes32Dir/configuration/ftpte.exe.ini");

InstallHelper::copyRequired("$destArtifact64Dir/smartftp.exe.ini",		"$toRes32Dir/configuration/smartftp.exe.ini");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****


#
# Prepare packages
#

my	$package32Dir = "$installDir/build/package32";
my	$package64Dir = "$installDir/build/package64";

InstallHelper::copyRequired("$neConfigDir/product.xml",					"$package32Dir/product.xml");
InstallHelper::copyRequired("$neConfigDir/product.xml",					"$package64Dir/product.xml");


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

#InstallHelper::copyRequired("$installDir/$ismTemplateFileName",			"$assemblyDir/$ismTemplateFileName");
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
