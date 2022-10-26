#!perl
#
# DESCRIPTION
#	This script find all Makefile.inc under a directory and update VERSION_*
#	variables in the Makefile.inc file.

use strict;
use warnings;

use Getopt::Long;
use File::Find;
use File::Basename;


$| = 1;							# Auto flush stdout


#
# Global variables
#

my	$rootPath = "";
my	$versionStr = "";
my	$dryRun = 0;
my	$print = 0;
my	$major = 0;
my	$minor = 0;
my	$maintenance = 0;
my	$patch = 0;


#
# Process parameters
#

print "NextLabs Product Version Number Updating Script\n";

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: updateProductVersion.pl --root=<path> --version=<verison string> [--dryRun]\n";
	print "         [--print]\n";
	print "  dryRun          Perform the command but do not update Makefile.inc file.\n";
	print "  print           Print modified Makefile.inc to stdout.\n";
	print "  root            Specify the topmost directory to start scanning for Makefile.inc.\n";
	print "                  All Makefile.inc under this directory will be updated.\n";
	print "  version         Specify a version string (e.g., 5.5.1.0).\n";
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
			'dryRun' => \$dryRun,				# --dryRun
			'help' => \$help,					# --help
			'print' => \$print,					# --print
			'root=s' => \$rootPath,				# --root
			'version=s' => \$versionStr			# --version
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

	if ($rootPath eq '')
	{
		print "Missing root path\n";
		exit(1);
	}

	if (! -d $rootPath)
	{
		print "Invalid root path\n";
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

	$major = $1;
	$minor = $2;
	$maintenance = $3;
	$patch = $4;

	if ($major < 1 || $major > 100)
	{
		print "Invalid major verison # (expects 1-100)\n";
		exit(1);
	}

	if ($minor < 0 || $minor > 100)
	{
		print "Invalid minor verison # (expects 1-100)\n";
		exit(1);
	}

	if ($maintenance < 0 || $maintenance > 100)
	{
		print "Invalid maintenance verison # (expects 1-100)\n";
		exit(1);
	}

	if ($patch < 0 || $patch > 1000)
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
print "  Root path       = $rootPath\n";
print "  Version String  = $versionStr\n";
print "  Dry run         = $dryRun\n";
print "  Print           = $print\n";
print "  Major           = $major\n";
print "  Minor           = $minor\n";
print "  Maintenance     = $maintenance\n";
print "  Patch           = $patch\n";


#
# General utilities
#

# -----------------------------------------------------------------------------
# Remove leading and trailing whitespaces in a string

sub trim
{
	my	($buf) = @_;

	$buf =~ s/^\s+|\s+$//g;

	return $buf;
}

# -----------------------------------------------------------------------------
# Print array

sub printArray
{
	my	($refArray, $title) = @_;
	my	$count = scalar(@$refArray);

	print "\n$title (total $count):\n";

	foreach (@$refArray)
	{
		print "  $_\n";
	}
}


#
# Locate Makefile.inc
#

print "INFO: Locating makefiles\n";

# File::find() compare function
my	@makeDirList = ();

# -----------------------------------------------------------------------------
# Find::File compare function

sub findFileCompareHelper
{
	my ($filename, $dir, $suffix) = fileparse($_);

	if ($filename =~ /^Makefile$/i)
	{
#		print "$_\n  Parsed: Dir=$dir, File=$filename, Suffix=$suffix\n";

		push @makeDirList, $dir;
	}
}

find({
		wanted => \&findFileCompareHelper,
		no_chdir => 1,
		bydepth => 1
	},
	$rootPath);

my	@makefileList = sort @makeDirList;
my	$makefileCount = $#makefileList + 1;

printArray \@makefileList, "Found Makefiles";


#
#  Modify makefiles
#

print "INFO: Update makefiles\n";

# -----------------------------------------------------------------------------
# Update version info in Makefile.inc

sub updateMakefile
{
	my	$dir = $_[0];

#	print "Processing $dir\n";

	# No Makefile.inc file
	my	$file = $dir . "/Makefile.inc";

	if ( ! -f $file )
	{
		print "No Makefile.inc in $dir\n";
		return -1;
	}

	# Read Makefile.inc and update version
	my	$fileBuf = '';
	my	$changed = 0;

	open FILE, $file || die "Error opening makefile $file\n";

	while (<FILE>)
	{
#		print "Line: $_\n";

		# Modify
		my	$buf = $_;

		$buf =~ s/^\s*VERSION_MAJOR\s*=.*$/VERSION_MAJOR = $major/;
		$buf =~ s/^\s*VERSION_MINOR\s*=.*$/VERSION_MINOR = $minor/;
		$buf =~ s/^\s*VERSION_MAINTENANCE\s*=.*$/VERSION_MAINTENANCE = $maintenance/;
		$buf =~ s/^\s*VERSION_PATCH\s*=.*$/VERSION_PATCH = $patch/;

		# Check if changed
		if ($buf ne $_)
		{
			$changed = 1;
		}

		# Add to file
		$fileBuf .= $buf;
	}

	close FILE;

	# Print to stdout
	if ($print && $changed)
	{
		print "\n--------\n";
		print "FILE: $file\n";
		print $fileBuf;
	}

	# Rewrite Makefile.inc
	if ($changed && !$dryRun)
	{
		print "INFO: Rewriting $file\n";

		if (chmod(0777, $file) == 0)
		{
			die "### ERROR: Failed to chmod on file $file\n";
		}

		open FILE, ">$file" || die "Error opening makefile $file (write)\n";
		print FILE $fileBuf;
		close FILE;
	}

	return $changed;
}

# Process Makefile.inc
my	$missingCount = 0;
my	$changedCount = 0;
my	$unchangedCount = 0;

for my $dir (@makefileList)
{
	my	$status = updateMakefile($dir);

	if ($status == -1)
	{
		$missingCount++;
	}
	elsif ($status == 1)
	{
		$changedCount++;
	}
	else
	{
		$unchangedCount++;
	}
}

# Print status
print "Update Status:\n";
print "  File missing    = $missingCount\n";
print "  Changed         = $changedCount\n";
print "  Unchanged       = $unchangedCount\n";

exit 0;
