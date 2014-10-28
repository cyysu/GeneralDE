use strict;
use Encode;
use Getopt::Long;
use Spreadsheet::ParseExcel;
use Data::Dumper;
use File::Basename;
use File::Spec;
use File::stat;

binmode(STDOUT, ":utf8");

my @inputs;
my $output;
my @languages;
my $startLine;

GetOptions("input=s" => \@inputs,
           "output=s"   => \$output,
           "language=s"   => \@languages,
           "start-line=i" => \$startLine);

$startLine = 1 if ! defined $startLine;

sub is_role_all_empty {
  my $sheet = shift;
  my $rowPos = shift;
  my $col_min = shift;
  my $col_max = shift;

  foreach my $colPos ( $col_min .. $col_max ) {
    my $cell = $sheet->get_cell($rowPos, $colPos);
    next if not defined $cell;
    next if not defined $cell->value() or $cell->value() =~ m/^\s*$/;
    return 0;
  }

  return 1;
}

my %language_tables = ();

foreach my $input ( @inputs ) {
  $input =~ /^(.*)\.([^.]*)$/ or die "input $input format error!";

  my $inputSheet = decode("utf8", $2);
  my $inputFile = decode("utf8", $1) . ".xls";

  my $parser =  Spreadsheet::ParseExcel->new();
  my $workbook = $parser->parse($inputFile);
  if ( ! defined $workbook ) {
    die $parser->error(), ".\n";
  }

  my $sheet = $workbook->worksheet($inputSheet);
  if ( ! defined $sheet ) {
    die "sheet $inputSheet not exist in $inputFile!";
  }

  my ( $row_min, $row_max ) = $sheet->row_range();
  my ( $col_min, $col_max ) = $sheet->col_range();

  if ( $row_max < 0 || $row_min > $row_max ) {
    die "sheet $inputSheet row range error!";
  }
  if ( $col_max < 0 || $col_min >= $col_max ) {
    die "sheet $inputSheet col range error!";
  }

  my %tableHead = ();
  foreach my $colPos ($col_min .. $col_max) {
    my $cell = $sheet->get_cell($row_min, $colPos);
    if ($cell) {
      $tableHead{$cell->value()} = $colPos;
    }
  }

  die "$inputFile -- $inputSheet: id column not exist!" if not exists $tableHead{id};
  foreach my $language ( @languages ) {
    die "$inputFile -- $inputSheet: $language column not exist!" if (not exists $tableHead{$language});
  }

  foreach my $rowPos ( $row_min + $startLine .. $row_max ) {

    last if is_role_all_empty($sheet, $rowPos, $col_min, $col_max);

    my $msg_id;
    if (my $cell = $sheet->get_cell($rowPos, $tableHead{id})) {
      $msg_id = $cell->value();
    }

    next and print "$inputFile -- $inputSheet: row $rowPos id not config!"
      if not defined $msg_id;

    foreach my $language ( @languages ) {
      my $msg = "";
      if (my $cell = $sheet->get_cell($rowPos, $tableHead{$language})) {
        $msg = $cell->value();
      }

      my $table = $language_tables{$language};
      if (not defined $table) {
        $table = {};
        $language_tables{$language} = $table;
      }

      print "$inputFile -- $inputSheet: row $rowPos id $msg_id language $language already defined!" and next
        if exists $table->{$msg_id};

      $table->{$msg_id} = encode("utf-8", $msg);
    }
  }
}

foreach my $lan ( keys %language_tables ) {
  my $string_tab = $language_tables{$lan};
  my $file_name = $output . "_" . $lan . ".stb";

  my @sorted_string_tab = 
    sort { $a->{msg_id} <=> $b->{msg_id} } 
      map { { msg_id => $_ , msg => $string_tab->{$_} } } keys $string_tab;

  my $start_pos = 0;
  map { $_->{pos} = $start_pos; $start_pos += length($_->{msg}) + 1 } @sorted_string_tab;

  open(my $output, '>:bytes', $file_name) or die "open output file $file_name fail $!";

  print $output pack("a4N", "STB\01", scalar(@sorted_string_tab));

  foreach my $msg ( @sorted_string_tab ) {
    print $output pack("NN", $msg->{msg_id}, $msg->{pos});
  }

  foreach my $msg ( @sorted_string_tab ) {
    print $output pack("VV", $msg->{msg_id}, $msg->{pos});
  }

  foreach my $msg ( @sorted_string_tab ) {
    print $output $msg->{msg};
    print $output "\0";
  }

  close($output);
}

1;

# -*- encoding: utf-8 -*-
