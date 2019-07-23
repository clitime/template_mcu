#!/user/bin/perl


use Getopt::Long;
use File::Copy;
use File::Basename;
use IO::Compress::Gzip qw(gzip $GzipError);
use JavaScript::Minifier; # qw(minify);

my $path = "./";
my $rls = '';
my $dir_tmp = "./js/tmp";

GetOptions(	'path=s' => \$path,
			'rls' => \$rls);
print("rls = $rls\n");

open (OUTPUT, ">", $path."fsdata.c");
open (OUTPUT2, ">", $path."fsdata.h");

chdir($path."httpd");

opendir(DIR, ".");
@files =  grep { !/^\./ && !/(CVS|~)/ } readdir(DIR);
closedir(DIR);

foreach $file (@files) {

    if(-d $file && $file !~ /^\./) {
		print "Processing directory $file\n";
		opendir(DIR, $file);
		@newfiles =  grep { !/^\./ && !/(CVS|~)/ } readdir(DIR);
		closedir(DIR);
		printf "Adding files @newfiles\n";
		@files = (@files, map { $_ = "$file/$_" } @newfiles);
		next;
    }
}

print(OUTPUT "#include \"fsdata.h\"\n\n");
print(OUTPUT "#include <stddef.h>\n\n");

foreach $file (@files) {
    if(-f $file) {

		print "Adding file $file\n";

#нужно получить расширение файла
		my ($ext) = $file =~ /\/?\w+\.([a-z0-9]+)$/;

		if ($ext =~ /js/) {
			$dir_tmp = dirname($file, ".js")."/tmp/";
			my $name = basename($file);

			if(-d $dir_tmp) {
			    unlink glob "$dir_name/*";
			}
			else {
			    mkdir $dir_tmp;
			}

			if ($rls) {
				open(TMPFILE, ">", $dir_tmp."min".$name) || die "Could not open file $minfile\n";
		     	open(FILE, $file) || die "Could not open file $file\n";
		     	JavaScript::Minifier::minify(input => *FILE, outfile => *TMPFILE);
				close(TMPFILE);
				close(FILE);
				gzip $dir_tmp."min".$name => $dir_tmp.$name or die "gzip failed: $GzipError\n";
				open(FILE, $dir_tmp.$name) || die "Could not open file $file\n";
			}
			else {
				gzip $file => $dir_tmp.$name or die "gzip failed: $GzipError\n";
				open(FILE, $dir_tmp.$name) || die "Could not open file $file\n";
			}
		}
		else {
			open(FILE, $file) || die "Could not open file $file\n";
		}

		$file =~ s-^-/-;
		$fvar = $file;
		$fvar =~ s-/-_-g;
		$fvar =~ s-\.-_-g;

		print(OUTPUT "static const unsigned char data".$fvar."[] = {\n");
		print(OUTPUT "\t/* $file */\n\t");
		for($j = 0; $j < length($file); $j++) {
			printf(OUTPUT "%#02x, ", unpack("C", substr($file, $j, 1)));
		}
		printf(OUTPUT "0,\n");

		$i = 0;
		while(read(FILE, $data, 1)) {
			if($i == 0) {
				print(OUTPUT "\t");
			}
			printf(OUTPUT "%#02x, ", unpack("C", $data));
			$i++;
			if($i == 10) {
				print(OUTPUT "\n");
				$i = 0;
			}	
		}
		print(OUTPUT "0};\n\n");
		close(FILE);
		push(@fvars, $fvar);
		push(@pfiles, $file);
    }
}

if(-d $dir_tmp) {
	unlink glob "$dir_tmp/*";
	rmdir $dir_tmp;
}

for($i = 0; $i < @fvars; $i++) {
    $file = $pfiles[$i];
    $fvar = $fvars[$i];

    if($i == 0) {
        $prevfile = "NULL";
    } else {
        $prevfile = "&file" . $fvars[$i - 1];
    }
    if ($i < @fvars - 1) {
    	print(OUTPUT "static const struct fsdata_file file".$fvar." = {$prevfile, data$fvar, ");
    }
    else {
    	print(OUTPUT "const struct fsdata_file file".$fvar." = {$prevfile, data$fvar, ");
    }
    print(OUTPUT "data$fvar + ". (length($file) + 1) .", ");
    print(OUTPUT "sizeof(data$fvar) - ". (length($file) + 1) ."};\n\n");
}

print(OUTPUT "#define FS_NUMFILES $i\n");


print(OUTPUT2 "#ifndef __FSDATA_H__\n");
print(OUTPUT2 "#define __FSDATA_H__\n");
print(OUTPUT2 "\n");
print(OUTPUT2 "#include \"fs.h\"\n");
print(OUTPUT2 "\n");
print(OUTPUT2 "extern const struct fsdata_file file$fvars[$i - 1];\n");
print(OUTPUT2 "\n");
print(OUTPUT2 "#define FS_ROOT file$fvars[$i - 1]\n");
print(OUTPUT2 "\n");
print(OUTPUT2 "struct fsdata_file {\n");
print(OUTPUT2 "  const struct fsdata_file *next;\n");
print(OUTPUT2 "  const unsigned char *name;\n");
print(OUTPUT2 "  const unsigned char *data;\n");
print(OUTPUT2 "  const int len;\n");
print(OUTPUT2 "};\n");
print(OUTPUT2 "\n");
print(OUTPUT2 "struct fsdata_file_noconst {\n");
print(OUTPUT2 "  struct fsdata_file *next;\n");
print(OUTPUT2 "  char *name;\n");
print(OUTPUT2 "  char *data;\n");
print(OUTPUT2 "  int len;\n");
print(OUTPUT2 "};\n");
print(OUTPUT2 "\n");
print(OUTPUT2 "#endif /* __FSDATA_H__ */\n");
print(OUTPUT2 "\n");

close (OUTPUT);
close (OUTPUT2);
