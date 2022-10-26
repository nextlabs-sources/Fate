#!perl
#
# DESCRIPTION
# This script correlates all Makefiles with all corresponding compiled binaries.

use strict;
use warnings;

use File::Find;
use File::Basename;
use HTML::Entities;


#
# Arguements
#

print "NextLabs Build Inventory Correlator\n";

# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: correlateBuildInventory.pl <root dir> <config file> <report file> [nodebug]\n";
	print "  Config file  - A text file specifies what makefile to ignore and special/\n";
	print "                 makefile to binaries mapping.\n";
	print "  nodebug      - Do not include debug binaries in the report (default include debug).\n";
	print "  report file  - An HTML report file (e.g., c:/nightly/current/D_SiriusR2/\n";
	print "                 buildInventory_cdc_20110101.html).\n";
	print "  root dir     - Source tree root direcrory (e.g., c:/nightly/current/D_SiriusR2).\n";
	print "                 This script will find all Makefiles in the subdirectories and\n";
	print "                 binaries in <root dir>/bin/<debug|release>_win_<x86|x64>.\n";
}

# Print usage
if (($#ARGV < 2 || $#ARGV > 3) || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")
{
	printUsage;
	exit 1;
}

# Collect parameters
my	$rootDir = $ARGV[0];
my	$configFile = $ARGV[1];
my	$reportFile = $ARGV[2];
my	$showDebug = 1;

if (defined $ARGV[3] && $ARGV[3] eq "nodebug")
{
	$showDebug = 0;
}

my	$relX86Dir = $rootDir . "/bin/release_win_x86";
my	$debugX86Dir = $rootDir . "/bin/debug_win_x86";
my	$relX64Dir = $rootDir . "/bin/release_win_x64";
my	$debugX64Dir = $rootDir . "/bin/debug_win_x64";
my	$relDotNetDir = $rootDir . "/bin/release_dotnet";
my	$debugDotNetDir = $rootDir . "/bin/debug_dotnet";

# Print parameters
print "Parameters:\n";
print "  Root directory      = $rootDir\n";
print "  Config File         = $configFile\n";
print "  Report File         = $reportFile\n";
print "  Show Debug          = $showDebug\n";


#
# Check for errors
#

if (! -d $rootDir)
{
	print "ERROR: $rootDir does not exist\n";
	exit 1;
}

if (! -f $configFile)
{
	print "ERROR: $configFile does not exist\n";
	exit 1;
}


#
# General utilities
#

# -----------------------------------------------------------------------------
# Remove leading and trailing whitespaces in a string

sub trim
{
	my	($buf) = @_;

	$buf =~ s/^\s+|\s+$//g;

	return $buf;
}

# -----------------------------------------------------------------------------
# Print array

sub printArray
{
	my	($refArray, $title) = @_;
	my	$count = scalar(@$refArray);

	print "\n$title (total $count):\n";

	foreach (@$refArray)
	{
		print "  $_\n";
	}
}


#
# Read config file
#

# -----------------------------------------------------------------------------
# Read config file into a

sub readConfigFile
{
	my	($file, $refIgnoreList, $refTargetMap, $refSubtargetMap) = @_;
	my	$line = 1;

	open FILE, $file || die "Error opening config file $file\n";

	while (<FILE>)
	{
		my	$buf = trim($_);

#		print "Config: $buf\n";

		$line++;

		# Skip comment
		if ($buf =~ /^#|^$/)
		{
			next;
		}

		# Parse line
		my	@tokenList = split(/;/, $buf);
		my	$count = scalar(@tokenList);

		if ($tokenList[0] =~ /ignore/i)
		{
			if ($count == 2)
			{
				my	$file = trim($tokenList[1]);

				push @$refIgnoreList, $file;
			}
			else
			{
				print "ERROR: Line $line invalid (expected 2 tokens, got $count): $buf\n";
			}
		}
		elsif ($tokenList[0] =~ /map/i)
		{
			# Fix trailing ';' problem
			if ($count == 3 && $buf =~ /;$/)
			{
				$tokenList[3] = "";
				$count = 4;
			}

			if ($count == 4)
			{
				my	$file = trim($tokenList[1]);
				my	$targetStr = trim($tokenList[2]);
				my	$subtargetStr = trim($tokenList[3]);

				if ($targetStr ne "")
				{
					$$refTargetMap{$file} = [ split(/,/, $targetStr) ];
				}

				if ($subtargetStr ne "")
				{
					$$refSubtargetMap{$file} = [ split(/,/, $subtargetStr) ];
				}
			}
			else
			{
				print "ERROR: Line $line invalid (expected 4 tokens, got $count): $buf\n";
			}
		}
		else
		{
			print "ERROR: invalid config at line $line - $buf\n"
		}
	}

	close FILE;
}

# -----------------------------------------------------------------------------
# Print target mappings

sub printTargetMappings
{
	my	($refTargetMap, $title) = @_;
	my	$count = scalar(keys %$refTargetMap);

	print "\n$title (total $count):\n";

	foreach my $file (keys %$refTargetMap)
	{
		print "  $file => ";

		for my $val (@{$$refTargetMap{$file}})
		{
			print "$val ";
		}

		print "\n";
	}
}

# Process config file
my	@ignoreList = ();
my	%targetMap = ();
my	%subtargetMap = ();

print "Reading config file\n";

readConfigFile $configFile, \@ignoreList, \%targetMap, \%subtargetMap;

printArray \@ignoreList, "Makefiles to Ignore";
printTargetMappings \%targetMap, "Map Makefiles to Targets";
printTargetMappings \%subtargetMap, "Map Makefiles to Subtargets";


#
# Locate makefiles
#

print "Locating makefiles\n";

# File::find() compare function
my	@modDirList = ();

# -----------------------------------------------------------------------------
# Find::File compare function

sub findFileCompareHelper
{
	my ($filename, $dir, $suffix) = fileparse($_);

	if ($filename =~ /^Makefile$/i)
	{
#		print "$_\n  Parsed: Dir=$dir, File=$filename, Suffix=$suffix\n";

		push @modDirList, $dir;
	}
}

# -----------------------------------------------------------------------------
# Remove ignored makefiles

sub filterMakefiles
{
	my	($refMakefileList, $prefix, $refIgnoreList, $refAcceptedList) = @_;

	for my $file (@$refMakefileList)
	{
		# Trim path
		my	$relPath = $file;

		if ($file =~ /^$prefix[\/]$/i)
		{
			$relPath = "/";
		}
		else
		{
			$relPath =~ s#^$prefix/##i;
		}

		# Ignore this makefile
		my	@list = grep(/^$relPath$/i, @$refIgnoreList);

		if (scalar(@list) > 0)
		{
			print "Ignoring $file\n";
			next;
		}

		push @$refAcceptedList, $file;
	}
}

# Find makefiles
# $rootDir = "c:/nightly/current/D_SiriusR2/prod/common";			# For debug only
# $rootDir = "c:/nightly/current/D_SiriusR2/prod/pep/endpoint/wde";	# For debug only

find({
		wanted => \&findFileCompareHelper,
		no_chdir => 1,
		bydepth => 1
	},
	$rootDir);

my	@makefileList = sort @modDirList;
my	$makefileCount = $#makefileList + 1;

printArray \@makefileList, "Found Makefiles";

# Remove ignored makefiles
my	@acceptedList = ();

filterMakefiles \@makefileList, $rootDir, \@ignoreList, \@acceptedList;

my	$acceptedCount = $#acceptedList + 1;
my	$ignoredCount = $makefileCount - $acceptedCount;

printArray \@acceptedList, "Accepted Makefiles";


#
# Extract target and subtarget
#
# Sample:
#	SUBTARGET = nlscekeeper

print "Extract make target and subtarget\n";

# -----------------------------------------------------------------------------
# Function to extract target and subtarget from a Makefile.inc

sub extractMakeTargetAndSubtarget
{
	my	$dir = $_[0];

#	print "Processing $dir\n";

	# No Makefile.inc file
	my	$file = $dir . "/Makefile.inc";

	if ( ! -f $file )
	{
		print "No Makefile.inc in $dir\n";
		return ("", "", "");
	}

	# Parse Makefile.inc
	my	$targetBuf = "";
	my	$subtargetBuf = "";
	my	$subtargetStatic = 0;

	open FILE, $file || die "Error opening makefile $file\n";

	while (<FILE>)
	{
#		print "Line: $_\n";

		# EXE or DLL name
		if (/^\s*TARGET\s*=(.*)/)
		{
			if ($targetBuf ne "")
			{
				$targetBuf .= ", ";
			}

			$targetBuf .= trim($1);
		}
		elsif (/^\s*SUBTARGET\s*=(.*)/)
		{
			if ($subtargetBuf ne "")
			{
				$subtargetBuf .= ", ";
			}

			$subtargetBuf .= trim($1);
		}
		elsif (/^\s*CSTARGET\s*=(.*)/)
		{
			if ($targetBuf ne "")
			{
				$targetBuf .= ", ";
			}

			$targetBuf .= trim($1);
		}
		elsif (/^\s*CSSUBTARGET\s*=(.*)/)
		{
			if ($subtargetBuf ne "")
			{
				$subtargetBuf .= ", ";
			}

			$subtargetBuf .= trim($1);
		}

		# Static library
		if (/^\s*SUBTARGET_STATIC\s*=\s*[Yy][Ee][Ss]/)
		{
			$subtargetStatic = 1;
		}
	}

	close FILE;

#	print "File				= $file\n";
#	print "Target			= $targetBuf\n";
#	print "Subtarget		= $subtargetBuf\n";
#	print "Subtarget static = $subtargetBuf\n";

	# No target or subtarget
	if ($targetBuf eq "" && $subtargetBuf eq "")
	{
		print "No target or subtarget in $file\n";
	}

	# Static library
	if ($subtargetStatic)
	{
		print "Static subtarget in $file\n";
	}

	return ($targetBuf, $subtargetBuf, $subtargetStatic);
}

# -----------------------------------------------------------------------------
# Create file table

sub createFileTable
{
	my	$refAcceptedList = $_[0];
	my	$refFileTable = $_[1];
	my	$targetCount = 0;
	my	$subtargetCount = 0;
	my	$subtargetStaticCount = 0;

	for my $dir (@$refAcceptedList)
	{
		my	($targetStr, $subtargetStr, $subtargetStatic) = extractMakeTargetAndSubtarget($dir);

#		print "Dir				= $dir\n";
#		print "Target			= $targetStr\n";
#		print "Subtarget		= $subtargetStr\n";
#		print "Subtarget static = $subtargetStatic\n";

		if ($targetStr ne "")
		{
			$targetCount++;
		}

		if ($subtargetStr ne "")
		{
			$subtargetCount++;
		}

		if ($subtargetStatic)
		{
			$subtargetStaticCount++;
		}

		if (!$subtargetStatic)
		{
			push @$refFileTable, {dir => $dir, target => $targetStr, subtarget => $subtargetStr};
		}
	}

	return ($targetCount, $subtargetCount, $subtargetStaticCount);
}

# -----------------------------------------------------------------------------
# Print file table (an array of hashes)

sub printFileTable
{
	my	($refFileTable, $title) = @_;
	my	$count = scalar(@$refFileTable);

	print "\n$title (total $count)):\n";

	for my $ref (@$refFileTable)
	{
		print "  dir => $ref->{dir}\n";

		for my $name ( keys %$ref )
		{
			next unless $name !~ /^dir$/;

			print "    $name => $ref->{$name}\n";
		}
	}
}

# Process Makefile.inc
my	@fileTable = ();
my	$targetCount = 0;
my	$subtargetCount = 0;
my	$subtargetStaticCount = 0;

($targetCount, $subtargetCount, $subtargetStaticCount) = createFileTable \@acceptedList, \@fileTable;

printFileTable \@fileTable, "Found Targets and Subtargets";


#
# Locate binaries
#

# -----------------------------------------------------------------------------
# Find all files in list that match names in name list

sub matchFilesInList
{
	my	($refFileList, $refNameList, $dir) = @_;

#	printArray $refNameList, "Match Name List";

	# No name
	if (scalar(@$refNameList) == 0)
	{
		print "WARNING: No name to match with $dir.\n";
		return "";
	}

	# Construct grep pattern
	my	$pattern = "^" . $refNameList->[0];

	for (my $index=1; $index<scalar(@$refNameList); $index++)
	{
		$pattern .= "|^" . $refNameList->[$index];
	}

#	print "Pattern   = $pattern\n";

	# Grep files
	my	@list = grep(/$pattern/i, @$refFileList);

#	print "Matched   = @list\n";

	return join(", ", @list);
}

# Get files in bin directories
my	@relX86FileList = ();
my	@debugX86FileList = ();
my	@relX64FileList = ();
my	@debugX64FileList = ();
my	@relDotNetFileList = ();
my	@debugDotNetFileList = ();

if (-d $relX86Dir)
{
	opendir DIR, $relX86Dir || die "Error opening bin directory $relX86Dir\n";
	@relX86FileList = grep(/\.(dll|exe)$/, readdir(DIR));
	close DIR;
}

if ($showDebug && -d $debugX86Dir)
{
	opendir DIR, $debugX86Dir || die "Error opening bin directory $debugX86Dir\n";
	@debugX86FileList = grep(/\.(dll|exe)$/, readdir(DIR));
	close DIR;
}

if (-d $relX64Dir)
{
	opendir DIR, $relX64Dir || die "Error opening bin directory $relX64Dir\n";
	@relX64FileList = grep(/\.(dll|exe)$/, readdir(DIR));
	close DIR;
}

if ($showDebug && -d $debugX64Dir)
{
	opendir DIR, $debugX64Dir || die "Error opening bin directory $debugX64Dir\n";
	@debugX64FileList = grep(/\.(dll|exe)$/, readdir(DIR));
	close DIR;
}

if (-d $relDotNetDir)
{
	opendir DIR, $relDotNetDir || die "Error opening bin directory $relDotNetDir\n";
	@relDotNetFileList = grep(/\.(dll|exe)$/, readdir(DIR));
	close DIR;
}

if (-d $debugDotNetDir)
{
	opendir DIR, $debugDotNetDir || die "Error opening bin directory $debugDotNetDir\n";
	@debugDotNetFileList = grep(/\.(dll|exe)$/, readdir(DIR));
	close DIR;
}

printArray \@relX86FileList, "Release x86 Binaries";
printArray \@debugX86FileList, "Debug x86 Binaries";
printArray \@relX64FileList, "Release x64 Binaries";
printArray \@debugX64FileList, "Debug x64 Binaries";
printArray \@relDotNetFileList, "Release .NET Binaries";
printArray \@debugDotNetFileList, "Debug .NET Binaries";

# Match files bin directories to targets and subtargets
my	$mappedCount = 0;

for my $ref (@fileTable)
{
	# Trim path
	my	$dir = $ref->{dir};

	if ($dir =~ /^$rootDir[\/]$/i)
	{
		$dir = "/";
	}
	else
	{
		$dir =~ s#^$rootDir/##i;
	}

	# Construct name list
	my	@nameList = ();

	if (exists $targetMap{$dir})
	{
		push @nameList, @{$targetMap{$dir}};

		$mappedCount++;
	}
	elsif ($ref->{target} ne "")
	{
		push @nameList, $ref->{target};
	}

	if (exists $subtargetMap{$dir})
	{
		push @nameList, @{$subtargetMap{$dir}};

		$mappedCount++;
	}
	elsif ($ref->{subtarget} ne "")
	{
		push @nameList, $ref->{subtarget};
	}


	# No name
	if (scalar(@nameList) == 0)
	{
		$ref->{relX86} = "";
		$ref->{debugX86} = "";
		$ref->{relX64} = "";
		$ref->{debugX64} = "";
		$ref->{relDotNet} = "";
		$ref->{debugDotNet} = "";
		next;
	}

	# Find files
	$ref->{relX86} = matchFilesInList(\@relX86FileList, \@nameList, $dir);
	$ref->{debugX86} = matchFilesInList(\@debugX86FileList, \@nameList, $dir);
	$ref->{relX64} = matchFilesInList(\@relX64FileList, \@nameList, $dir);
	$ref->{debugX64} = matchFilesInList(\@debugX64FileList, \@nameList, $dir);
	$ref->{relDotNet} = matchFilesInList(\@relDotNetFileList, \@nameList, $dir);
	$ref->{debugDotNet} = matchFilesInList(\@debugDotNetFileList, \@nameList, $dir);
}


#
# Write report
#

open FILE, ">$reportFile" || die "Error opening report file $reportFile\n";

# Header
print FILE <<"EOT";
<html>
<head>
<style>
body
{
	font-family: "Book Antiqua";
}
.title
{
	font-size: 18pt; font-weight: bold; color: rgb(83, 61, 168);
}
.sect
{
	margin: 20px 0 5px 0; font-size: 14pt; font-weight: bold; color: rgb(126, 107, 201);
}
table.info
{
	margin: 0; padding: 0; border-collapse: collapse;
}
table.info th
{
	padding: 2px 10px 2px 5px; border: solid 1px #888888; font-weight: bold;
}
table.info td.gray
{
	padding: 2px 10px 2px 5px; border: solid 1px #888888; color: #a0a0a0;
}
table.info td.na
{
	padding: 2px 10px 2px 5px; border: solid 1px #888888; color: #c0c0c0;
}
table.info td.success
{
	padding: 2px 10px 2px 5px; border: solid 1px #888888; background: #70e870;
}
table.info td.failed
{
	padding: 2px 10px 2px 5px; border: solid 1px #888888; background: #ffc0c0;
}
table.info td
{
	padding: 2px 10px 2px 5px; border: solid 1px #888888;
}
.note
{
	margin: 8px 0; font-size: smaller;
}
.error
{
	margin: 10px 0; padding: 10px; background: #fcfcfc; border: solid 1px #888888;
}
</style>
</head>
<body>
<div class="title">NextLabs Build Inventory</div>
<div class="sect">$rootDir</div>
<p>Found $makefileCount makefile(s), $acceptedCount accepted, $ignoredCount ignored.
Makefiles contain $targetCount target(s), $subtargetCount subtarget(s), $subtargetStaticCount subtarget(s) are static libraries.
$mappedCount makefile(s) are mapped to new target(s) or subtarget(s).</p>
<table class="info">
EOT

if ($showDebug)
{
	print FILE <<"EOT";
<tr>
	<th class="c1"></th>
	<th class="c2">Module</th>
	<th class="c3">Target</th>
	<th class="c4">Subtarget</th>
	<th class="c5">x86 Release</th>
	<th class="c6">x86 Debug</th>
	<th class="c7">x64 Release</th>
	<th class="c8">x64 Debug</th>
	<th class="c9">.NET Release</th>
	<th class="c10">.NET Debug</th>
</tr>
EOT
}
else
{
	print FILE <<"EOT";
<tr>
	<th class="c1"></th>
	<th class="c2">Module</th>
	<th class="c3">Target</th>
	<th class="c4">Subtarget</th>
	<th class="c5">x86 Release</th>
	<th class="c6">x64 Release</th>
	<th class="c7">.NET Release</th>
</tr>
EOT
}

# Listing
my	$index = 1;

for my $ref (@fileTable)
{
	# Prepare module name
	my	$dir = $ref->{dir};

	$dir =~ s#^$rootDir\/##;
	$dir =~ s#\/$##;

	# Prepare HTML class
	my	$classIgnore = "";
	my	$classRelX86 = "";
	my	$classDebugX86 = "";
	my	$classRelX64 = "";
	my	$classDebugX64 = "";
	my	$classRelDotNet = "";
	my	$classDebugDotNet = "";

	if ($ref->{target} eq "" && $ref->{subtarget} eq "")
	{
		$classIgnore = 'class="gray"';
	}
	else
	{
		if ($ref->{relX86} ne "" || $ref->{debugX86} ne "" || $ref->{relX64} ne "" || $ref->{debugX64} ne "")
		{
			if ($ref->{relX86} eq "")
			{
				$classRelX86 = 'class="failed"';
			}
			else
			{
				$classRelX86 = 'class="success"';
			}

			if ($ref->{debugX86} eq "")
			{
				$classDebugX86 = 'class="failed"';
			}
			else
			{
				$classDebugX86 = 'class="success"';
			}

			if ($ref->{relX64} eq "")
			{
				$classRelX64 = 'class="failed"';
			}
			else
			{
				$classRelX64 = 'class="success"';
			}

			if ($ref->{debugX64} eq "")
			{
				$classDebugX64 = 'class="failed"';
			}
			else
			{
				$classDebugX64 = 'class="success"';
			}

			$classRelDotNet = 'class="na"';
			$classDebugDotNet = 'class="na"';

			$ref->{relDotNet} = "n/a";
			$ref->{debugDotNet} = "n/a";
		}
		elsif ($ref->{relDotNet} ne "" || $ref->{debugDotNet} ne "")
		{
			if ($ref->{relDotNet} eq "")
			{
				$classRelDotNet = 'class="failed"';
			}
			else
			{
				$classRelDotNet = 'class="success"';
			}

			if ($ref->{debugDotNet} eq "")
			{
				$classDebugDotNet = 'class="failed"';
			}
			else
			{
				$classDebugDotNet = 'class="success"';
			}

			$classDebugX86 = 'class="na"';
			$classRelX86 = 'class="na"';
			$classRelX64 = 'class="na"';
			$classDebugX64 = 'class="na"';

			$ref->{relX86} = "n/a";
			$ref->{debugX86} = "n/a";
			$ref->{relX64} = "n/a";
			$ref->{debugX64} = "n/a";
		}
		else
		{
			$classDebugX86 = 'class="gray"';
			$classRelX86 = 'class="gray"';
			$classRelX64 = 'class="gray"';
			$classDebugX64 = 'class="gray"';
			$classRelDotNet = 'class="gray"';
			$classDebugDotNet = 'class="gray"';
		}
	}

	if ($showDebug)
	{
		print FILE <<"EOT";
<tr>
	<td $classIgnore>$index</td>
	<td $classIgnore>$dir</td>
	<td $classIgnore>$ref->{target}</td>
	<td $classIgnore>$ref->{subtarget}</td>
	<td $classRelX86>$ref->{relX86}</td>
	<td $classDebugX86>$ref->{debugX86}</td>
	<td $classRelX64>$ref->{relX64}</td>
	<td $classDebugX64>$ref->{debugX64}</td>
	<td $classRelDotNet>$ref->{relDotNet}</td>
	<td $classDebugDotNet>$ref->{debugDotNet}</td>
</tr>
EOT
	}
	else
	{
		print FILE <<"EOT";
<tr>
	<td $classIgnore>$index</td>
	<td $classIgnore>$dir</td>
	<td $classIgnore>$ref->{target}</td>
	<td $classIgnore>$ref->{subtarget}</td>
	<td $classRelX86>$ref->{relX86}</td>
	<td $classRelX64>$ref->{relX64}</td>
	<td $classRelDotNet>$ref->{relDotNet}</td>
</tr>
EOT
	}

	$index++;
}

# Trailer
print FILE <<"EOT";
</table>
</body>
</html>
EOT

close FILE;
