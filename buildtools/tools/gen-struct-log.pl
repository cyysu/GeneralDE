use strict;
use Encode;
use XML::Simple;
use Getopt::Long;
use Data::Dumper;

binmode(STDOUT, ":utf8");

my $inputFile;
my $outputH;
my $outputC;
my $prefix;
my @categories;

GetOptions("input-file=s" => \$inputFile,
           "output-h=s"   => \$outputH,
           "output-c=s"   => \$outputC,
           "prefix=s" => \$prefix,
           "category=s" => \@categories);

$inputFile = decode("utf8", $inputFile);
$outputH = decode("utf8", $outputH);
$outputC = decode("utf8", $outputC);

sub entry_is_auto_gen {
  my $v = shift;

  return 0 if not defined $v;
  return 1 if $v =~ m/^-.*/;
  return 1 if $v =~ m/^\@.+\|.*/;
  return 0;
}

sub generate_c_entry_to_string_datetime_def {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  print $output "    struct tm tm_" . $entry_name . ";\n";
  print $output "    char buf_" . $entry_name . "[60];\n";
  print $output "\n";
}

sub generate_c_entry_to_string_datetime_init {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  print $output "    localtime_r(&" . $entry_name . ", &tm_". $entry_name . ");\n";
  print $output "    strftime(buf_" . $entry_name . ",sizeof(buf_" . $entry_name . "),\"%Y-%m-%d %T\",&tm_" . $entry_name . ");\n";
  print $output "\n";
}

my $type_info_def = { int8 => { formator => "\"FMT_INT8_T\"", type => "int8_t" }
               , uint8 => { formator => "\"FMT_UINT8_T\"", type => "uint8_t" }
               , int16 => { formator => "\"FMT_INT16_T\"", type => "int16_t" }
               , uint16 => { formator => "\"FMT_UINT16_T\"", type => "uint16_t" }
               , int32 => { formator => "\"FMT_INT32_T\"", type => "int32_t" }
               , uint32 => { formator => "\"FMT_UINT32_T\"", type => "uint32_t" }
               , int64 => { formator => "\"FMT_INT64_T\"", type => "int64_t" }
               , uint64 => { formator => "\"FMT_UINT64_T\"", type => "uint64_t" }
               , string => { formator => "\%s", type => "const char *" }
               , datetime => { formator => "\%s", type => "time_t"
                               , "to-string-init" => \&generate_c_entry_to_string_datetime_init
                               , "to-string-def" => \&generate_c_entry_to_string_datetime_def }
};

my $xml_lib_source = XMLin($inputFile, KeyAttr => {struct => 'name', union => 'name'}, ForceArray => [ 'struct', 'union' ]);

sub generate_h_entry {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  return if entry_is_auto_gen($entry->{desc});

  my $type_info = $type_info_def->{$entry->{type}};

  print $meta_name . "." . $entry_name . ": type '" . $entry->{type} ."' not support!\n" and return
    if not defined $type_info;

  print $output "\n    , $type_info->{type} $entry_name";
}

sub generate_c_entry_formator {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  if (defined $entry->{desc} and $entry->{desc} =~ m/^-.*/) {
    print $output "|0";
  }
  elsif (defined $entry->{desc} and $entry->{desc} =~ m/^\@(.+)\|.*/) {
    my $type_info = $type_info_def->{$entry->{type}};

    print $output "|0" if not defined $type_info;

    print $output "|$type_info->{formator}";
  }
  else {
    my $type_info = $type_info_def->{$entry->{type}};

    print $output "|0" if not defined $type_info;

    print $output "|$type_info->{formator}";
  }
}

sub generate_c_entry_arg {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  if (defined $entry->{desc} and $entry->{desc} =~ m/^-.*/) {
  }
  elsif (defined $entry->{desc} and $entry->{desc} =~ m/^\@(.+)\|.*/) {
    my $ref_arg = $1;
    my $type_info = $type_info_def->{$entry->{type}};

    return if not defined $type_info;

    if (exists $type_info->{"to-string-init"}) {
      print $output "        , buf_" . $entry_name . "\n";
    }
    else {
      print $output "        , " . $ref_arg. "\n";
    }
  }
  else {
    my $type_info = $type_info_def->{$entry->{type}};

    return if not defined $type_info;

    if (exists $type_info->{"to-string-init"}) {
      print $output "        , buf_" . $entry_name . "\n";
    }
    else {
      print $output "        , " . $entry_name. "\n";
    }
  }
}

sub generate_c_entry_buf_def {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  return if defined $entry->{desc} and $entry->{desc} =~ m/^-.*/;

  my $type_info = $type_info_def->{$entry->{type}};
  return if not defined $type_info;

  my $fun = $type_info->{"to-string-def"};
  return if not defined $fun;

  $fun->($meta_name, $meta, $entry_name, $entry, $output);
}

sub generate_c_entry_buf_init {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  if (defined $entry->{desc} and $entry->{desc} =~ m/^-.*/) {
  }
  elsif (defined $entry->{desc} and $entry->{desc} =~ m/^\@(.+)\|.*/) {
    my $ref_arg = $1;
  }
  else {
    my $type_info = $type_info_def->{$entry->{type}};
    return if not defined $type_info;

    my $fun = $type_info->{"to-string-init"};
    return if not defined $fun;

    $fun->($meta_name, $meta, $entry_name, $entry, $output);
  }
}

sub foreach_entry {
  (my $meta_name, my $output, my $fun) = @_;

  my $meta = $xml_lib_source->{struct}->{$meta_name};

  if (ref($meta->{entry}) eq "ARRAY") {
    foreach my $entry (@{$meta->{entry}}) {
      $fun->($meta_name, $meta, $entry->{name}, $entry, $output);
    }
  }
  elsif (ref($meta->{entry}) eq "HASH") {
    my $entry = $meta->{entry};
    $fun->($meta_name, $meta, $entry->{name}, $entry, $output);
  }
}

sub generate_h {
  my $output = shift;

  print $output "#ifndef _STRUCT_LOG_". $prefix . "_H_INCLEDE_\n";
  print $output "#define _STRUCT_LOG_" . $prefix . "_H_INCLEDE_\n";
  print $output "#include \"cpe/pal/pal_time.h\"\n";
  print $output "#include \"gd/app/app_types.h\"\n";
  print $output "\n";

  print $output "#ifdef __cplusplus\n";
  print $output "extern \"C\" {\n";
  print $output "#endif\n";

  if (exists $xml_lib_source->{struct}) {
    foreach my $meta_name ( keys %{$xml_lib_source->{struct}} ) {
      my $meta = $xml_lib_source->{struct}->{$meta_name};

      print $output "\nvoid log_" . $prefix . "_" . $meta_name . "(gd_app_context_t app";

      foreach_entry($meta_name, $output, \&generate_h_entry);

      print $output ");\n";
    }
  }

  print $output "\n";
  print $output "#ifdef __cplusplus\n";
  print $output "}\n";
  print $output "#endif\n";
  print $output "\n";
  print $output "#endif\n";
}

sub generate_c {
  (my $output, my $categories) = @_;

  print $output "#include \"cpe/pal/pal_stdio.h\"\n";
  print $output "#include \"cpe/pal/pal_time.h\"\n";
  print $output "#include \"gd/app/app_context.h\"\n";

  if (exists $xml_lib_source->{struct}) {
    foreach my $meta_name ( keys %{$xml_lib_source->{struct}} ) {
      print $output "\nvoid log_" . $prefix . "_" . $meta_name . "(gd_app_context_t app";
      foreach_entry($meta_name, $output, \&generate_h_entry);
      print $output ")\n";
      print $output "{\n";

      foreach_entry($meta_name, $output, \&generate_c_entry_buf_def);
      foreach_entry($meta_name, $output, \&generate_c_entry_buf_init);

      foreach my $category (@{ $categories }) {
        print $output "    CPE_INFO(\n";
        print $output "        gd_app_named_em(app, \"$category\"),\n";
        print $output "        \"$meta_name";


        foreach_entry($meta_name, $output, \&generate_c_entry_formator);
        print $output "\"\n";


        foreach_entry($meta_name, $output, \&generate_c_entry_arg);

        print $output "    );\n";
        print $output "\n";
      }

      print $output "}\n";
    }
  }
}


open(my $output_h, '>::encoding(utf8)', $outputH) or die "open output head file $outputH fail $!";
generate_h($output_h);
close($output_h);

open(my $output_c, '>::encoding(utf8)', $outputC) or die "open output file $outputH fail $!";
generate_c($output_c, \@categories);
close($output_c);

