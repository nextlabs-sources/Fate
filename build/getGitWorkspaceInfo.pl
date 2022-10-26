#!perl
#
# DESCRIPTION
#	This script extracts Git workspace information from a Jenkins job config file.

use strict;
use warnings;

print "NextLabs Get Git Workspace Setting Script\n";


#
# Arguements
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: getGitWorkspaceInfo.pl <config file>\n";
	print "  config file  - Path to a Jenkins job config file.\n";
}

# Print usage
my	$argCount = scalar(@ARGV);

if (($argCount < 1 || $argCount > 1) || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")
{
	printUsage;
	exit 1;
}

# Collect parameters
my	$configFile = $ARGV[0];

# Print parameters
print "Parameters:\n";
print "  Config file       = $configFile\n";


#
# Check for errors
#

if (! -f $configFile)
{
	print "ERROR: $configFile does not exist\n";
	exit 1;
}


#
# Read config file
#

local $/ = undef;
open FILE, $configFile || die "Error opening config file $configFile";
my	$buf = <FILE>;
close FILE;

#print "\nSource Data:\n----------------\n$buf\n\n";

#
# Extract info
#

print "\nINFO: Parse config data\n";

if ($buf =~ /<hudson\.plugins\.git\.UserRemoteConfig>[^<]*<url>([^<]+)<\/url>[^<]*<\/hudson\.plugins\.git\.UserRemoteConfig>/s)
{
	print "[BUILD MANIFEST] Git Repo URL      : $1\n";
}

if ($buf =~ /<hudson\.plugins\.git\.BranchSpec>[^<]*<name>([^<]+)<\/name>[^<]*<\/hudson\.plugins\.git\.BranchSpec>/s)
{
	print "[BUILD MANIFEST] Git Branch        : $1\n";
}

my $gitHEADSHA = `git rev-parse --short --verify HEAD`;
print "[BUILD MANIFEST] Git Commit SHA    : $gitHEADSHA \n";

print "[BUILD MANIFEST] Git Label         : $ENV{BUILD_TAG}\n";

print "[BUILD MANIFEST] Jenkins Build URL       : $ENV{BUILD_URL}\n";
print "[BUILD MANIFEST] Jenkins Build Number    : $ENV{BUILD_NUMBER}\n";
print "[BUILD MANIFEST] Jenkins Build Id        : $ENV{BUILD_ID}\n";
print "[BUILD MANIFEST] Jenkins Workspace Path  : $ENV{WORKSPACE}\n";

if (defined $ENV{JENKINS_USER})
{
	print "[BUILD MANIFEST] Jenkins User            : $ENV{JENKINS_USER}\n";
}

if ($buf =~ /<hudson\.tasks\.Shell>[^<]*<command>([^<]+)<\/command>[^<]*<\/hudson\.tasks\.Shell>/s)
{
	my	$tmpBuf = $1;

	$tmpBuf =~ s/\n/; /g;

	print "[BUILD MANIFEST] Jenkins Build Script    : $tmpBuf\n";
}

exit 0;
