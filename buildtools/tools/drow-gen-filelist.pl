use strict;
use Getopt::Long;
use File::Basename;
use File::Find;
use File::Path;

my $inputDir;
my $outputFile;

GetOptions("input=s" => \$inputDir
           , "output=s" => \$outputFile
           );

my @moduleFiles;
my @spriteFiles;
my @actionFiles;

my $outputDir = dirname($outputFile);
if ( not -e $outputDir) {
  mkpath($outputDir) or die "create output dir $outputDir fail $!";
}

sub wanted {
  return if not /\.meta$/s;
  return if /^2DSResource\.meta$/s;

  open(my $meta_file, "<:crlf:encoding(utf8)", $File::Find::name) or die "open $File::Find::name fail $!";

  (my $guid, my $path, my $file) = <$meta_file>;

  $guid =~ s/[\r\n]//;
  $path =~ s/[\r\n]//;
  $file =~ s/[\r\n]//;

  close($meta_file);

  my $array_ref;

  if (/\.ibk\.meta$/) {
    $array_ref = \@moduleFiles;
  }
  elsif (/\.frm\.meta$/) {
    $array_ref = \@spriteFiles;
  }
  elsif (/\.act\.meta$/) {
    $array_ref = \@actionFiles;
  }
  else {
    die "unknown meta file $_\n";
  }

  push @{ $array_ref }, { guid => $guid, path => $path, file => $file };
}

find(\&wanted, $inputDir);

open(my $output, '>::encoding(utf8)', $outputFile) or die "open output file $outputFile fail $!";

print $output "<?xml version='1.0' encoding='UTF-8' ?>\n";
print $output "<Resources>\n";
print $output "    <Module>\n";
foreach my $file (@moduleFiles) {
  print $output "        <Meta Guid='$file->{guid}' Path='$file->{path}' File='$file->{file}' />\n";
}
print $output "    </Module>\n";

print $output "    <Sprite>\n";
foreach my $file (@spriteFiles) {
  print $output "        <Meta Guid='$file->{guid}' Path='$file->{path}' File='$file->{file}' />\n";
}
print $output "    </Sprite>\n";

print $output "    <Action>\n";
foreach my $file (@actionFiles) {
  print $output "        <Meta Guid='$file->{guid}' Path='$file->{path}' File='$file->{file}' />\n";
}
print $output "    </Action>\n";
print $output "</Resources>\n";
1;

