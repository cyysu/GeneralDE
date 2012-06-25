use strict;
use YAML;
use Encode;
use XML::Simple;
use Getopt::Long;
use Spreadsheet::ParseExcel;
use Data::Dumper;

binmode(STDOUT, ":utf8");

my $inputFile;
my $inputSheet;
my $outputFile;
my $metaLibFile;
my $metaName;

GetOptions("input-file=s" => \$inputFile,
           "input-sheet=s" => \$inputSheet,
           "output=s"   => \$outputFile,
           "meta-lib=s" => \$metaLibFile,
           "meta-name=s" => \$metaName);

$inputSheet = decode("utf8", $inputSheet);
$inputFile = decode("utf8", $inputFile);

my $xml_lib_source = XMLin($metaLibFile, KeyAttr => {struct => 'name', union => 'name'}, ForceArray => [ 'struct', 'union' ]);

my %metaLib;

if (exists $xml_lib_source->{struct}) {
  foreach my $meta_name ( keys %{$xml_lib_source->{struct}} ) {
    $metaLib{$meta_name} = $xml_lib_source->{struct}->{$meta_name};
    $metaLib{$meta_name}->{meta_type} = 'struct';
  }
}

if (exists $xml_lib_source->{union}) {
  foreach my $meta_name ( keys %{$xml_lib_source->{union}} ) {
    $metaLib{$meta_name} = $xml_lib_source->{union}->{$meta_name};
    $metaLib{$meta_name}->{meta_type} = 'union';
  }
}
#print Dump(\%metaLib);

my %input_col_processors;

sub is_entry_basic_type {
  my $entry = shift;

  return $entry->{type} =~ /^(string)|(char)|(float)|(double)|(u?int(8|16|32|64))$/;
}

sub add_col_processor {
  my ($colName, $fun) = @_;

  $input_col_processors{$colName} = []
    if not exists $input_col_processors{$colName};

  push @{$input_col_processors{$colName}}, $fun;
}

sub calc_col_fun {
  my ($colName, $resultColName, $selector) = @_;

  if ($selector && $selector =~ /split\s*\(\s*'([^']+)'\s*,\s*(\d+)\s*\)/) {
    my $sep = $1;
    my $getPos = $2;

    return sub {
      my ($row, $value, $input_row) = @_;
      my @values = split(/$sep/, $value);
      if ($getPos < @values + 0) {
        $row->{$resultColName} = $values[$getPos];
      }
    };
  }

  if ($selector && $selector =~ /match\s*\(\s*'([^']+)'\s*\)/) {
    my $matcher = $1;

    return sub {
      my ($row, $value, $input_row) = @_;
      if ($value =~ /$matcher/) {
        $row->{$resultColName} = $1;
      }
    };
  }

  if ($selector && $selector =~ /convert\s*\(\s*'([^']+)'\s*:\s*([^)]*)\)/) {
    my $matcher = $1;

    my %converts;
    my $default;

    foreach my $selectItem (split(/,/, $2)) {
      if ($selectItem =~ /^\s*'([^']+)'\s*\?\s*([^\s]+)\s*$/) {
        $converts{$1} = $2;
      }
      elsif ( $selectItem =~ /^\s*default\s*([^\s]+)\s*$/ ) {
        $default = $1;
      }
    }

    return sub {
      my ($row, $value, $input_row) = @_;

      if ($value =~ /$matcher/) {
        if (not defined $1) {
          if ($default) {
            $row->{$resultColName} = $default;
          }
        }
        else {
          my $input = $1;
          if (defined $input && exists $converts{$input} ) {
            $row->{$resultColName} = $converts{$input};
          } elsif ($default) {
            $row->{$resultColName} = $default;
          }
        }
      }
    };
  }

  if ($selector && $selector =~ /replace\s*\(\s*'([^']+)'\s*:\s*'([^']+)'\s*\)/) {
    my $replace_from = $1;
    my $replace_to = $2;

    return sub {
      my ($row, $value, $input) = @_;
      $value =~ s/$replace_from/$replace_to/;
      $row->{$resultColName} = $value;
    };
  }

  if ($selector && $selector =~ /value\s*\(\s*'([^']+)'\s*\)/) {
    my $value = $1;

    return sub {
      my ($row, $old_value) = @_;
      $row->{$resultColName} = $value;
    };
  }

  return sub {
    my ($row, $value, $input_row) = @_;
    $row->{$resultColName} = $value;
  };
}

sub analize_entry_processor_union {
  my ($meta, $entry, $cname_post_fix, $col_fun_derator) = @_;

  return if (not exists $entry->{cname});

  my $input_col_name = $entry->{cname} . $cname_post_fix;

  if ($entry->{customattr} =~ /match\s*\(\s*'([^']*)'\s*\)/) {
    my $filter = $1;

    my $subMeta = $metaLib{$entry->{type}};
    if (not $subMeta) {
      print("$entry->{name} ref type $entry->{type} is unknown!\n");
      return;
    }

    analize_submeta($subMeta,
                    $cname_post_fix,
                    $entry,
                    sub {
                      my $innerSub = shift;

                      my $newSub = sub {
                        my ($row, $value, $input_row) = @_;

                        return if not exists $input_row->{$input_col_name};
                        return if $input_row->{$input_col_name} !~ $filter;

                        $innerSub->($row, $value, $input_row);
                      };

                      $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                      return $newSub;
                    });
  }
}

sub analize_entry_processor_struct {
  my ($meta, $entry, $cname_post_fix, $col_fun_derator) = @_;

  if (defined $entry->{count} and $entry->{count} != 1) {
    my $count = $entry->{count};

    if (is_entry_basic_type($entry)) {
      return if not exists $entry->{cname};

      my $input_col_name = $entry->{cname} . $cname_post_fix;

      if ($entry->{customattr} && $entry->{customattr} =~ /split\s*\(\s*'([^']+)'\s*\)/) {
        my $sep = $1;
        my $getPos = $2;

        my $col_fun = sub {
          my ($row, $value, $input_row) = @_;

          $row->{$entry->{name}} = [];

          @{$row->{$entry->{name}}} = split(/$sep/, $value);
        };

        $col_fun = $col_fun_derator->($col_fun) if defined $col_fun_derator;
        add_col_processor($input_col_name, $col_fun);
      }
      elsif ($entry->{customattr} && $entry->{customattr} =~ /make_array\s*\(\s*'([^']+)'\s*\)/) {
        my @postfixs = split(':', $1);

        foreach my $pos ( 0 .. $#postfixs ) {
          my $col_name = $input_col_name . $postfixs[$pos];

          my $col_fun = sub {
            my ($row, $value, $input_row) = @_;

            return if (not $value) or ($value eq "");

            $row->{$entry->{name}} = []
              if not exists $row->{$entry->{name}};

            while ( @{$row->{$entry->{name}}} < $pos) {
              push @{$row->{$entry->{name}}}, "";
            }

            ${$row->{$entry->{name}}}[$pos] = $value;
          };

          $col_fun = $col_fun_derator->($col_fun) if defined $col_fun_derator;
          add_col_processor($col_name, $col_fun);
        }
      }

      return;
    }

    my $subMeta = $metaLib{$entry->{type}};
    if (not $subMeta) {
      print("$entry->{name} ref type $entry->{type} is unknown!\n");
      return;
    }

    if ($subMeta->{meta_type} eq "union") {
      analize_submeta($subMeta, $cname_post_fix, $entry, $col_fun_derator);
      return;
    }

    return if not exists $entry->{customattr};

    if ($entry->{customattr} eq "repeat") {
      foreach my $c ( 1 .. $count ) {
        analize_meta_processors($subMeta,
                                "$cname_post_fix$c",
                                sub {
                                  my $innerSub = shift;

                                  my $newSub = sub {
                                    my ($row, $value, $input_row) = @_;

                                    return if (not $value) or ($value eq "");

                                    $row->{$entry->{name}} = []
                                      if not exists $row->{$entry->{name}};

                                    while ( @{$row->{$entry->{name}}} < $c) {
                                      push @{$row->{$entry->{name}}}, {};
                                    }

                                    $innerSub->(${$row->{$entry->{name}}}[$c - 1], $value, $input_row);
                                  };

                                  $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                                  return $newSub;
                                });
      }
    } elsif ($entry->{customattr} =~ /split\s*\(\s*'([^']+)'\s*\)/) {
      my $sep = $1;
      analize_meta_processors($subMeta,
                              $cname_post_fix,
                              sub {
                                my $innerSub = shift;

                                my $newSub = sub {
                                  my ($row, $value, $input_row) = @_;

                                  my @values = split /$sep/, $value;
                                  return if not @values;

                                  $row->{$entry->{name}} = []
                                    if not exists $row->{$entry->{name}};

                                  my $seq = $row->{$entry->{name}};

                                  foreach my $pos (0 .. $#values) {
                                    push @{$seq}, {} if $pos > $#{$seq};

                                    $innerSub->(${$seq}[$pos], $values[$pos], $input_row);
                                  }
                                };

                                $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                                return $newSub;
                              });
    } else {
      print("$entry->{name} seq-type $entry->{customattr} is unknown!\n");
    }
  } else {
    if (is_entry_basic_type($entry)) {
      return if not exists $entry->{cname};

      my $input_col_name = $entry->{cname} . $cname_post_fix;

      my $col_fun = calc_col_fun($entry->{cname}, $entry->{name}, $entry->{customattr});

      $col_fun = $col_fun_derator->($col_fun)
        if defined $col_fun_derator;

      add_col_processor($input_col_name, $col_fun);
      return;
    }

    my $subMeta = $metaLib{$entry->{type}};
    if (not $subMeta) {
      print("$entry->{name} ref type $entry->{type} is unknown!\n");
      return;
    }

    analize_submeta($subMeta, $cname_post_fix, $entry, $col_fun_derator);
  }
}

sub analize_submeta {
  my ($subMeta, $cname_post_fix, $entry, $col_fun_derator) = @_;

    analize_meta_processors($subMeta,
                            $cname_post_fix,
                            sub {
                              my $innerSub = shift;

                              my $newSub = sub {
                                my ($row, $value, $input_row) = @_;

                                if (not exists $row->{$entry->{name}} ){
                                  my $subRow = {};
                                  $innerSub->($subRow, $value, $input_row);
                                  if (keys %{$subRow}) {
                                    $row->{$entry->{name}} = $subRow;
                                  }
                                }
                                else {
                                  $innerSub->($row->{$entry->{name}}, $value, $input_row);
                                }
                              };

                              $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                              return $newSub;
                            });
}

sub analize_meta_processors {
  my ($meta, $cname_post_fix, $col_fun_derator) = @_;

  my $type = $meta->{meta_type};

  if ($type eq "struct") {
    if (ref($meta->{entry}) eq "ARRAY") {
      foreach my $entry (@{$meta->{entry}}) {
        analize_entry_processor_struct($meta, $entry, $cname_post_fix, $col_fun_derator);
      }
    }
    elsif (ref($meta->{entry}) eq "HASH") {
      analize_entry_processor_struct($meta, $meta->{entry}, $cname_post_fix, $col_fun_derator);
    }
  }
  else {
    if (ref($meta->{entry}) eq "ARRAY") {
      foreach my $entry (@{$meta->{entry}}) {
        analize_entry_processor_union($meta, $entry, $cname_post_fix, $col_fun_derator);
      }
    }
    elsif (ref($meta->{entry}) eq "HASH") {
      analize_entry_processor_union($meta, $meta->{entry}, $cname_post_fix, $col_fun_derator);
    }
  }
}

analize_meta_processors($metaLib{$metaName}, "");

my $parser =  Spreadsheet::ParseExcel->new();
my $workbook = $parser->parse($inputFile);
if ( ! defined $workbook ) { die $parser->error(), ".\n"; }

my $sheet = $workbook->worksheet($inputSheet);
if ( ! defined $sheet ) { die "sheet $inputSheet not exist in $inputFile!"; }

my ( $row_min, $row_max ) = $sheet->row_range();
my ( $col_min, $col_max ) = $sheet->col_range();

if ( $row_max < 0 || $row_min >= $row_max ) { die "sheet $inputSheet row range error!"; }
if ( $col_max < 0 || $col_min >= $col_max ) { die "sheet $inputSheet col range error!"; }

my %tableHead = ();
foreach my $colPos ($col_min .. $col_max) {
  my $colName;
  my $cell = $sheet->get_cell($row_min, $colPos);
  if ($cell) {
    $colName = $cell->value();
  }

  $tableHead{$colPos} = $colName;
}

my @table;

foreach my $rowPos ( $row_min + 1 .. $row_max ) {
  my %row;

  my %input_row;
  foreach my $colPos ( $col_min .. $col_max ) {
    my $cell = $sheet->get_cell($rowPos, $colPos);
    next if not defined $cell;

    my $colName = $tableHead{$colPos};
    next if not defined $colName;

    $input_row{$colName} = $cell->value();
  }

  foreach my $colName ( keys %input_row ) {
    next if not exists $input_col_processors{$colName};

    foreach my $processor ( @{$input_col_processors{$colName}} ) {
      $processor->(\%row, $input_row{$colName}, \%input_row);
    }
  }

  if (scalar(%row)) {
    if (exists ($row{is_valid}) ) {
      if ($row{is_valid}) {
        delete ($row{is_valid});
        push @table, \%row;
      }
    } else {
      push @table, \%row;
    }
  }
}

my @no_emptty_role_table = ();
foreach my $row (@table) {
  foreach my $value ( values %{$row} ) {
    if ($value !~ /^\s*$/) { #goto next row if not empty
      push @no_emptty_role_table, $row;
      last;
    }
  }
}

open(my $output, '>::encoding(utf8)', $outputFile) or die "open output file $outputFile fail $!";
print $output Dump( \@no_emptty_role_table );

1;
