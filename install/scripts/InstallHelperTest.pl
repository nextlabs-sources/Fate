#!perl
#
# DESCRIPTION
# Script to test InstallHelper.pm functions.

use InstallHelper;

use strict;
use warnings;


my	$file = "C:/nightly/current/D_SiriusR2/install/common/nextlabs-clib-5.5.0.0/release_win_x64/ceeval.dll";

my	%verInfo = Win32API::Resources::GetFileVersion($file, my $return);

print "$_ : $verInfo{$_}\n" foreach sort keys %verInfo;

exit;


#
# Test InstallHelper::trim()
#

sub testTrim
{
	my	($text) = @_;

	my	$result = InstallHelper::trim($text);

	print "InstallHelper::trim() Test:\n";
	print "-------------------------\n";
	print "IN : [$text]\n";
	print "OUT: [$result]\n";
}


testTrim "  this one has spaces before and after   ";
testTrim "  \tthis one has spaces and tabs before and after   \t\t\t";
testTrim "\t\t\t  this one has spaces, tabs and linefeed before and after \n";


#
# Test InstallHelper::printArray()
#

my	@array1 = ("value 1", "value2", "value 3333");

print "InstallHelper::printArray() Test:\n";
print "-------------------------\n";

InstallHelper::printArray(\@array1, "TEST PRINT");


#
# Test InstallHelper::copyRequired()
#

print "InstallHelper::copyRequired() Test:\n";
print "-------------------------\n";

#InstallHelper::copyRequired("c:/tmp/fileNotExist.txt", "c:/tmp/something.txt");
InstallHelper::copyRequired("InstallHelperTest.pl", "c:/tmp/something.txt");

#InstallHelper::copyVersionRequired("c:/tmp/fileNotExist.txt", "c:/tmp/something.txt", "5.5.0");
#InstallHelper::copyVersionRequired("InstallHelperTest.pl", "c:/tmp/something.txt", "5.0.0");
#InstallHelper::copyVersionRequired("InstallHelperTest.pl", "c:/tmp/something.txt", "5.5.0");


#
# Test InstallHelper::constructIsmFile()
#

print "InstallHelper::constructIsmFile() Test:\n";
print "-------------------------\n";

#my	($outDirName, $msiFileName32, $msiFileName64) = InstallHelper::makeInstallerNames_Release("nextlabs-clib", "5.5.0");
my	($outDirName, $msiFileName32, $msiFileName64) = InstallHelper::makeInstallerNames_PreRelease("nextlabs-clib", "5.5.0", "100", "nightly");

#InstallHelper::constructIsmFile("c:/tmp/templateFileNotExist.txt", "c:/tmp/something.txt", $msiFileName32, $msiFileName64, "5.5.0", "100");
InstallHelper::constructIsmFile("C:/nightly/current/D_SiriusR2/install/common/src/installer/main/CommonLibraries.ism", "test_install.ism", $msiFileName32, $msiFileName64, "5.5.0", "100");
