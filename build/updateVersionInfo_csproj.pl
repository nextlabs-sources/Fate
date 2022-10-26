#!perl
#
# DESCRIPTION
# This script updates assembly attributes in an existing Visual Studio project C# file
# using version information in Makefile.inc. It should be called from Pre-Build Event of
# a VS project.

use strict;
use warnings;

print "NextLabs Update C# File Assembly Attribute Script (.csproj Pre-Build Event)\n";


#
# Check for errors
#

# -----------------------------------------------------------------------------
# Dump parameters

sub dumpParameters
{
	print "ERROR: Wrong # of arguments (expect 2 or 3).\n";
	print "  See updateVersionInfo_csproj.pl and Makefile in //depot/Fate/main/build/ for more details.\n";
	print "Argv[0] = $ARGV[0]\n";
	print "Argv[1] = $ARGV[1]\n";
	print "Argv[2] = $ARGV[2]\n";
	print "Argv[3] = $ARGV[3]\n";
}

my	$argCount = scalar(@ARGV);

if ($argCount < 3 || $argCount > 4)
{
	dumpParameters;
	exit 1;
}


#
# Process parameters
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: updateVersionInfo_csproj.pl <C# file> <makefile> <build code>\n";
	print "         <architecture>\n";
	print "  architecture  - Targeted architecture (may be x86 or x64).\n";
	print "  build code    - Build code. Valid values are number, 'dev' and 'nightly'.\n";
	print "                  Specify a number for release build.\n";
	print "  makefile      - Project Makefile.inc relative to OutputPath of .csproj file.\n";
	print "  C# file       - C# file containing assembly attributes to be updated.\n";
	print "                  The path is relative to OutputPath of .csproj file.\n";
}

if (($argCount < 3 || $argCount > 4) || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")
{
	printUsage;
	exit 1;
}

# Collect parameters
my	$csFile = $ARGV[0];
my	$makefile = $ARGV[1];
my	$buildCode = $ARGV[2];
my	$architecture = $ARGV[3];
my	$showUpdatedFile = 0;

# Print parameters
print "Parameters:\n";
print "  C# File             = $csFile\n";
print "  Makefile            = $makefile\n";
print "  Build Code          = $buildCode\n";
print "  Architecture        = $architecture\n";
print "  Show Updated File   = $showUpdatedFile\n";


#
# Check for errors
#

if ( ! -e $csFile)
{
	print "ERROR: $csFile does not exist\n";
	exit 1;
}


#
# Get VERSION_* values from Makefile.inc
#

my	$product = "";
my	$majorVer = 0;
my	$minorVer = 0;

open FILE, $makefile || die "Error opening makefile $makefile";

while (! eof(FILE))
{
	my	$line = readline(*FILE);

	if ($line =~ /VERSION_PRODUCT\s*=\s*(.+)\n*/)
	{
		$product = $1;
	}
	elsif ($line =~ /VERSION_MAJOR\s*=\s*(\d+)/)
	{
		$majorVer = $1;
	}
	elsif ($line =~ /VERSION_MINOR\s*=\s*(\d+)/)
	{
		$minorVer = $1;
	}
}

close FILE;

print "Makefile Version Data:\n";
print "  Product Name        = $product\n";
print "  Major Version       = $majorVer\n";
print "  Minor Version       = $minorVer\n";


#
# Construct build number
#

my	$buildNum = "$ENV{BUILD_NUMBER}";

print "Build Number:\n";
print "  Build Number        = $buildNum\n";


#
# Read C# file
#

local $/ = undef;
open FILE, $csFile || die "Error opening C# file $csFile (read)";
my	$buf = <FILE>;
close FILE;

#print "\nSource Data:\n----------------\n$buf\n\n";


#
# Update version info
#


$product=~ s/\s*\n//g;
$product=~ s/\s*\r//g;
$buf =~ s/\[assembly:\s+AssemblyVersion\("\s*[^"]*"\)\]/\[assembly: AssemblyVersion(\"$majorVer.$minorVer.$buildNum\")\]/g;
$buf =~ s/\[assembly:\s+AssemblyCompany\("\s*[^"]*"\)\]/\[assembly: AssemblyCompany(\"NextLabs, Inc.\")\]/g;
$buf =~ s/\[assembly:\s+AssemblyCopyright\("\s*[^"]*"\)\]/\[assembly: AssemblyCopyright(\"Copyright (C) 2015 NextLabs, Inc. All rights reserved.\")\]/g;
$buf =~ s/\[assembly:\s+AssemblyProduct\("\s*[^"]*"\)\]/\[assembly: AssemblyProduct(\"$product\")\]/g;
$buf =~ s/\[assembly:\s+AssemblyFileVersion\("\s*[^"]*"\)\]/\[assembly: AssemblyFileVersion(\"$majorVer.$minorVer.$buildNum\")\]/g;

#print "\nUpdated Data:\n----------------\n$buf\n\n";


#
# Write C# file
#
# Notes: There is a problem with Cygwin + Perforce combination. If you run chmod from
# Cygwin, you will get an error. If you run "ls -al" you will see no permission and
# group is mkpasswd. If you check "if (-r myfile)", it will always return true. To
# work around this problem, we call Windows ATTRIB command directly.

my	$csFileDos = $csFile;

$csFileDos =~ s#/#\\#;

system("ATTRIB -R \"$csFileDos\"");

#if (chmod(0777, $csFile) == 0)
#{
#	die "### ERROR: Failed to chmod on file $csFile\n";
#}

open FILE, ">$csFile" || die "Error opening C# file $csFile (write)";
print FILE $buf;
close FILE;


#
# Print updated file
#

if ($showUpdatedFile)
{
	open FILE, $csFile || die "Error opening updated file $csFile\n";

	while (<FILE>)
	{
		print $_;
	}

	close FILE;
}

exit 0;
