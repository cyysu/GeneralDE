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
my @spineFiles;
my @bulletsFiles;
my @emitterFiles;

my $outputDir = dirname($outputFile);
if ( not -e $outputDir) {
  mkpath($outputDir) or die "create output dir $outputDir fail $!";
}

sub readMetaFromFile {
  my $array_ref = shift;
  my $file_name = shift;

  open(my $meta_file, "<:crlf:encoding(utf8)", $file_name) or die "open $file_name fail $!";

  (my $guid, my $path, my $file) = <$meta_file>;

  $guid =~ s/[\r\n]//;
  $path =~ s/[\r\n]//;
  $file =~ s/[\r\n]//;

  close($meta_file);

  push @{ $array_ref }, { guid => $guid, path => $path, file => $file };
}

sub wanted {
  if (/\.spine$/) {
    $File::Find::name =~ m/^$inputDir\/(.*[\/]+)([^\/]+)$/;
    push @spineFiles, { guid => 0, path => $1, file => $2 };
    return;
  }
  elsif (/\.emitter$/) {
    $File::Find::name =~ m/^$inputDir\/(.*[\/]+)([^\/]+)$/;
    push @emitterFiles, { guid => 0, path => $1, file => $2 };
    return;
  }
  elsif (/\.bullets$/) {
    $File::Find::name =~ m/^$inputDir\/(.*[\/]+)([^\/]+)$/;
    push @bulletsFiles, { guid => 0, path => $1, file => $2 };
    return;
  }

  return if not /\.meta$/s;
  return if /^2DSResource\.meta$/s;

  if (/\.ibk\.meta$/) {
    readMetaFromFile(\@moduleFiles, $File::Find::name);
  }
  elsif (/\.frm\.meta$/) {
    readMetaFromFile(\@spriteFiles, $File::Find::name);
  }
  elsif (/\.act\.meta$/) {
    readMetaFromFile(\@actionFiles, $File::Find::name);
  }
  else {
    die "unknown meta file $_\n";
  }
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
print $output "    <Spine>\n";
foreach my $file (@spineFiles) {
  print $output "        <Meta Guid='$file->{guid}' Path='$file->{path}' File='$file->{file}' />\n";
}
print $output "    </Spine>\n";
print $output "    <Bullets>\n";
foreach my $file (@bulletsFiles) {
  print $output "        <Meta Guid='$file->{guid}' Path='$file->{path}' File='$file->{file}' />\n";
}
print $output "    </Bullets>\n";
print $output "    <Emitter>\n";
foreach my $file (@emitterFiles) {
  print $output "        <Meta Guid='$file->{guid}' Path='$file->{path}' File='$file->{file}' />\n";
}
print $output "    </Emitter>\n";
print $output "</Resources>\n";
1;

