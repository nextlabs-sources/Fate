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
use File::Basename;
use Text::Diff;
use InstallHelper;


print "NextLabs Installer Assembly Preparation Script\n";


#
# Global variables
#

my	$ismTemplateFileName = "";
my	$msiFileName32 = "";
my	$msiFileName64 = "";


#
# Process parameters
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: prepareAssembly.pl --template=<file> --msiFileName32=<file>\n";
	print "         --msiFileName64=<file>\n";
	print "  msiFileName32   32-bit installer output .msi file name.\n";
	print "  msiFileName64   64-bit installer output .msi file name.\n";
	print "  template        Name of an InstallShield build script (.ism file).\n";
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
			'help' => \$help,						# --help
			'msiFileName32=s' => \$msiFileName32,	# --msiFileName32
			'msiFileName64=s' => \$msiFileName64,	# --msiFileName64
			'template=s' => \$ismTemplateFileName	# --template
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

	if ($msiFileName32 eq '')
	{
		print "Missing 32-bit MSI file name\n";
		exit(1);
	}

	if ($msiFileName64 eq '')
	{
		print "Missing 64-bit MSI file name\n";
		exit(1);
	}

	if ($ismTemplateFileName eq '')
	{
		print "Missing ISM template file name\n";
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
print "  Template File Name     = $ismTemplateFileName\n";
print "  MSI file name (32-bit) = $msiFileName32\n";
print "  MSI file name (64-bit) = $msiFileName64\n";


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
# Modify installer script
#

# -----------------------------------------------------------------------------
# Extract product version #s

sub extractProductVersions
{
	my	($versionFile) = @_;

	print "INFO: Extracting product versions from $versionFile.\n";

	if (! -e $versionFile)
	{
		die "### ERROR: File $versionFile does not exist.\n";
	}

	# Read version file
	my	$versionStr_PC = "";
	my	$buildNum_PC = "";
	my	$versionStr_KMC = "";
	my	$buildNum_KMC = "";
	my	$versionStr_WDE = "";
	my	$buildNum_WDE = "";
	my	$versionStr_NE = "";
	my	$buildNum_NE = "";
	my	$versionStr_RDE = "";
	my	$buildNum_RDE = "";
	my	$versionStr_OE = "";
	my	$buildNum_OE = "";
	my	$versionStr_OCE = "";
	my	$buildNum_OCE = "";
	my	$versionStr_LME = "";
	my	$buildNum_LME = "";
	my	$versionStr_SE = "";
	my	$buildNum_SE = "";
	my	$versionStr_SPWFPA = "";
	my	$buildNum_SPWFPA = "";
	my	$versionStr_Updater = "";
	my	$buildNum_Updater = "";

	open FILE, $versionFile || die "Error opening version file $versionFile\n";

	while (my $buf = <FILE>)
	{
#		print "VER: $buf";

		# NextLabs Data Protection v5.5.11.502 (10001PS_lmeOceRde)
		if ($buf =~ /NextLabs Data Protection v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_Updater = $1;
			$buildNum_Updater = $2;
		}

		if ($buf =~ /(Policy Controller|Policy Controller for Microsoft Windows) v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_PC = $2;
			$buildNum_PC = $3;
		}

 		if ($buf =~ /Key Management Client v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_KMC = $1;
			$buildNum_KMC = $2;
		}

 		if ($buf =~ /Desktop Enforcer for Microsoft Windows v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_WDE = $1;
			$buildNum_WDE = $2;
		}

		if ($buf =~ /Network Enforcer for Windows v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_NE = $1;
			$buildNum_NE = $2;
		}

 		if ($buf =~ /Removable Device Enforcer for Microsoft Windows v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_RDE = $1;
			$buildNum_RDE = $2;
		}

 		if ($buf =~ /Enforcer for Microsoft Outlook v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_OE = $1;
			$buildNum_OE = $2;
		}

 		if ($buf =~ /Enforcer for Microsoft Live Meeting v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_LME = $1;
			$buildNum_LME = $2;
		}

 		if ($buf =~ /Enforcer for Microsoft Office Communicator v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_OCE = $1;
			$buildNum_OCE = $2;
		}

 		if ($buf =~ /System Encryption v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_SE = $1;
			$buildNum_SE = $2;
		}

 		if ($buf =~ /Workflow Policy Assistant for Microsoft SharePoint v(\d+\.\d+\.\d+\.\d+) \(([^)]+)\)/)
		{
			$versionStr_SPWFPA = $1;
			$buildNum_SPWFPA = $2;
		}
	}

	close FILE;

	# Print extracted values
	print "Extracted product versions:\n";
	print "  versionStr_PC       = $versionStr_PC\n";
	print "  buildNum_PC         = $buildNum_PC\n";
	print "  versionStr_KMC      = $versionStr_KMC\n";
	print "  buildNum_KMC        = $buildNum_KMC\n";
	print "  versionStr_WDE      = $versionStr_WDE\n";
	print "  buildNum_WDE        = $buildNum_WDE\n";
	print "  versionStr_NE       = $versionStr_NE\n";
	print "  buildNum_NE         = $buildNum_NE\n";
	print "  versionStr_RDE      = $versionStr_RDE\n";
	print "  buildNum_RDE        = $buildNum_RDE\n";
	print "  versionStr_OE       = $versionStr_OE\n";
	print "  buildNum_OE         = $buildNum_OE\n";
	print "  versionStr_OCE      = $versionStr_OCE\n";
	print "  buildNum_OCE        = $buildNum_OCE\n";
	print "  versionStr_LME      = $versionStr_LME\n";
	print "  buildNum_LME        = $buildNum_LME\n";
	print "  versionStr_SE       = $versionStr_SE\n";
	print "  buildNum_SE         = $buildNum_SE\n";
	print "  versionStr_SPWFPA   = $versionStr_SPWFPA\n";
	print "  buildNum_SPWFPA     = $buildNum_SPWFPA\n";
	print "  versionStr_Updater  = $versionStr_Updater\n";
	print "  buildNum_Updater    = $buildNum_Updater\n";

	return ($versionStr_PC, $buildNum_PC, $versionStr_KMC, $buildNum_KMC, $versionStr_WDE, $buildNum_WDE,
		$versionStr_NE, $buildNum_NE, $versionStr_RDE, $buildNum_RDE, $versionStr_OE, $buildNum_OE,
		$versionStr_OCE, $buildNum_OCE, $versionStr_LME, $buildNum_LME, $versionStr_SE, $buildNum_SE,
		$versionStr_SPWFPA, $buildNum_SPWFPA, $versionStr_Updater, $buildNum_Updater);
}

# -----------------------------------------------------------------------------
# Create a new ISM file based on an existing one

sub constructIsmFile_updater
{
	my	($ismTemplateFile, $ismOutFile, $msiFileName32, $msiFileName64, $versionStr_Updater, $buildNum_Updater,
		$versionStr_PC, $buildNum_PC, $versionStr_KMC, $buildNum_KMC, $versionStr_WDE, $buildNum_WDE,
		$versionStr_NE, $buildNum_NE, $versionStr_RDE, $buildNum_RDE, $versionStr_OE, $buildNum_OE,
		$versionStr_OCE, $buildNum_OCE, $versionStr_LME, $buildNum_LME, $versionStr_SE, $buildNum_SE,
		$versionStr_SPWFPA, $buildNum_SPWFPA) = @_;
	my	$outDir = dirname($ismOutFile);

	print "INFO: Constructing InstallShield ISM file $ismOutFile based on template $ismTemplateFile.\n";

	# Delete output file
	if (-e $ismOutFile)
	{
		unlink($ismOutFile) || die "### ERROR: Failed to remove destination file $ismOutFile. Cannot construct ISM file.\n";
	}

	# Create output directory
	if (! -d $outDir)
	{
		make_path($outDir, {mode => 0711, error => \my $err});

		if (@$err)
		{
			for my $diag (@$err)
			{
				my	($file, $message) = %$diag;

				if ($file eq "")
				{
					print "### ERROR: Failed to create directory $outDir. Cause: $message.\n";
				}
				else
				{
					print "### ERROR: Failed to create directory $file. Cause: $message.\n";
				}
			}

			die "### ERROR: Failed to construct ISM file $ismOutFile based on $ismTemplateFile.\n";
		}
	}

	# Read ISM file
	my	$data = "";

	open FILE, $ismTemplateFile || die "Error opening ISM file $ismTemplateFile\n";

	while (my $buf = <FILE>)
	{
#		print "ISM: $buf";

		$buf =~ s#<row><td>\s*32bit\s*</td><td>MSIPackageFileName</td><td>[^<]*</td></row>#<row><td>32bit</td><td>MSIPackageFileName</td><td>$msiFileName32</td></row>#g;
		$buf =~ s#<row><td>\s*64bit\s*</td><td>MSIPackageFileName</td><td>[^<]*</td></row>#<row><td>64bit</td><td>MSIPackageFileName</td><td>$msiFileName64</td></row>#g;
		$buf =~ s#<row><td>\s*32bit\s*</td><td>ProductName</td><td>[^<]*</td></row>#<row><td>32bit</td><td>ProductName</td><td>$msiFileName32</td></row>#g;
		$buf =~ s#<row><td>\s*64bit\s*</td><td>ProductName</td><td>[^<]*</td></row>#<row><td>64bit</td><td>ProductName</td><td>$msiFileName64</td></row>#g;
		$buf =~ s#</td><td>ProductVersion</td><td>[^<]*</td></row>#</td><td>ProductVersion</td><td>$versionStr_Updater ($buildNum_Updater)</td></row>#g;
		$buf =~ s#<row><td>ProductVersion</td><td>[^<]*</td><td/></row>#<row><td>ProductVersion</td><td>$versionStr_Updater ($buildNum_Updater)</td><td/></row>#g;

		$buf =~ s#<row><td>Version_KMC</td><td>[^<]*</td><td/></row>#<row><td>Version_KMC</td><td>$versionStr_KMC ($buildNum_KMC)</td><td/></row>#;
		$buf =~ s#<row><td>Version_LME</td><td>[^<]*</td><td/></row>#<row><td>Version_LME</td><td>$versionStr_LME ($buildNum_LME</td><td/></row>#;
		$buf =~ s#<row><td>Version_NE</td><td>[^<]*</td><td/></row>#<row><td>Version_NE</td><td>$versionStr_NE ($buildNum_NE)</td><td/></row>#;
		$buf =~ s#<row><td>Version_OCE</td><td>[^<]*</td><td/></row>#<row><td>Version_OCE</td><td>$versionStr_OCE ($buildNum_OCE)</td><td/></row>#;
		$buf =~ s#<row><td>Version_OE</td><td>[^<]*</td><td/></row>#<row><td>Version_OE</td><td>$versionStr_OE ($buildNum_OE)</td><td/></row>#;
		$buf =~ s#<row><td>Version_PC</td><td>[^<]*</td><td/></row>#<row><td>Version_PC</td><td>$versionStr_PC ($buildNum_PC)</td><td/></row>#;
		$buf =~ s#<row><td>Version_RDE</td><td>[^<]*</td><td/></row>#<row><td>Version_RDE</td><td>$versionStr_RDE ($buildNum_RDE)</td><td/></row>#;
		$buf =~ s#<row><td>Version_SE</td><td>[^<]*</td><td/></row>#<row><td>Version_SE</td><td>$versionStr_SE ($buildNum_SE)</td><td/></row>#;
		$buf =~ s#<row><td>Version_WDE</td><td>[^<]*</td><td/></row>#<row><td>Version_WDE</td><td>$versionStr_WDE ($buildNum_WDE)</td><td/></row>#;

		$data .= $buf;
	}

	close FILE;

	# Write ISM file
	open FILE, ">$ismOutFile" || die "Error opening output file $ismOutFile\n";
	print FILE $data;
	close FILE;

	# Print differences
	my	$result = diff($ismTemplateFile, $ismOutFile, {STYLE => "OldStyle"});

	print "INFO:   ISM File Differences:\n";
	print $result;

	print "INFO:   Successfully wrote ISM file $ismOutFile.\n";
}

print "INFO: modify installer script\n";

my	$installDir = "$buildRootDir/install/updater";
my	$assemblyDir = "$installDir/build/data";
my	$templateFile = "$installDir/$ismTemplateFileName";
my	$newIsmFile = "$assemblyDir/$ismTemplateFileName";
my	$versionFile = "$assemblyDir/NextLabsDataProtection.ver";
my	$versionStr_PC = "";
my	$buildNum_PC = "";
my	$versionStr_KMC = "";
my	$buildNum_KMC = "";
my	$versionStr_WDE = "";
my	$buildNum_WDE = "";
my	$versionStr_NE = "";
my	$buildNum_NE = "";
my	$versionStr_RDE = "";
my	$buildNum_RDE = "";
my	$versionStr_OE = "";
my	$buildNum_OE = "";
my	$versionStr_OCE = "";
my	$buildNum_OCE = "";
my	$versionStr_LME = "";
my	$buildNum_LME = "";
my	$versionStr_SE = "";
my	$buildNum_SE = "";
my	$versionStr_SPWFPA = "";
my	$buildNum_SPWFPA = "";
my	$versionStr_Updater = "";
my	$buildNum_Updater = "";

($versionStr_PC, $buildNum_PC, $versionStr_KMC, $buildNum_KMC, $versionStr_WDE, $buildNum_WDE,
		$versionStr_NE, $buildNum_NE, $versionStr_RDE, $buildNum_RDE, $versionStr_OE, $buildNum_OE,
		$versionStr_OCE, $buildNum_OCE, $versionStr_LME, $buildNum_LME, $versionStr_SE, $buildNum_SE,
		$versionStr_SPWFPA, $buildNum_SPWFPA, $versionStr_Updater, $buildNum_Updater) = &extractProductVersions($versionFile);
&constructIsmFile_updater($templateFile, $newIsmFile, $msiFileName32, $msiFileName64, $versionStr_Updater, $buildNum_Updater,
		$versionStr_PC, $buildNum_PC, $versionStr_KMC, $buildNum_KMC, $versionStr_WDE, $buildNum_WDE,
		$versionStr_NE, $buildNum_NE, $versionStr_RDE, $buildNum_RDE, $versionStr_OE, $buildNum_OE,
		$versionStr_OCE, $buildNum_OCE, $versionStr_LME, $buildNum_LME, $versionStr_SE, $buildNum_SE,
		$versionStr_SPWFPA, $buildNum_SPWFPA);
