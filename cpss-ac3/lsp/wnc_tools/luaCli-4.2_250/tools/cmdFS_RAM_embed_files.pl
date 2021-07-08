#!/usr/local/bin/perl -w
#
# DESCRIPTION:
#       This script generate C include file to embed files into
#        cmdFS_RAM (reference script)
#       Usage:
#       cmdFS_RAM_embed_files.pl [-o fname] -t tname list_file 
#       cmdFS_RAM_embed_files.pl [-o fname] [-x fname] -t tname -D dir
#           -o fname    - output file. Default cmdFS_RAM_embedded_data.inc
#           -x fname    - exclude list file
#           -D          - the argument is a directory. All files from this
#                         directory will be embedded
#           -S size     - the size of single file
#           -t          - target directory name (like "dxCh" or "exMxPm")
#           -Z          - compress .lua and .xml files
#           -E          - CMDFS_RAM_INIT_EMBEDDED extension
#
#
# INPUTS:
#       listing file -  file with two columns:
#            1. host file name (relative or absolute)
#            2. cmdFS file name (use / as path sep)
#
#            # Sample file list for embedding into cmdFS_RAM
#            #
#            # source file name                              target name
#            # ----------------                              -----------
#            ../lua/luaCLI/scripts/cmdLuaCLIDefs.lua         cmdLuaCLIDefs.lua
#            ../lua/luaCLI/scripts/genwrapper_API.lua        genwrapper_API.lua
#            ../lua/luaCLI/sample/bit_wise_test.lua          test/bit_wise.lua
#
#       exclude list file - the list of cmdFS file names to be excluded
#            # Sample exclude file list for embedding into cmdFS_RAM
#            #
#            # target name
#            # -----------
#            test/bit_wise.lua
#
# OUTPUTS:
#       cmdFS_RAM_embed_files.inc
#
# RETURNS:
#       none
#
# COMMENTS:
#
# FILE REVISION NUMBER:
#      $Revision: $8
#		11/03/2014 I.Rossovsky
#####################################################################

use strict;
use Cwd;
use Getopt::Std;
use File::Find;
use File::Path;
use File::Basename;
use File::Copy;
use Mail::Sendmail;
use MIME::Lite;

my $this = $0;
my (%options,
	$command,
	$inview,
	$CT,
	$OS,
	$log_file,
	$size_limit,
    $data,
	$slash,
	$result,
	$work_dir,
	$message,
	$int_list,
	$cc_list,
	@converted_local,
	@converted,
	@array,
	@converted_cc,
    );

$OS = MSWIN();
$log_file = $OS ? "c:\\temp\\cmdFS_RAM_embed_files_log.txt" : '/tmp/cmdFS_RAM_embed_files_log.txt';
$slash = $OS ? '\\' : '/';
$this = $0;
my $drive = substr(getcwd, 0, 2);
# net use | find /i "irinar_DEV_view"
open (LOG, "> $log_file");
##########################################
sub usage() {
print <<WIN;
    Usage:
    $this 
    $this [-S size]
	
	Parameters:
		-S size     - the size of single file
WIN
    exit 1;
}

##########################################
sub MSWIN { ($^O || $ENV{OS}) =~ /MSWin32|Windows_NT/i }

##########################################
sub init () {
    %options=();
	if ($#ARGV < 1) { &usage(); }
    getopts("hS:",\%options);
    &usage if ($options{h});	
	if ($options{S}) {
		$size_limit = $options{S};
	}
	$CT = $OS ? "cleartool" : '/usr/atria/bin/cleartool';
	$inview = `$CT pwv -s`;
    chomp($inview);
	$int_list = "adik michaelb";
	$cc_list = "GR-MISL-EnterpriseSW-ClearCase_Sup";
#	$int_list = "irinar";
#	$cc_list = "irinar";
	$message = "cmdFS_RAM_embed_files run results:<br>";
	$message = $message."==========================================<br>";
}

##########################################
sub doit(){
    my $file = $File::Find::name;
	if (-f $file) {
		$file =~ s/\//\\/g;
		push(@array,"$file\n");
	}
}

##########################################
sub update_cc() {
	my($local, $server) = @_;
    my ($file_name,@local_updated,@server_list,$updated);
	@local_updated = @{$local};
	@server_list = @{$server};
	foreach my $file (@server_list) {
		$updated = 0;
		chomp $file;
		if (-f $file) {
			system("$CT checkout -nc $file");
			my $file_name = basename($file);
			foreach my $tmp_file (@local_updated) {
				chomp $tmp_file;
				if ($tmp_file =~ $file_name) {
					$command = "copy /Y $tmp_file $file";
					system($command);
					$updated = 1;
					system("$CT diff -pre $file > c:\\temp\\output.txt");
					open(TMP,"< c:\\temp\\output.txt");
					$result = <TMP>;
					close TMP;
					if ($result =~ /Files are identical/) {
						system("$CT unco -rm $file");
						print LOG "File $file was not changed\n";
						$message = $message . "File $file was not changed<br>";
					} else {
						system("$CT checkin -nc $file");
						print LOG "Copy output file $file...\n";
						$message = $message . "Copy output file $file<br>";
					}
				}
			}
			unless ($updated) {
				open(TMP,"> $file");
				print TMP "";
				close(TMP);
				system("$CT diff -pre $file > c:\\temp\\output.txt");
				open(TMP,"< c:\\temp\\output.txt");
				$result = <TMP>;
				close TMP;
				if ($result =~ /Files are identical/) {
					system("$CT unco -rm $file");
				} else {
					system("$CT checkin -nc $file");
				}
			}
		}
	}
	return;
}

##########################################
sub send_mail() {
    my ($mail_to,$cc_mail_to,@addresses,@cc_addresses);
	my $mailsrv = 'owail.marvell.com';
    my $domain = 'marvell.com';
    $mail_to = "";
	my $user = getlogin();
	$data = shift;
    @addresses = split(' ', shift);
    while($#addresses+1 > 0) {
        $mail_to = $mail_to . $addresses[0] . '@' . $domain.", ";
        shift(@addresses);
    }
    @cc_addresses = split(' ', shift);
    while($#cc_addresses+1 > 0) {
        $cc_mail_to = $cc_mail_to . $cc_addresses[0] . '@' . $domain.", ";
        shift(@cc_addresses);
    }

    my $msg = MIME::Lite->new(From    => $user . '@' . $domain,
                    To      => $mail_to,
					Cc      => $cc_mail_to,
                    Subject => 'cmdFS_RAM_embed_files run results',
                    Type    => 'multipart/mixed');

    $msg->attach(
        Type => 'text/html',
        Data => qq{
            <body>
                $data
            </body>
        },
    );

    $msg->send('smtp', $mailsrv);

}

##########################################	
sub prepare() {
	unless (-e "c:\\temp") {
		mkdir("c:\\temp");
	}

	unless (-e "c:\\temp\\temp") {
		mkdir("c:\\temp\\temp");
	}
	
	if (-e "c:\\temp\\temp\\tools") {
		rename("c:\\temp\\temp\\tools","c:\\temp\\temp\\tools_cp");
	}
	mkdir("c:\\temp\\temp\\tools");
	copy("\\lua\\lua_cli\\tools\\cmdFS_RAM_embed_files.py", "c:\\temp\\temp\\tools\\cmdFS_RAM_embed_files.py");
	if (-e "c:\\temp\\temp\\tools\\scripts") {
		rmtree("c:\\temp\\temp\\tools\\scripts");
	}
	mkdir("c:\\temp\\temp\\tools\\scripts");
	
	if (-e "c:\\temp\\temp\\scripts") {
		rmtree("c:\\temp\\temp\\scripts");
	}
	mkdir("c:\\temp\\temp\\scripts");
	$command = "xcopy \\lua\\lua_cli\\scripts c:\\temp\\temp\\scripts\\ /I /Q /K /Y /R /E";
	system($command);
	&remove_contribs("c:\\temp\\temp\\scripts");
	
	if (-e "c:\\temp\\temp\\tools\\scripts_pm") {
		rmtree("c:\\temp\\temp\\tools\\scripts_pm");
	}
	mkdir("c:\\temp\\temp\\tools\\scripts_pm");
	
	if (-e "c:\\temp\\temp\\scripts_pm") {
		rmtree("c:\\temp\\temp\\scripts_pm");
	}
	mkdir("c:\\temp\\temp\\scripts_pm");
	$command = "xcopy \\lua\\lua_cli\\scripts_pm c:\\temp\\temp\\scripts_pm\\ /I /Q /K /Y /R /E";
	system($command);
	&remove_contribs("c:\\temp\\temp\\scripts_pm");
	
	chdir("c:\\temp\\temp\\tools");
	chdir("c:\\temp\\temp\\tools");

	$command = "cmdFS_RAM_embed_files.py -o scripts\\cmdFS_RAM_embed_files.c -z .lua -z .xml -S $size_limit -D c:\\temp\\temp\\scripts";
	$result = system($command);
	if ($result) {
		print LOG "There are some problems with running: \"cmdFS_RAM_embed_files.py -o scripts\\cmdFS_RAM_embed_files.c -z .lua -z .xml -S $size_limit -D c:\\temp\\temp\\scripts\"\n";
		$message = $message."There are some problems with running: \"cmdFS_RAM_embed_files.py -o scripts\\cmdFS_RAM_embed_files.c -z .lua -z .xml -S $size_limit -D c:\\temp\\temp\\scripts\"\n";
	}
	$command = "cmdFS_RAM_embed_files.py -o scripts_pm\\cmdFS_RAM_embed_files.c -z .lua -z .xml -S $size_limit -D c:\\temp\\temp\\scripts_pm";
	$result = system($command);
	if ($result) {
		print LOG "There are some problems with running: \"cmdFS_RAM_embed_files.py -o scripts_pm\\cmdFS_RAM_embed_files.c -z .lua -z .xml -S $size_limit -D c:\\temp\\temp\\scripts_pm\"\n";
		$message = $message."There are some problems with running: \"cmdFS_RAM_embed_files.py -o scripts_pm\\cmdFS_RAM_embed_files.c -z .lua -z .xml -S $size_limit -D c:\\temp\\temp\\scripts_pm\"<br>";
	}
}

##########################################	
sub remove_contribs () {
	my $dir = shift;
	find(\&del_contrib,"$dir");
}

##########################################	
sub del_contrib () {
    my $file = $File::Find::name;
	if ($file =~ /contrib/) {
		unlink($file);
	}
}

##########################################	
sub main () {
	&prepare();
	
	find(\&doit,"c:\\temp\\temp\\tools\\scripts");
	@converted_local = @array;
	@array = ("");
	find(\&doit,"$drive\\lua\\lua_cli\\inc\\dxCh");
	@converted = @array;
	@array = ("");
	&update_cc(\@converted_local,\@converted);

	find(\&doit,"c:\\temp\\temp\\tools\\scripts_pm");
	@converted_local = @array;
	@array = ("");
	find(\&doit,"$drive\\lua\\lua_cli\\inc\\exMxPm");
	@converted = @array;
	&update_cc(\@converted_local,\@converted);

	&send_mail($message,$int_list,$cc_list);
}
&init();
&main();
exit;