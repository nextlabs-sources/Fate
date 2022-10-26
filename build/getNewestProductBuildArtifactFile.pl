#!perl
#
# DESCRIPTION
#	This script finds a build artifact file (*-bin.zip) of the latest build under
#	the specified product folder. The full path of the build artifact file is saved
#	in a Bash script.
#
#	The scanning logic makes the following assumptions:
#
#	1. The start folder specified is at the version # level.
#		For example:
#		- S:\build\release_candidate\artifacts\Fate\6.0.0.0
#		- S:\build\release_candidate\artifacts\PolicyStudio\6.0.0.701
#		- S:\build\pcv\artifacts\SharePointEnforcer\6.0.0.512
#	2. 	The build number folder may be numeric or start with a build #.
#		For example:
#		- 128
#		- 26PS_perf
#		- 81PS_officepep
#	3. Build artifact zip file always ends with -bin.zip or -build.zip
#		For example:
#		- fate-6.0.0.0-151-release-20120607-bin.zip
#		- fate-6.0.0.509-81PS_officepep-pcv-20120607-bin.zip
#		- spe-6.0.0.511-25PS_perf-pcv-20120602-bin.zip
#		- destiny-6.0.0.703-15PS-archon-201211142106-build.zip
#

use strict;
use warnings;

use Getopt::Long;

print "NextLabs Newest Build Artifact File Locator v0.1\n";


#
# Global variables
#

my	$g_help = 0;
my	$g_verbose = 0;
my	$g_startPath = '';
my	$g_outFile = '';
my	$g_varName = '';


#
# Functions
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: getNewestProductBuildArtifactFile.pl [--verbose] --startpath=<path> --outfile=<file>\n";
	print "         --varname=<name>\n";
	print "       getNewestProductBuildArtifactFile.pl <-h|--help>\n";
	print "  help          Print this message (also -h)\n";
	print "  outfile       Output script file containing artifact file.\n";
	print "  startpath     Absolute path to a folder in build repository. The folder should contain a list of\n";
	print "                folders identified by build #s. For example:\n";
	print "                    S:/build/release_candidate/artifacts/Fate/6.0.0.0\n";
	print "                    S:/build/release_candidate/artifacts/PolicyStudio/6.0.0.701\n";
	print "  varname       Variable name to be used in outpur file.\n";
	print "  verbose       Print detailed messages\n";
}

# -----------------------------------------------------------------------------
# Parse command line arguments

sub parseCommandLine()
{
	#
	# Parse arguments
	#

	# GetOptions() key specification:
	#	option			Given as --option or not at all (value set to 0 or 1)
	#	option!			May be given as --option or --nooption (value set to 0 or 1)
	#	option=s		Mandatory string parameter: --option=somestring
	#	option:s		Optional string parameter: --option or --option=somestring
	#	option=i		Mandatory integer parameter: --option=35
	#	option:i		Optional integer parameter: --option or --option=35
	#	option=f		Mandatory floating point parameter: --option=3.14
	#	option:f		Optional floating point parameter: --option or --option=3.14

	if (!GetOptions(
			'help|h' => \$g_help,				# --help or -h
			'startPath=s' => \$g_startPath,		# --startPath
			'outFile=s' => \$g_outFile,			# --outFile
			'varName=s' => \$g_varName,	        # --varName
			'verbose|v' => \$g_verbose			# --verbose
		))
	{
		return 0;
	}

	#
	# Help
	#

	if ($g_help == 1)
	{
		return 1;
	}

	# Check for errors
	if ($g_startPath eq '')
	{
		print STDERR "Missing start path\n";
		return 0;
	}

	if (! -d $g_startPath)
	{
		print STDERR "Start path does not exists - $g_startPath\n";
		return 0;
	}

	if ($g_outFile eq '')
	{
		print STDERR "Missing output file\n";
		return 0;
	}

	if ($g_varName eq '')
	{
		print STDERR "Missing variable name\n";
		return 0;
	}

	return 1;
}

# -----------------------------------------------------------------------------
# Find newest product build folder (largest build number)
#
# Description
#	The path passed in should look like this:
#	S:/build/release_candidate/artifacts/Fate/6.0.0.0
#
# Return Value:
#	Return a value > 0 if a folder matches [0-9]+ or
#	[0-9]+(PS|PC)_<branch name>. Return 0 if there is no match.
#	Build folder name is returned in the $refBuf parameter.

sub getNewestProductBuildFolder()
{
	my	($path, $refBuf) = @_;

	# Debug
	if ($g_verbose)
	{
		print "Start path = $path\n";
	}

	# Get largest build number
	my	$maxVal = 0;
	my	$folderName = '';

	opendir(HANDLE, "$path") || die "ERROR: Failed to open directory $path";

	while (my $entry = readdir(HANDLE))
	{
		# Debug
		if ($g_verbose)
		{
			print "  entry = $entry\n";
		}

		# Check file name
		if ($entry =~ /^(\d+)/)
		{
#			print "Build number = $1\n";

			if ($1 > $maxVal)
			{
				$maxVal = $1;
				$folderName = $entry;
			}
		}
	}

	closedir(HANDLE);

	# Debug
	if ($g_verbose)
	{
		print "Max value   = $maxVal\n";
		print "Folder name = $folderName\n";
	}

	# Return folder name
	if ($maxVal == 0)
	{
		return 0;
	}

	${$refBuf} = $folderName;
	return 1;
}

# -----------------------------------------------------------------------------
# Find product build artifact file (*-bin.zip)
#
# Description
#	The path passed in should look like this:
#	S:/build/release_candidate/artifacts/Fate/6.0.0.0/101
#
# Return Value:
#	Return a value > 0 if a file name matches *-bin.zip. Return 0 if there is no match.
#	Build artifact file name is returned in the $refBuf parameter.

sub getProductBuildArtifactFile()
{
	my	($path, $refBuf) = @_;

	# Debug
	if ($g_verbose)
	{
		print "Start path = $path\n";
	}

	# Get build artifact file
	my	$found = 0;
	my	$fileName = '';

	opendir(HANDLE, "$path") || die "ERROR: Failed to open directory $path";

	while (my $entry = readdir(HANDLE))
	{
		# Debug
		if ($g_verbose)
		{
			print "  entry = $entry\n";
		}

		# Check file name
		if ($entry =~ /(.*-bin\.zip)$/)
		{
#			print "Matched = $1\n";

			$found = 1;
			$fileName = $1;
			last;
		}

		if ($entry =~ /(.*-build\.zip)$/)
		{
#			print "Matched = $1\n";

			$found = 1;
			$fileName = $1;
			last;
		}
	}

	closedir(HANDLE);

	# Debug
	if ($g_verbose)
	{
		print "Found     = $found\n";
		print "File name = $fileName\n";
	}

	# Return file name
	if (!$found)
	{
		return 0;
	}

	${$refBuf} = $fileName;
	return 1;
}


#
# Main Program
#

# Parse command line arguements
my	$argCount = scalar(@ARGV);

if ($argCount < 1 || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")
{
	printUsage;
	exit 0;
}

if (!&parseCommandLine())
{
	exit 1;
}

if ($g_help == 1)
{
	printUsage;
	exit 0;
}

# Print parameters
print "Parameters:\n";
print "  g_startPath       = $g_startPath\n";
print "  g_outFile         = $g_outFile\n";
print "  g_varName         = $g_varName\n";

# Process command
my	$buildFolder = '';

if (!&getNewestProductBuildFolder($g_startPath, \$buildFolder))
{
	exit 1;
}

my	$path = $g_startPath . '/' . $buildFolder;
my	$fileName = '';

if (!&getProductBuildArtifactFile($path, \$fileName))
{
	exit 1;
}

my	$file = $path . '/' . $fileName;

print "Build artifact file = $file\n";


#
# Write output file
#

open FILE, ">$g_outFile" || die "Error opening output file $g_outFile\n";
binmode FILE;

print FILE <<"EOT";
#!/bin/bash

export $g_varName="$file"
EOT

close FILE;

exit 0;
