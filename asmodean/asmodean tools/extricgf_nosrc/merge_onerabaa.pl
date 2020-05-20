#!/usr/bin/perl
# merge_onerabaa.pl, v1.0 2010/12/27
# coded by asmodean

# contact: 
#   web:   http://asmodean.reverse.net
#   email: asmodean [at] hush.com
#   irc:   asmodean on efnet (irc.efnet.net)

# This script merges event images found in One‚ç‚Î[.
# Assumes that images are converted to PNG with alpha.

mkdir("merged");
mkdir("used");

@parts = glob("E*[A-Z]0+*.png");

for $p (@parts) {
  $p =~ m@((.*)....)\+x(\d+)y(\d+)@;
  $name   = $1;
  $prefix = $2;
  $x      = $3;
  $y      = $4;
  
  @bases = glob("${prefix}??00.png");
  
  for $base (@bases) {  
    $out = "${base}+${name}";
    $out =~ s@\.png@@g;
    
    system("convert",
           "-background", "transparent",
           "-page", "+0+0", $base,
           "-page", "+${x}+${y}", $p,
           "-mosaic",
           "merged/${out}.png");
           
    push(@used, $base, $p);
  }
}

for $f (@used) {
  rename($f, "used/$f");
}
