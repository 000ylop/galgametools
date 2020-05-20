#!/usr/bin/perl

# mergebssc.pl, v1.0 2007/07/05
# coded by asmodean

# contact:
#   web:   http://plaza.rakuten.co.jp/asmodean
#   email: asmodean [at] hush.com
#   irc:   asmodean on efnet (irc.efnet.net)

# This script merges character images parts extracted from some BSS-Composition
# images.  Requires ImageMagick's convert tool on the path...

use File::Compare;

mkdir("merged");

@bases = glob("*00+00000_*.png");

sub do_merge {
  my $base   = shift(@_);
  my $base_x = shift(@_);
  my $base_y = shift(@_);
  my $part   = shift(@_);
  my $out    = shift(@_);

  $part =~ m/(\d+)x_(\d+)y/;
  my $x = $1 - $base_x;
  my $y = $2 - $base_y;

  system("convert",
	 $base,
	 "-draw", "image Over $x,$y 0,0 $part",
	 $out);

  return $out;
}

for $b (@bases) {
  $base = $b;

  $base =~ m/^(.*)00\+.*?(\d+)x_(\d+)y/;
  $prefix = $1;
  $x      = $2;
  $y      = $3;

  ($face)  = glob($prefix . "01+00000_*.png");
  ($under) = glob($prefix . "00+00001_*.png");
  ($cloth) = glob($prefix . "00+00002_*.png");

  if ($face ne "") {
    $base = do_merge($base, $x, $y, $face, "merged/$face");
  }

  if ($under ne "") {
    $temp = do_merge($base, $x, $y, $under, "merged/$under");

    if ($cloth ne "") {
      $out2 = $temp;
      $out2 =~ s/\.png/b.png/;
      do_merge($temp, $x, $y, $cloth, $out2);

      if (!compare($temp, $out2)) {
	unlink($out2);
      }
    }
  } else {
    print "$b : no under?\n";
  }

  if ($cloth ne "") {
    do_merge($base, $x, $y, $cloth, "merged/$cloth");
  } else {
    print "$b : no cloth?\n";
  }
}
