#!/usr/bin/perl
# merge_pimg.pl, v1.01 2012/03/30
# coded by asmodean

# contact: 
#   web:   http://asmodean.reverse.net
#   email: asmodean [at] hush.com
#   irc:   asmodean on efnet (irc.efnet.net)

# This script merges layers extracted from PSB (*.pimg) composites used in DRACU-RIOT.
# Extract the composites and convert to PNG before using this.

die "usage: $0 <input+layers.txt>\n"
  unless ($#ARGV == 0);

$filename = $ARGV[0] =~ m@(.*)\+pimg\+layers@;
$prefix = $1;

sub get_value {
  $line = <>;
  
  if ($line =~ m@.*:\s*(\S+)@) {  
    return $1;
  } else {
    return "";
  }
}

$image_width  = get_value();
$image_height = get_value();
get_value();

while (1) {
  $name     = get_value();
  
  last unless $name;
  
  $layer_id = get_value();
  $width    = get_value();
  $height   = get_value();
  $left     = get_value();
  $top      = get_value();
  
  while (get_value() ne "") {}

  $layers{$layer_id}{"name"}      = $name;  
  $layers{$layer_id}{"layer_id"}  = $layer_id;
  $layers{$layer_id}{"width"}     = $width;
  $layers{$layer_id}{"height"}    = $height;
  $layers{$layer_id}{"left"}      = $left;
  $layers{$layer_id}{"top"}       = $top;
  
  $last_layer_id = $layer_id;
}

for $layer_id (keys(%layers)) {
  $name   = $layers{$layer_id}{"name"};  
  $width  = $layers{$layer_id}{"width"};
  $height = $layers{$layer_id}{"height"};
  $left   = $layers{$layer_id}{"left"};
  $top    = $layers{$layer_id}{"top"};
  
  if ($layer_id != $last_layer_id) { 
    $base_name   = $layers{$last_layer_id}{"name"};  
    $base_width  = $layers{$last_layer_id}{"width"};
    $base_height = $layers{$last_layer_id}{"height"};
    $base_left   = $layers{$last_layer_id}{"left"};
    $base_top    = $layers{$last_layer_id}{"top"};  
  
    system("convert",
           "-size", "${image_width}x${image_height}",
           "-page", "+${base_left}+${base_top}", "${prefix}+pimg+${last_layer_id}.png",
           "-page", "+${left}+${top}", "${prefix}+pimg+${layer_id}.png",
           "-mosaic",
           "${prefix}+${base_name}+${name}.png");
  } else {
    system("convert",
           "-size", "${image_width}x${image_height}",
           "-page", "+${left}+${top}", "${prefix}+pimg+${layer_id}.png",
           "-mosaic",
           "${prefix}+${name}.png");
  }
    
}
