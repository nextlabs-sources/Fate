#!/usr/local/bin/perl


# FTPE Destroyer
# Company: NextLabs, Inc.
# Tool To Verify Correctness, Stability, Reliability of FTP Enforcer
# Built: May 27, 2009
# Modified: June 9, 2009 (Added options for -server,-user,-passwd)
# Author: Abdur Rehman Pathan


use Net::FTP;
use Getopt::Long;
use Time::HiRes qw(gettimeofday);

my $upcount = 0;
my $downcount = 0;
my $cwdcount = 0;
my $upiter = -1;
my $downiter = -1;
my $cwditer = -1;
my $upfile = "";
my $downfile = "";
my $defaultiter = 1;
my $listflag = 0;
my $helpflag = 0;
my $servername = "";
my $username = "";
my $passwd = "";

if( @ARGV <= 0 ) {
 &help;
} 


# Read the Command Line Options
 GetOptions(	'up:i'     	=> \$upiter,
 		'down:i'   	=> \$downiter,
 		'upfile:s' 	=> \$upfile,
 		'downfile:s'	=> \$downfile,
		'cwd:i'	   	=> \$cwditer,
		'list'	   	=> \$listflag,
		'server:s'   	=> \$servername,
		'user:s'   	=> \$username,
		'passwd:s'   	=> \$passwd,
		'help'	   	=> \$helpflag)
 or &help;


# Check for -help Option, and display help
if( $helpflag ) {
 &help;
 exit;
}

# Connect to the FTP Server and Login
 if( $servername eq "" ) {
  print "\nNo Servername Specified. Please use -server option and specify a servername\n";
  &help;
 }
 else {
  $ftp = Net::FTP->new($servername, Active => 0)
   or die "\nCannot connect to Server: $servername\n";
 
  if( $username eq "" ) {
   print "\nUsername not specified with -user. Using default username = foo\n";
   $username="foo"; 
  }  
  if( $passwd eq "" ) {
   print "\nPassword not specified with -passwd. Using default passwd = foo\n";
   $passwd="foo"; 
  }  
  $ftp->login($username,$passwd)
   or die "\nUnable to login to ftp server with username: $user and password:$passwd: ", $ftp->message;
 }

# Uploads first
 if ( $upiter >= 0) {
  if ( $upiter == 0) {
   $upiter=$defaultiter;
  }
  if( $upfile eq "" ) { 
   $upfile=&generate_random_string((int(rand(251)))); 
   $upfile = $upfile . ".txt";
  }   
  $before = gettimeofday;  
  for ($upcount = 0; $upcount < $upiter; $upcount++ ) {
   $ftp->put($upfile)
    or print "\nUnable to upload File $upfile to ftp server: ", $ftp->message;
  }
  $elapsed = gettimeofday - $before;
  $elapsedms = $elapsed * 1000;
  print "\nUpload Operation on File: $upfile $upiter time(s) took $elapsedms milliseconds\n"; 
 }


# Downloads next
 if ( $downiter >= 0) {
  if ( $downiter == 0) {
   $downiter=$defaultiter; 
  }
  if( $downfile eq "" ) { 
   $downfile=&generate_random_string((int(rand(251)))); 
   $downfile = $downfile . ".txt";
  }   
  $before = gettimeofday;  
  for ($downcount = 0; $downcount < $downiter; $downcount++ ) {
   $ftp->get($downfile)
    or print "\nUnable to download File $downfile from ftp server: ", $ftp->message;
  }
  $elapsed = gettimeofday - $before;
  $elapsedms = $elapsed * 1000;
  print "\nDownload  Operation on File: $downfile $downiter time(s) took $elapsedms milliseconds\n"; 
 } 


# Change Working Directory- cwd
 if ($cwditer >= 0) {
  if ($cwditer == 0) { 
   $cwditer=$defaultiter; 
  }
  $before = gettimeofday;  
  for ($cwdcount = 0; $cwdcount < $cwditer; $cwdcount++ ) {
   my $random_cwd=&generate_random_string((int(rand(255)))); 
   $ftp->cwd($random_cwd); 
  } 
  $elapsed = gettimeofday - $before;
  $elapsedms = $elapsed * 1000;
  print "\nCWD $cwditer time(s) took $elapsedms milliseconds\n"; 
 }

@dir = $ftp->dir();

# List files in a directory
 if ( $listflag == 1) {
  print "\nThe following files are in the root directory:\n";
  $before = gettimeofday;  
  foreach $file (@dir ) {
   print "$file\n";
  } 
  $elapsed = gettimeofday - $before;
  $elapsedms = $elapsed * 1000;
  print "\nList Files took $elapsedms milliseconds\n"; 
 }

$ftp->quit;

sub generate_random_string
{
	my $length_of_randomstring=shift;# the length of 
			 # the random string to generate

	my @chars=('a'..'z','A'..'Z','0'..'9','_');
	my $random_string;
	foreach (1..$length_of_randomstring) 
	{
		# rand @chars will generate a random 
		# number between 0 and scalar @chars
		$random_string.=$chars[rand @chars];
	}
	return $random_string;
}

sub help {
 print "\n\nUsage: \n";
 print "\n";
 print "ftpedestroyer [options]";
 print "\n";
 print "\nOptions:\n";
 print "\n";
 print "-server	[servername]\t\tFTP Server Name to connect to (Required Arg)\n";
 print "-user [username]\t\tUsername to login on server (Default: foo)\n";
 print "-passwd [passwd]\t\tPassword to login on server (Default: foo)\n";
 print "\n";
 print "-up [count]\t\t\tUpload a File count times (default count is 1)\n";
 print "-upfile	[filename]\t\tFile to be uploaded. This parameter -upfile is\n"; 
 print "\t\t\t\toptional. If this option is not used with -up \n";
 print "\t\t\t\tor no filename is specified, a random filename\n";
 print "\t\t\t\twill be generated and used\n";
 print "\n";
 print "-down [count]\t\t\tDownload a File count times(default count is 1)\n";
 print "-downfile [filename]\t\tFile to be downloaded. This parameter\n";
 print "\t\t\t\t-downfile is optional. If this option is not\n";
 print "\t\t\t\tused with -down or no filename is specified,\n";
 print "\t\t\t\ta random filename will be generated and used\n";
 print "\n";
 print "-cwd [count]\t\t\tChange Working Directory count times (randomly)\n";
 print "\t\t\t\t(Default count is 1)\n";
 print "\n-list\t\t\t\tList all files under the home directory\n";
 print "\n";
 print "**NOTE: The sequence of operations will be:";
 print "\n\t1)Connect to Server";
 print "\n\t2)Uploads";
 print "\n\t3)Downloads";
 print "\n\t4)CWD";
 print "\n\t5)List\n";
 print "\nExamples:\n\n";
 print "perl ftpedestroyer -server test-ftp.com -up 1000\n";
 print "\tConnects to Server test-ftp.com with User=foo and Password=foo\n";
 print "\tInvokes Upload on a random filename 1000 times\n";
 print "\n";
 print "perl ftpedestroyer -server test-ftp.com -user test -passwd test -up 1000 -upfile c:/foo.txt\n";
 print "\tConnects to Server test-ftp.com with User=test and Password=test\n";
 print "\tUploads c:/foo.txt 1000 times to the Home directory\n";
 print "\n";
 print "perl ftpedestroyer -server test-ftp.com  -down 100 -downfile foo.txt\n";
 print "\tConnects to Server test-ftp.com with User=foo and Password=foo\n";
 print "\tDownloads foo.txt 1000 times to the current directory\n";
 print "\n";
 print "perl ftpedestroyer -server test-ftp.com -up 10000 -down 100 -cwd\n";
 print "\tConnects to Server test-ftp.com with User=foo and Password=foo\n";
 print "\tInvokes Upload on a random filename 10000 times\n";
 print "\tInvokes Download on random filenames 100 times\n";
 print "\tPerforms CWD on a random path 1 time\n";
 print "\n";
 print "perl ftpedestroyer -server test-ftp.com -cwd 100\n";
 print "\tConnects to Server test-ftp.com with User=foo and Password=foo\n";
 print "\tPerforms CWD on random paths 100 times\n";
 print "\n";
 print "perl ftpedestroyer -server test-ftp.com -list\n";
 print "\tConnects to Server test-ftp.com with User=foo and Password=foo\n";
 print "\tList files in the home directory\n";
 print "\n";
 exit;
}

