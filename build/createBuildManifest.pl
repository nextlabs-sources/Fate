#!perl
#
# DESCRIPTION
#	This script generates a build manifest based on information in a build log file.

use strict;
use warnings;

print "NextLabs Create Build Manifest Script\n";


#
# Arguements
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: createBuildManifest.pl <log file>\n";
	print "  Log file  - Path to a log file.\n";
}

# Print usage
my	$argCount = scalar(@ARGV);

if (($argCount < 1 || $argCount > 1) || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")
{
	printUsage;
	exit 1;
}

# Collect parameters
my	$logFile = $ARGV[0];

# Print parameters
print "Parameters:\n";
print "  Log File       = $logFile\n";


#
# Check for errors
#

if (! -f $logFile)
{
	print "ERROR: $logFile does not exist\n";
	exit 1;
}


#
# Main Program
#

print "\nINFO: Parse log file\n";

# Parse log file
my	@msgList = ();

open FILE, $logFile || die "Error opening log file $logFile\n";

loop: while (<FILE>)
{
#		print "Line: $_\n";

	if (/\[BUILD MANIFEST\] (.*)/)
	{
		push @msgList, $1;
	}
}

close FILE;

# Print messages
my	$count = scalar(@msgList);

foreach my $msg (@msgList)
{
	print "$msg\n";
}

exit 0;
