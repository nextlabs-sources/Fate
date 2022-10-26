#!perl
#
# DESCRIPTION
#	This script checks build log file for build error. It returns 0 if no error
#	is found. Otherwise, it returns # of errors.

use strict;
use warnings;

print "NextLabs Check Build Error Script\n";


#
# Arguements
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: checkBuildLog.pl <log file>\n";
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

	if (/error/i)
	{
#		print "INSPECT: $_";

		# Exclude commandline parameters
		if (/-error all/)
		{
			next loop;
		}

		# Exclude warnings
		if (/warning C6283: 'errorAsString'/)
		{
			next loop;
		}

		# Exclude Java messages
		if (/\[echo\] INFO: |\[javac\] \[loading java\\lang\\/)
		{
			next loop;
		}

		# Exclude InstallShield output messages
		if (/- 0 error\(s\),|\w table successfully built|Dialog [^ ]+ for language\s+built|ERROR\s+\:\s+ISDEV\s+\:/)
		{
			next loop;
		}

		# Exclude Prefast error
		if (/warning C4189: 'Error'/)
		{
			next loop;
		}

		# Exclude SignTool error
		if (/SignTool Error: The specified timestamp server either could not be reached|SignTool Warning: Signing succeeded, but an error occurred while attempting to/)
		{
			next loop;
		}

		# Exclude output from drvBuild.bat
		if (/^Errors:\s*$/)
		{
			next loop;
		}

		# Exclude content of BuildLog.htm
		if (/\/errorReport:queue|\/ERRORREPORT:QUEUE|goto VCReportError|:VCReportError|echo Project : error PRJ0019:/)
		{
			next loop;
		}

		# Exclude build.log errors
		if (/\/errorreport:prompt|Compile complete -- \d+ errors, \d+ warnings/)
		{
			next loop;
		}

		# Exclude output from Java build_*.xml
		if (/INFO: Sanity check completed \(no error\)|Error.class/)
		{
			next loop;
		}

		# Exclude output from this script
		if (/NextLabs Check Build Error Script|INFO:  Found \d+ error line\(s\)/)
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
	print "ERROR  : $msg";
}

print "INFO:  Found $count error line(s)\n";

exit $count;
