#!perl
#
# DESCRIPTION
#	This script summarize build warnings in a log file.

use strict;
use warnings;

print "NextLabs Build Warning Analysis Script\n";


#
# Arguements
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: summarizeWarnings.pl <log file>\n";
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
my	%compileCodeMap = ();
my	%linkCodeMap = ();
my	$folderCount = 0;
my	$warningCount = 0;

open FILE, $logFile || die "Error opening $logFile\n";

loop: while (<FILE>)
{
#		print "Line: $_\n";

	if (/: warning [CD]|: warning LNK|make\[\d+\]: Entering directory/)
	{
		# Count lines
		if (/make\[\d+\]: Entering directory/)
		{
			print "\n------------------------------- \n";
			print "FOLDER: $_\n";

			$folderCount++;
		}
		elsif (/: warning [CD]/)
		{
			if (/: warning ([CD]\d+)/)
			{
#				print "CODE: $1\n";

				if (defined $compileCodeMap{$1})
				{
					$compileCodeMap{$1}++;
				}
				else
				{
					$compileCodeMap{$1} = 1;
				}
			}

			print "$_";
			$warningCount++;
		}
		else
		{
			if (/: warning (LNK\d+)/)
			{
#				print "CODE: $1\n";

				if (defined $linkCodeMap{$1})
				{
					$linkCodeMap{$1}++;
				}
				else
				{
					$linkCodeMap{$1} = 1;
				}
			}

			print "$_";
			$warningCount++;
		}
	}
}

close FILE;

# Print warnings
print "\n------------------------------- \n";
print "Compiler Warnings\n\n";

my	$compileCount = 0;

foreach my $key (sort keys %compileCodeMap)
{
	my	$count = $compileCodeMap{$key};

    print "$key: $count\n";

    $compileCount += $count;
}

print "\nTotal $compileCount compiler warning(s)\n";

print "\n------------------------------- \n";
print "Linker Warnings\n\n";

my	$linkCount = 0;

foreach my $key (sort keys %linkCodeMap)
{
	my	$count = $linkCodeMap{$key};

    print "$key: $count\n";

    $linkCount += $count;
}

print "\nTotal $linkCount linker warning(s)\n";

# Print summary
print "\nINFO: $folderCount folder(s), $warningCount warning(s)\n";

exit $compileCount + $linkCount;
