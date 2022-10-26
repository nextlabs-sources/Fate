#!perl
#
# DESCRIPTION
# This script updates an InstallShield .ism file.

use strict;
use warnings;

use Getopt::Long;
use File::Basename;
use File::Path qw(make_path remove_tree);
use Text::Diff;

print "NextLabs InstallShield Script Updater\n";


#
# Global variables
#

my	$buildNum = "";
my	$ismTemplateFileName = "";
my	$ismOutputFileName = "";
my	$msiFileName32 = "";
my	$msiFileName64 = "";
my	$versionStr = "";


#
# Functions
#

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
			'buildNum=s' 			=> \$buildNum,				# --buildNum
			'help' 					=> \$help,					# --help
			'msiFileName32=s' 		=> \$msiFileName32,			# --msiFileName32
			'msiFileName64=s' 		=> \$msiFileName64,			# --msiFileName64
			'template=s' 			=> \$ismTemplateFileName,	# --template
			'outputIsm=s' 			=> \$ismOutputFileName,		# --outputIsm
			'versionStr=s' 			=> \$versionStr				# --versionStr
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

	if ($buildNum eq '')
	{
		print "Missing build #\n";
		exit(1);
	}

	if ($ismTemplateFileName eq '')
	{
		print "Missing ISM template file name\n";
		exit(1);
	}

		if ($ismOutputFileName eq '')
	{
		print "Missing ISM output file name\n";
		exit(1);
	}

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

	if ($versionStr eq '')
	{
		print "Missing version string\n";
		exit(1);
	}
}

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: updateIsmFile.pl --template=<file> --msiFileName32=<file>\n";
	print "         --msiFileName64=<file> --versionStr=<version> --buildNum=<#>\n";
	print "  buildNum         Build number (e.g., 274).\n";
	print "  msiFileName32   32-bit installer output .msi file name.\n";
	print "  msiFileName64   64-bit installer output .msi file name.\n";
	print "  template        Name of an InstallShield build script (.ism file).\n";
	print "  versionStr      Product version (e.g., 6.2.0.0).\n";
	print "\nEnvironment Variables:\n";
	print "  NLBUILDROOT     Source tree root (e.g., c:/depot/Fate/main).\n";
}

# -----------------------------------------------------------------------------
# Create a new ISM file based on an existing one

sub constructIsmFile
{
	my	($ismTemplateFile, $ismOutFile, $msiFileName32, $msiFileName64, $versionStr, $buildNum) = @_;
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
		$buf =~ s#</td><td>ProductVersion</td><td>[^<]*</td></row>#</td><td>ProductVersion</td><td>$versionStr.$buildNum</td></row>#g;
		$buf =~ s#<row><td>ProductVersion</td><td>[^<]*</td><td/></row>#<row><td>ProductVersion</td><td>$versionStr.$buildNum</td><td/></row>#g;
		$buf =~ s#<row><td>Version</td><td>[^<]*</td><td/></row>#<row><td>Version</td><td>$versionStr.$buildNum</td><td/></row>#;

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


#
# Main program
#

# Proces command line
my	$argCount = scalar(@ARGV);

if ($argCount < 2 || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")
{
	printUsage;
	exit 1;
}

&parseCommandLine();

# Get environment variable
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

# Print variables
print "INFO: Parameters:\n";
print "INFO:   ismTemplateFileName = $ismTemplateFileName\n";
print "INFO:   ismOutputFileName   = $ismOutputFileName\n";
print "INFO:   msiFileName32       = $msiFileName32\n";
print "INFO:   msiFileName64       = $msiFileName64\n";
print "INFO:   buildRootDir        = $buildRootDir\n";
print "INFO:   buildRootPath       = $buildRootPath\n";

&constructIsmFile($ismTemplateFileName, $ismOutputFileName, $msiFileName32, $msiFileName64, $versionStr, $buildNum);
