#!perl
#
# DESCRIPTION
#	This script checks build log file for serious warnings. It returns 0 if no serious
#	warning is found. Otherwise, it returns # of errors.

use strict;
use warnings;

print "NextLabs Check Build Warning Script\n";


#
# Arguements
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: checkBuildLogWarning.pl <log file>\n";
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

open FILE, $logFile || die "Error opening makefile $logFile\n";

loop: while (<FILE>)
{
#		print "Line: $_\n";

	if (/warning/i)
	{
#		print "INSPECT: $_";

		# Exclude compilation warnings
		if (/warning LNK4099|warning C4995|warning LNK4070|warning C4101|warning C4251|warning C4101/)
		{
			next loop;
		}


		if (/Command line warning D9040|Command line warning D9035|-analyze -W4|link.exe -NOLOGO/)
		{
			next loop;
		}

		if (/This function or variable may be unsafe.|local variable is initialized but not referenced|hides declaration of the same name in outer scope|Return value ignored:|warning C4005: 'WIN32_LEAN_AND_MEAN' : macro redefinition|WARNING: TARGETENVOS undefined.|warning C4005: '_WIN32_WINNT' : macro redefinition|has been changed to conform with the ISO C standard|The POSIX name for this item is deprecated./)
		{
			next loop;
		}

		# Exclude InstallShield warnings
		if (/0 error\(s\),|ISDEV : warning -4354:|ISDEV : warning -7143:|: warning W7503:/)
		{
			next loop;
		}

		# Add to collection
		push @msgList, $_;
	}
}

close FILE;

# Print messages
my	$count = scalar(@msgList);

foreach my $msg (@msgList)
{
	print "WARNING  : $msg";
}

print "INFO:  Found $count serious warning line(s)\n";

exit $count;
