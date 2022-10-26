#!perl
#
# DESCRIPTION
# This script updates Microsoft Windows driver .inf and .rc files.

use strict;
use warnings;

print "NextLabs Update Windows Driver Version Script (.inf and .rc)\n";


#
# Check for errors
#

my	$argCount = scalar(@ARGV);

if ($argCount != 7)
{
	print "### ERROR: Wrong # of arguments (expected 7).\n";
	print "Argv[0] = $ARGV[0]\n";
	print "Argv[1] = $ARGV[1]\n";
	print "Argv[2] = $ARGV[2]\n";
	print "Argv[3] = $ARGV[3]\n";
	print "Argv[4] = $ARGV[4]\n";
	print "Argv[5] = $ARGV[5]\n";
	print "Argv[6] = $ARGV[6]\n";
	exit 1;
}


#
# Process parameters
#

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: updateVersionInfo_driver.pl <inf-file> <rc-file> <date> <versionStr> <versionArch>\n";
	print "          						  <build-#> <product-name> <verArch>\n";
	print "  build-#         - Specify build # or string\n";
	print "  date            - Specify date (e.g. 01/12/2012)\n";
	print "  inf-file        - .inf file name\n";
	print "  product-name    - Specify product name\n";
	print "  rc-file         - Resource file name\n";
	print "  version         - Specify version string (e.g., 5.5.0.0)\n";
}

if ($argCount != 7 || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")
{
	printUsage;
	exit 1;
}

# Collect parameters
my	$infFile = $ARGV[0];
my	$rcFile = $ARGV[1];
my	$date = $ARGV[2];
my	$versionStr = $ARGV[3];
my	$versionArch = $ARGV[4];
my	$buildNum = $ARGV[5];
my	$prodName = $ARGV[6];

my	$fileVersion = "$versionStr.$buildNum";
my	$fileVersionStr = "$versionStr.$buildNum";
my	$fileDescStr = "$prodName ($versionArch)";
my	$prodVersion = "$versionStr.$buildNum";
my	$prodVersionStr = "$versionStr.$buildNum";

$fileVersion =~ s/\./,/g;

# Print parameters
print "Parameters:\n";
print "  infFile       = $infFile\n";
print "  rcFile        = $rcFile\n";
print "  date          = $date\n";
print "  versionStr    = $versionStr\n";
print "  versionArch   = $versionArch\n";
print "  buildNum      = $buildNum\n";
print "  prodName      = $prodName\n";
print "  fileVer       = $fileVersion\n";
print "  fileVerStr    = $fileVersionStr\n";
print "  fileDescStr   = $fileDescStr\n";
print "  prodVer       = $prodVersion\n";
print "  prodVerStr    = $prodVersionStr\n";


#
# Check for errors
#

if (! -e $infFile)
{
	print "ERROR: $infFile does not exist\n";
	exit 1;
}

if (! -e $rcFile)
{
	print "ERROR: $rcFile does not exist\n";
	exit 1;
}


#
# Read .inf file
#

local $/ = undef;
open FILE, $infFile || die "Error opening .inf file $infFile (read)";
my	$buf = <FILE>;
close FILE;

#print "\nSource Data:\n----------------\n$buf\n\n";


#
# Update .inf file
#

my	$driverVer = "$date;$versionStr.$buildNum";

#$buf =~ s/DriverVer\s*=.*?\n/DriverVer = "$driverVer"\n/;
$buf =~ s/ServiceDescription\s*=.*?\n/ServiceDescription = "$prodName"\n/;

#print "\nUpdated Data:\n----------------\n$buf\n\n";


#
# Write .inf file
#
# Notes: There is a problem with Cygwin + Perforce combination. If you run chmod from
# Cygwin, you will get an error. If you run "ls -al" you will see no permission and
# group is mkpasswd. If you check "if (-r myfile)", it will always return true. To
# work around this problem, we call Windows ATTRIB command directly.

my	$infFileDos = $infFile;

$infFileDos =~ s#/#\\#;

system("ATTRIB -R \"$infFileDos\"");

#if (chmod(0777, $infFile) == 0)
#{
#	die "### ERROR: Failed to chmod on file $infFile\n";
#}

open FILE, ">$infFile" || die "Error opening .inf file $infFile (write)";
print FILE $buf;
close FILE;


#
# Read .rc file
#

local $/ = undef;
open FILE, $rcFile || die "Error opening .rc file $rcFile (read)";
$buf = <FILE>;
close FILE;

#print "\nSource Data:\n----------------\n$buf\n\n";


#
# Update .rc file
#


$buf =~ s/#define\s+VER_FILEDESCRIPTION_STR\s+.*?\n/#define VER_FILEDESCRIPTION_STR "$fileDescStr"\n/;
$buf =~ s/#define\s+VER_FILEVERSION\s+.*?\n/#define VER_FILEVERSION $fileVersion\n/;
$buf =~ s/#define\s+VER_FILEVERSION_STR\s+.*?\n/#define VER_FILEVERSION_STR "$fileVersionStr"\n/;
$buf =~ s/#define\s+VER_PRODUCTNAME_STR\s+.*?\n/#define VER_PRODUCTNAME_STR "$prodName"\n/;
$buf =~ s/#define\s+VER_PRODUCTVERSION\s+.*?\n/#define VER_PRODUCTVERSION $prodVersion\n/;
$buf =~ s/#define\s+VER_PRODUCTVERSION_STR\s+.*?\n/#define VER_PRODUCTVERSION_STR "$prodVersionStr"\n/;

#print "\nUpdated Data:\n----------------\n$buf\n\n";

#
# Write .rc file
#
# Notes: There is a problem with Cygwin + Perforce combination. If you run chmod from
# Cygwin, you will get an error. If you run "ls -al" you will see no permission and
# group is mkpasswd. If you check "if (-r myfile)", it will always return true. To
# work around this problem, we call Windows ATTRIB command directly.

my	$rcFileDos = $rcFile;

$rcFileDos =~ s#/#\\#;

system("ATTRIB -R \"$rcFileDos\"");

#if (chmod(0777, $rcFile) == 0)
#{
#	die "### ERROR: Failed to chmod on file $rcFile\n";
#}

open FILE, ">$rcFile" || die "Error opening .rc file $rcFile (write)";
print FILE $buf;
close FILE;

exit 0;
