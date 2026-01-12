#!/usr/bin/perl -w
use strict;

my $desc=<<'DESC';
Continuous scan of last aqbanking logfile for images used as photo TANs.
Usable in parallel to aqbanking-cli.

Usage:  perl show_photoTANs.pl [OPTION ...] [LOGFILES] [SHOW]
	OPTION	: -t : test output
		  -v : print filename(s)
	LOGFILES : including directory and name (use wild cards)
	SHOW	: (optional) command/program to show a png image

  If the program reaches end-of-file of the currently read logfile, it will
  look for a newer logfile and use that file.

  Only the currently last picture in a logfile is shown.
  The picture must be closed to enable the search for the next picture.
  
  (c) H. Ruprecht : usable according to GNU public license (no warrenty)
DESC

my $defLogFiles = "~/.aqbanking/backends/aqhbci/data/banks/de/*/logs/*.log";
my $defShow = "eog"; # command to show the image (LINUX)
my $imageFile = "x.png";

my $test=0; my $verbose=0;
while ($ARGV[0] && substr($ARGV[0],0,1) eq "-") {
    my $option=shift;
    $test++ if $option=~/t/;
    $verbose++ if $option=~/v/;
    print $desc,"\ndefaults: ",$defLogFiles," ",$defShow,"\n" 
	if $option=~/h/i;
    }
my $logFiles=$ARGV[0] || $defLogFiles;
my $showImage=$ARGV[1] || $defShow;

my $startImage = chr(0x89)."PNG\r\n".chr(0x1a)."\n";
my $endImage = "IEND".chr(0xAE).chr(0x42).chr(0x60).chr(0x82);

my $logFile=findLogFile($logFiles);
die "No logfile found in $logFiles\n" unless $logFile;

$|=1;
print ">" if $test;
my $n=0;

do {
    print "Using $logFile\n" if $verbose;
    $logFile=readNextImage($logFile,$logFiles);
    }
until $logFile eq "";

sub findLogFile {
    my $files = shift;
    
    my $file=""; my $rdt=0;
    my @files=glob($files);
    foreach my $next (@files) {
	my $rdtNext=(lstat($next))[9];
	next if $rdtNext<$rdt;
	$file=$next;
	$rdt=$rdtNext;
	}
    return $file;
    } # findLogFile

sub readNextImage {
    my ($logFile, $logFiles) = @_;
    
    open(my $log,$logFile) || die "$logFile open error: $!\n";
    binmode($log);
    my $full=""; 
    while (1) {
	my $part=<$log>; 
	if (!defined($part)) { # eof($log)
	    if (findImage($full)) { $full=""; }
	    elsif ($logFiles=~/[\*\?]/) {
		my $newFile=findLogFile($logFiles);
		if ($newFile ne $logFile) {
		    close($log);
		    return $newFile;
		    }
		}
	    sleep(1);
	    next; 
	    }
	$full.=$part;
	}
    } # readNextImage
    
sub findImage {
    my $full= shift; 
    my $image="";
    while ((my $pS=index($full,$startImage))>=0) {
	my $pE=index($full,$endImage,$pS);
	if ($pE<0) {
	    last if $image ne "";
	    sleep(1); 
	    return; 
	    }
	$image=substr($full,$pS,$pE-$pS);
	$n++;
	$full=substr($full,$pE+1);
	}
    if ($image eq "") { sleep(1); return; }
    open(IMG,'>',"$imageFile") || die "$imageFile open error: $!\n";
    print IMG $image;
    close(IMG);
    system("$showImage $imageFile");
    return 1;
    } # findImage
