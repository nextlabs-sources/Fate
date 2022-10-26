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
my	$maintenanceVer = 0;
my	$patchVer = 0;
my	$msiFilePrefix = "removable-device-enforcer";


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

my	$installDir = "$buildRootDir/install/rde";
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

my	$rdeProjDir = "$buildRootDir/prod/pep/endpoint/rde";
my	$rdeDriverDir = "$rdeProjDir/driver";
my	$srcBin32Dir = "$buildRootDir/bin/release_win_x86";
my	$srcBin64Dir = "$buildRootDir/bin/release_win_x64";
my	$destBin32Dir = "$assemblyDir/bin32";
my	$destBin64Dir = "$assemblyDir/bin64";
my	$releaseOnly = 0;

if ($buildType eq "release")
{
	$releaseOnly = 1;
}

my	@bin32FileList = (
		["nl_devenf_plugin32.dll",				$versionStr, "x86", $releaseOnly],
		["logon_detection_win7.exe",			$versionStr, "x86", $releaseOnly],
		["NextLabsCredentialProvider32.dll",	$versionStr, "x86", $releaseOnly],
		["nlPnpDriverInstaller32.dll",			$versionStr, "x86", $releaseOnly],
		["nlQuench.exe",						"5.5.4.0", "x86", $releaseOnly],
		["PluginInstallerSDK32.dll",			"5.5.4.0", "x86", $releaseOnly]
	);
my	@bin64FileList = (
		["nl_devenf_plugin.dll",				$versionStr, "x64", $releaseOnly],
		["logon_detection_win7.exe",			$versionStr, "x64", $releaseOnly],
		["NextLabsCredentialProvider.dll",		$versionStr, "x64", $releaseOnly]
);


InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin32Dir, $destBin32Dir, \@bin32FileList);
InstallHelper::copyListVersionArchitectureAndReleaseRequired($srcBin64Dir, $destBin64Dir, \@bin64FileList);

InstallHelper::copyRequired("$srcBin32Dir/nl_devenf.sys",					"$destBin32Dir/nl_devenf.sys");
InstallHelper::copyRequired("$srcBin64Dir/nl_devenf.sys",					"$destBin64Dir/nl_devenf.sys");

#
# Copy external libraries
#

print "INFO: Copying external libraries\n";

my $xlibX86Dir = "$buildRootDir/xlib/release_win_x86";
my $xlibX64Dir = "$buildRootDir/xlib/release_win_x64";


# 32-bit binaries
InstallHelper::copyRequired("$xlibX86Dir/WdfCoInstaller01009.dll",	"$destBin32Dir/WdfCoInstaller01009.dll");

# 64-bit binaries
InstallHelper::copyRequired("$xlibX64Dir/WdfCoInstaller01009.dll",	"$destBin64Dir/WdfCoInstaller01009.dll");


#
#  Prepare support files
#

print "INFO: Copying support files\n";

my	$rdeConfigDir = "$rdeProjDir/configuration";
my	$rdeUserDir = "$rdeProjDir/user";
my	$srcResDir = "$installDir/resource";
my	$destResDir = "$assemblyDir/resource";
my	$destRes32Dir = "$assemblyDir/resource32";
my	$destRes64Dir = "$assemblyDir/resource64";

# Common
#InstallHelper::copyRequired("$rdeConfigDir/license.cfg",					"$destResDir/license.cfg");
InstallHelper::copyRequired("$rdeConfigDir/TamperResistance.cfg",			"$destResDir/RemovableDeviceEnforcer_TamperResistance.cfg");

InstallHelper::copyRequired("$rdeUserDir/nl_devenf_plugin.def",				"$destResDir/nl_devenf_plugin.def");

InstallHelper::copyRequired("$srcResDir/README.txt",									"$destResDir/README.txt");
InstallHelper::copyRequired("$srcResDir/NextLabs Clickwrap Agreement v5-07 (2).rtf",	"$destResDir/NextLabs Clickwrap Agreement v5-07 (2).rtf");
InstallHelper::copyRequired("$srcResDir/ce-32.ico",										"$destResDir/ce-32.ico");
InstallHelper::copyRequired("$srcResDir/Nextlabs Removable Device Enforcer Uninstall.bat","$destResDir/Nextlabs Removable Device Enforcer Uninstall.bat");

# 32-bit
InstallHelper::copyRequired("$rdeDriverDir/nl_devenf.inf",					"$destRes32Dir/nl_devenf.inf");
InstallHelper::copyRequired("$srcBin32Dir/nl_devenf.x86.cat",				"$destRes32Dir/nl_devenf.x86.cat");

InstallHelper::copyRequired("$rdeConfigDir/rde_plugin32.cfg",				"$destRes32Dir/rde_plugin32.cfg");

InstallHelper::copyRequired("$srcBin32Dir/nldevcon.exe",					"$destRes32Dir/nldevcon.exe");

# 64-bit
InstallHelper::copyRequired("$rdeDriverDir/nl_devenf.inf",					"$destRes64Dir/nl_devenf.inf");
InstallHelper::copyRequired("$srcBin64Dir/nl_devenf.x64.cat",				"$destRes64Dir/nl_devenf.x64.cat");

InstallHelper::copyRequired("$rdeConfigDir/rde_plugin.cfg",					"$destRes64Dir/rde_plugin.cfg");

InstallHelper::copyRequired("$srcBin64Dir/nldevcon.exe",					"$destRes64Dir/nldevcon.exe");

# Update readme file
InstallHelper::updateReadMeFile("$destResDir/README.txt", $majorVer, $minorVer, $maintenanceVer, $patchVer, $buildNum);


#
# Prepare packages
#

my	$package32Dir = "$installDir/build/package32";
my	$package64Dir = "$installDir/build/package64";

InstallHelper::copyRequired("$rdeConfigDir/product.xml",					"$package32Dir/product.xml");

InstallHelper::copyRequired("$rdeConfigDir/product.xml",					"$package64Dir/product.xml");


#
# Prepare InstallShield files
#

print "INFO: Copying InstallShield files\n";

my	$srcScriptDir = "$installDir/Script Files";
my	$destScriptDir = "$assemblyDir/Script Files";

#InstallHelper::copyRequired("$installDir/$ismTemplateFileName",			"$assemblyDir/$ismTemplateFileName");

#dircopy($srcScriptDir, $destScriptDir) || die "### ERROR: Failed to copy resource from $srcScriptDir to $destScriptDir.\n";
if (system("cp -fR \"$srcScriptDir\" \"$destScriptDir\""))
{
	die "### ERROR: Failed to copy resource from $srcScriptDir to $destScriptDir.\n";
}


#
# Modify installer build script
#

print "INFO: modify installer script\n";

my	$templateFile = "$installDir/$ismTemplateFileName";
#my	$newIsmFile = "$assemblyDir/${ismFileBaseName}.ism";
my	$newIsmFile = "$assemblyDir/$ismTemplateFileName";

InstallHelper::constructIsmFile($templateFile, $newIsmFile, $msiFileName32, $msiFileName64, $versionStr, $buildNum);
