#!perl
#
# DESCRIPTION
# This script updates VERSIONINFO section in an existing Visual Studio project resouce file.
# It should be called from a make file.

use strict;
use warnings;

print "NextLabs Update Resource File VersionInfo Script (Make)\n";


#
# Check for errors
#

# -----------------------------------------------------------------------------
# Dump parameters

sub dumpParameters
{
	print "### ERROR: Wrong # of arguments (expected 8).\n";
	print "  See updateVersionInfo_make.pl and Makefile in //depot/dev/D_SiriusR2/build/ for more details.\n";
	print "Argv[0] = $ARGV[0]\n";
	print "Argv[1] = $ARGV[1]\n";
	print "Argv[2] = $ARGV[2]\n";
	print "Argv[3] = $ARGV[3]\n";
	print "Argv[4] = $ARGV[4]\n";
	print "Argv[5] = $ARGV[5]\n";
	print "Argv[6] = $ARGV[6]\n";
	print "Argv[7] = $ARGV[7]\n";
}

my	$argCount = scalar(@ARGV);

if ($argCount != 8)
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
	print "usage: updateVersionInfo_make.pl <resource file> <major version> <minor version>\n";
	print "         <maintenance version> <patch version> <build #> <product name>\n";
	print "         <architecture>\n";
	print "  architecture    - Targeted architecture (may be x86 or x64).\n";
	print "  build #         - Build number. Valid values are number or a quoted string.\n";
	print "                    Specify a number for release build.\n";
	print "  maintenance version - Maintenance version number (third of 0.0.0.0).\n";
	print "  major version   - Major version number (first of 0.0.0.0).\n";
	print "  minor version   - Minor version number (second of 0.0.0.0).\n";
	print "  patch version   - Patch version number (forth of 0.0.0.0).\n";
	print "  product name    - Name of product this module belongs to.\n";
	print "  resource file   - Resource file containing VERSIONINFO section to be updated.\n";
	print "                    The path is relative to location of .vsproj file.\n";
}

if ($argCount != 8 || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")
{
	printUsage;
	exit 1;
}

# Collect parameters
my	$resourceFile = $ARGV[0];
my	$majorVer = $ARGV[1];
my	$minorVer = $ARGV[2];
my	$maintenanceVer = $ARGV[3];
my	$patchVer = $ARGV[4];
my	$buildNum = $ARGV[5];
my	$product = $ARGV[6];
my	$architecture = $ARGV[7];
my	$showUpdatedFile = 0;

# Print parameters
print "Parameters:\n";
print "  Resource File       = $resourceFile\n";
print "  Product Name        = $product\n";
print "  Major Version       = $majorVer\n";
print "  Minor Version       = $minorVer\n";
print "  Maintenance Version = $maintenanceVer\n";
print "  Patch Version       = $patchVer\n";
print "  Build Number        = $buildNum\n";
print "  Architecture        = $architecture\n";
print "  Show Updated File   = $showUpdatedFile\n";


#
# Check for errors
#

if (! -e $resourceFile)
{
	print "ERROR: $resourceFile does not exist\n";
	exit 1;
}

#
# Read resource file
#

local $/ = undef;
open FILE, $resourceFile || die "Error opening resource file $resourceFile (read)";
my	$buf = <FILE>;
close FILE;

#print "\nSource Data:\n----------------\n$buf\n\n";


#
# Update version info
#
$buf =~ s#FILEVERSION\s+\d+,\s*\d+,\s*\d+,\s*\d+#FILEVERSION $majorVer, $minorVer, $buildNum#g;
$buf =~ s#FILEVERSION\s+\d+,\s*\d+,\s*\d+#FILEVERSION $majorVer, $minorVer, $buildNum#g;
$buf =~ s#PRODUCTVERSION\s+\d+,\s*\d+,\s*\d+,\s*\d+#PRODUCTVERSION $majorVer, $minorVer, $buildNum#g;
$buf =~ s#PRODUCTVERSION\s+\d+,\s*\d+,\s*\d+#PRODUCTVERSION $majorVer, $minorVer, $buildNum#g;
$buf =~ s#VALUE\s+"CompanyName"\s*,\s*"[^"]*"#VALUE "CompanyName", "NextLabs, Inc."#g;
$buf =~ s#VALUE\s+"FileDescription"\s*,\s*"[^"]*"#VALUE "FileDescription", "$product ($architecture)"#g;
#$buf =~ s#VALUE\s+"FileVersion"\s*,\s*"[^"]*"#VALUE "FileVersion", "$majorVer.$minorVer.$maintenanceVer.$patchVer ($buildNum)"#g;
$buf =~ s#VALUE\s+"FileVersion"\s*,\s*"[^"]*"#VALUE "FileVersion", "$majorVer.$minorVer.$buildNum"#g;
$buf =~ s#VALUE\s+"LegalCopyright"\s*,\s*"[^"]*"#VALUE "LegalCopyright", "Copyright (C) 2019 NextLabs, Inc. All rights reserved."#g;
$buf =~ s#VALUE\s+"ProductName"\s*,\s*"[^"]*"#VALUE "ProductName", "$product"#g;
#$buf =~ s#VALUE\s+"ProductVersion"\s*,\s*"[^"]*"#VALUE "ProductVersion", "$majorVer.$minorVer.$maintenanceVer.$patchVer ($buildNum)"#g;
$buf =~ s#VALUE\s+"ProductVersion"\s*,\s*"[^"]*"#VALUE "ProductVersion", "$majorVer.$minorVer.$buildNum"#g;

#print "\nUpdated Data:\n----------------\n$buf\n\n";


#
# Write resource file
#
# Notes: There is a problem with Cygwin + Perforce combination. If you run chmod from
# Cygwin, you will get an error. If you run "ls -al" you will see no permission and
# group is mkpasswd. If you check "if (-r myfile)", it will always return true. To
# work around this problem, we call Windows ATTRIB command directly.

my	$resourceFileDos = $resourceFile;

$resourceFileDos =~ s#/#\\#;

system("ATTRIB -R \"$resourceFileDos\"");

#if (chmod(0777, $resourceFile) == 0)
#{
#	die "### ERROR: Failed to chmod on file $resourceFile\n";
#}

open FILE, ">$resourceFile" || die "Error opening resource file $resourceFile (write)";
print FILE $buf;
close FILE;


#
# Print updated file
#

if ($showUpdatedFile)
{
	open FILE, $resourceFile || die "Error opening updated file $resourceFile\n";

	while (<FILE>)
	{
		print $_;
	}

	close FILE;
}

exit 0;
