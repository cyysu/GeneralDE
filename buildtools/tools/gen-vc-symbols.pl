use strict;
use Getopt::Long;

my $inputFile;
my $outputFile;

GetOptions("input-file=s" => \$inputFile,
           "output-file=s"   => \$outputFile
          );

open(my $input, '<::encoding(utf8)', $inputFile) or die "open input file $inputFile fail $!";
open(my $output, '>::encoding(utf8)', $outputFile) or die "open output file $outputFile fail $!";

print $output "EXPORTS\n";

while(my $line = <$input>) {
  $line =~ s/^_(.+)$/$1/;
  print $output $line;
}

close($input);
close($output);
