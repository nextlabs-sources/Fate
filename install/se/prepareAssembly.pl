#!perl
#
# DESCRIPTION
# This script prepare an installer assembly directory for building an installer.
#
# IMPORTANT:
#	1. "Script Files" folder must be in the same directory as *.ism file. Otherwise, you will get
#		an error message like this:
#			ISDEV : error -7132: An error occurred streaming ISSetup.dll support file
#			c:\nightly\current\D_SiriusR2\install\se\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\
#			desktop-enforcer-5.5.0.0-10001-dev-20110321063014\Script Files\Setup.inx
#		And you will not see these messages at the beginning:
#			Compiling...
#			Setup.rul
#			c:\nightly\current\D_SiriusR2\install\se\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\script files\Setup.rul(90)
#				: warning W7503: 'ProcessEnd' : function defined but never called
#			c:\nightly\current\D_SiriusR2\install\se\desktop-enforcer-5.5.0.0-10001-dev-20110321063014\script files\Setup.rul(90)
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
my	$msiFilePrefix = "system-encryption";


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

my	$installDir = "$buildRootDir/install/se";
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
my	$srcJavaDir = "$buildRootDir/bin/java";
my	$destBin32Dir = "$assemblyDir/release_win_x86";
my	$destBin64Dir = "$assemblyDir/release_win_x64";
my	$destBinWinXPDir = "$assemblyDir/release_win_winxp";
my	$destJavaDir = "$assemblyDir/jars";
my	$releaseOnly = 0;

if ($buildType eq "release")
{
	$releaseOnly = 1;
}

my	@bin32FileList = (
		["KeyManagementConsumer32.dll",		$versionStr, "x86", $releaseOnly],
		["IconBadging32.dll",				$versionStr, "x86", $releaseOnly],
		["nl_autounwrapper.exe",			$versionStr, "x86", $releaseOnly],
		["NLPortableEncryption.exe",		$versionStr, "x86", $releaseOnly],
		["NLPortableEncryptionCtx32.dll",	$versionStr, "x86", $releaseOnly],
		["nlse_plugin32.dll",				$versionStr, "x86", $releaseOnly],
		["nlsefw_plugin32.dll",				$versionStr, "x86", $releaseOnly],
		["nlQuench.exe",					$versionStr, "x86", $releaseOnly],
		["NlRegisterPlugins32.exe",			$versionStr, "x86", $releaseOnly],
		["nlSysEncryption.exe",				$versionStr, "x86", $releaseOnly],
		["nlSysEncryptionObligation.exe",	$versionStr, "x86", $releaseOnly],
		["pcs_server32.dll",				$versionStr, "x86", $releaseOnly],
		["PluginInstallerSDK32.dll",		$versionStr, "x86", $releaseOnly]
	);
my	@bin64FileList = (
		["KeyManagementConsumer.dll",		$versionStr, "x64", $releaseOnly],
		["IconBadging.dll",					$versionStr, "x64", $releaseOnly],
		["nl_autounwrapper.exe",			$versionStr, "x64", $releaseOnly],
		["NLPortableEncryption.exe",		$versionStr, "x64", $releaseOnly],
		["NLPortableEncryptionCtx.dll",		$versionStr, "x64", $releaseOnly],
		["nlse_plugin.dll",					$versionStr, "x64", $releaseOnly],
		["nlsefw_plugin.dll",				$versionStr, "x64", $releaseOnly],
		["NlRegisterPlugins.exe",			$versionStr, "x64", $releaseOnly],
		["nlSysEncryption.exe",				$versionStr, "x64", $releaseOnly],
		["nlSysEncryptionObligation.exe",	$versionStr, "x64", $releaseOnly],
		["pcs_server.dll",					$versionStr, "x64", $releaseOnly]
);

InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin32Dir, $destBin32Dir, \@bin32FileList);
InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin64Dir, $destBin64Dir, \@bin64FileList);

# 32-bit
InstallHelper::copyRequired("$srcBin32Dir/nl_crypto.sys",					"$destBin32Dir/nl_crypto.dll");
InstallHelper::copyRequired("$srcBin32Dir/nl_klog.sys",						"$destBin32Dir/nl_klog.dll");
InstallHelper::copyRequired("$srcBin32Dir/nl_SysEncryption.sys",			"$destBin32Dir/nl_SysEncryption.sys");
InstallHelper::copyRequired("$srcBin32Dir/nl_SysEncryptionFW.sys",			"$destBin32Dir/nl_SysEncryptionFW.sys");

# 64-bit
InstallHelper::copyRequired("$srcBin64Dir/nl_crypto.sys",					"$destBin64Dir/nl_crypto.dll");
InstallHelper::copyRequired("$srcBin64Dir/nl_klog.sys",						"$destBin64Dir/nl_klog.dll");
InstallHelper::copyRequired("$srcBin64Dir/nl_SysEncryption.sys",			"$destBin64Dir/nl_SysEncryption.sys");
InstallHelper::copyRequired("$srcBin64Dir/nl_SysEncryptionFW.sys",			"$destBin64Dir/nl_SysEncryptionFW.sys");

# Java
InstallHelper::copyRequired("$srcJavaDir/SystemEncryptionService.jar",		"$destJavaDir/SystemEncryptionService.jar");


# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup desired structure
my	$toRes32Dir = "$assemblyDir/main/resource";
my	$toRes64Dir = "$assemblyDir/main/resource64";
my	$toResWinXPDir = "$assemblyDir/main/resourcexp";

InstallHelper::copyRequired("$destBin32Dir/nl_crypto.dll",					"$toRes32Dir/nl_crypto.dll");
InstallHelper::copyRequired("$destBin32Dir/nl_klog.dll",					"$toRes32Dir/nl_klog.dll");
InstallHelper::copyRequired("$destBin32Dir/nl_SysEncryption.sys",			"$toRes32Dir/nl_SysEncryption.sys");
InstallHelper::copyRequired("$destBin32Dir/nl_SysEncryptionFW.sys",			"$toRes32Dir/nl_SysEncryptionFW.sys");

InstallHelper::copyRequired("$destBin64Dir/nl_crypto.dll",					"$toRes64Dir/nl_crypto.dll");
InstallHelper::copyRequired("$destBin64Dir/nl_klog.dll",					"$toRes64Dir/nl_klog.dll");
InstallHelper::copyRequired("$destBin64Dir/nl_SysEncryption.sys",			"$toRes64Dir/nl_SysEncryption.sys");
InstallHelper::copyRequired("$destBin64Dir/nl_SysEncryptionFW.sys",			"$toRes64Dir/nl_SysEncryptionFW.sys");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****

# TBD:
# SystemEncryptionService.jar
# ISProductFolder&gt;\samples\windowsinstaller\dimreferencetutorial\nlse_plugin32.cfg


#
#  Prepare artifacts
#

# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup desired structure
my	$fromDir = "$installDir/src/resource";
my	$toDir = "$installDir/artifacts";

InstallHelper::copyRequired("$fromDir/README.txt",									"$toDir/README.txt");
InstallHelper::copyRequired("$fromDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$toDir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$fromDir/ce-32.ico",									"$toDir/ce-32.ico");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****

print "INFO: Copying artifacts\n";

my	$seConfigDir = "$buildRootDir/prod/pep/endpoint/se/configuration";
my	$seFilterDir = "$buildRootDir/prod/pep/endpoint/se/nl_SysEncryption/filter";
my	$seFWFilterDir = "$buildRootDir/prod/pep/endpoint/se/nl_SysEncryptionFW/filter";
my	$srcArtifactDir = "$installDir/artifacts";
my	$destArtifactDir = "$assemblyDir/artifacts";
my	$destArtifact32Dir = "$assemblyDir/artifacts32";
my	$destArtifact64Dir = "$assemblyDir/artifacts64";

# Common
InstallHelper::copyRequired("$seConfigDir/license.cfg",							"$destArtifactDir/license.cfg");
InstallHelper::copyRequired("$seConfigDir/SystemEncryptionService.properties",	"$destArtifactDir/SystemEncryptionService.properties");
InstallHelper::copyRequired("$seConfigDir/TamperResistance.cfg",				"$destArtifactDir/SystemEncryption_TamperResistance.cfg");
InstallHelper::copyRequired("$seConfigDir/SystemEncryption.cfg",				"$destArtifactDir/SystemEncryption.cfg");

InstallHelper::copyRequired("$seFilterDir/NLSE.inf",					"$destArtifactDir/NLSE.inf");
InstallHelper::copyRequired("$seFWFilterDir/NLSEFW.inf",				"$destArtifactDir/NLSEFW.inf");

InstallHelper::copyRequired("$srcArtifactDir/README.txt",									"$destArtifactDir/README.txt");
InstallHelper::copyRequired("$srcArtifactDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$destArtifactDir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$srcArtifactDir/ce-32.ico",									"$destArtifactDir/ce-32.ico");

# 32-bit
InstallHelper::copyRequired("$seConfigDir/nlse_plugin32.cfg",			"$destArtifact32Dir/nlse_plugin32.cfg");
InstallHelper::copyRequired("$seConfigDir/nlsefw_plugin32.cfg",			"$destArtifact32Dir/nlsefw_plugin32.cfg");

# 64-bit
InstallHelper::copyRequired("$seConfigDir/nlse_plugin.cfg",				"$destArtifact64Dir/nlse_plugin.cfg");
InstallHelper::copyRequired("$seConfigDir/nlsefw_plugin.cfg",			"$destArtifact64Dir/nlsefw_plugin.cfg");

# Update readme file
InstallHelper::updateReadMeFile("$destArtifactDir/README.txt", $majorVer, $minorVer, $maintenanceVer, $patchVer, $buildNum);


# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup old structure
InstallHelper::copyRequired("$destArtifactDir/SystemEncryptionService.properties",			"$toRes32Dir/SystemEncryptionService.properties");
InstallHelper::copyRequired("$destArtifactDir/SystemEncryption_TamperResistance.cfg",		"$toRes32Dir/SystemEncryption_TamperResistance.cfg");
InstallHelper::copyRequired("$destArtifactDir/NLSE.inf",									"$toRes32Dir/NLSE.inf");
InstallHelper::copyRequired("$destArtifactDir/NLSEFW.inf",									"$toRes32Dir/NLSEFW.inf");
InstallHelper::copyRequired("$destArtifactDir/README.txt",									"$toRes32Dir/README.txt");
InstallHelper::copyRequired("$destArtifactDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$toRes32Dir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$destArtifactDir/ce-32.ico",									"$assemblyDir/main/image/ce-32.ico");

InstallHelper::copyRequired("$destArtifact32Dir/nlse_plugin32.cfg",		"$toRes32Dir/nlse_plugin32.cfg");
InstallHelper::copyRequired("$destArtifact32Dir/nlsefw_plugin32.cfg",	"$toRes32Dir/nlsefw_plugin32.cfg");

InstallHelper::copyRequired("$destArtifact64Dir/nlse_plugin.cfg",		"$toRes64Dir/nlse_plugin.cfg");
InstallHelper::copyRequired("$destArtifact64Dir/nlsefw_plugin.cfg",		"$toRes64Dir/nlsefw_plugin.cfg");
# ***** TEMPORARY WORKAROUND END -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****


#
# Prepare packages
#

print "INFO: Preparing packages\n";

my	$package32Dir = "$installDir/build/package32";
my	$package64Dir = "$installDir/build/package64";

InstallHelper::copyRequired("$seConfigDir/product.xml",					"$package32Dir/product.xml");
InstallHelper::copyRequired("$seConfigDir/EmbeddedPolicies.xml",		"$package32Dir/EmbeddedPolicies.xml");

InstallHelper::copyRequired("$seConfigDir/product.xml",					"$package64Dir/product.xml");
InstallHelper::copyRequired("$seConfigDir/EmbeddedPolicies.xml",		"$package64Dir/EmbeddedPolicies.xml");

# NLSEDecompression
my	$decompZipFile32 = "$package32Dir/NLSEDecompression-32-$versionStr.zip";
my	$decompZipFile64 = "$package64Dir/NLSEDecompression-64-$versionStr.zip";

system("zip -j $decompZipFile32 $srcBin32Dir/NLSEDecompression.exe") && die "Failed to zip NLSEDecompression x86.\n";
system("zip -j $decompZipFile64 $srcBin64Dir/NLSEDecompression.exe") && die "Failed to zip NLSEDecompression x64.\n";


# NLSERecovery
my	$work32Dir = "$installDir/build/work32";
my	$work64Dir = "$installDir/build/work64";
my	$recoveryZipFile32 = "$package32Dir/NLSERecovery-32-$versionStr.zip";
my	$recoveryZipFile64 = "$package64Dir/NLSERecovery-64-$versionStr.zip";

InstallHelper::copyRequired("$srcBin32Dir/NLSERecovery.exe",					"$work32Dir/NLSERecovery.exe");
InstallHelper::copyRequired("$srcBin32Dir/pcs_server32.dll",					"$work32Dir/pcs_server32.dll");
InstallHelper::copyRequired("$srcBin32Dir/cesdk32.dll",							"$work32Dir/cesdk32.dll");
InstallHelper::copyRequired("$srcBin32Dir/KeyManagementConsumer32.dll",			"$work32Dir/KeyManagementConsumer32.dll");
InstallHelper::copyRequired("$srcJavaDir/SystemEncryptionService.jar",			"$work32Dir/SystemEncryptionService.jar");
InstallHelper::copyRequired("$seConfigDir/SystemEncryptionService.properties",	"$work32Dir/SystemEncryptionService.properties");

system("zip -rj $recoveryZipFile32 $work32Dir") && die "Failed to zip NLSERecovery x86.\n";

InstallHelper::copyRequired("$srcBin64Dir/NLSERecovery.exe",					"$work64Dir/NLSERecovery.exe");
InstallHelper::copyRequired("$srcBin64Dir/pcs_server.dll",						"$work64Dir/pcs_server.dll");
InstallHelper::copyRequired("$srcBin64Dir/cesdk.dll",							"$work64Dir/cesdk.dll");
InstallHelper::copyRequired("$srcBin64Dir/KeyManagementConsumer.dll",			"$work64Dir/KeyManagementConsumer.dll");
InstallHelper::copyRequired("$srcJavaDir/SystemEncryptionService.jar",			"$work64Dir/SystemEncryptionService.jar");
InstallHelper::copyRequired("$seConfigDir/SystemEncryptionService.properties",	"$work64Dir/SystemEncryptionService.properties");

system("zip -rj $recoveryZipFile64 $work64Dir") && die "Failed to zip NLSERecovery x64.\n";


#
# Prepare InstallShield files
#

# ***** TEMPORARY WORKAROUND BEGIN -- SHOULD BE REMOVED AFTER ISM SCRIPT IS FIXED *****
# Setup desired structure
my	$fromOblDir = "$installDir/Script Files/include";

InstallHelper::copyRequired("$fromOblDir/CommonInstallScript.obl",		"$installDir/CommonInstallScript.obl");
InstallHelper::copyRequired("$fromOblDir/PluginInstallerSDK.obl",		"$installDir/PluginInstallerSDK.obl");
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
