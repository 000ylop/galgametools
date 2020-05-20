#!/usr/bin/perl
# merge_yatevt.pl, v1.0 2010/09/28
# coded by asmodean

# contact: 
#   web:   http://asmodean.reverse.net
#   email: asmodean [at] hush.com
#   irc:   asmodean on efnet (irc.efnet.net)

# This tool merges event images used by Yatagarasu in ObsceneGuild.
# Extract images and scripts into the directory where you run this.

$settings_file = "scriptSettings.lua";
open(SETTINGS, $settings_file) or die "${settings_file}: $!\n";

while (<SETTINGS>) {
  if (m/"(.*?):\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)/) {
    $posdb{$1} = "+$2+$3";
  }
}

close(SETTINGS);

mkdir("merged");

@evts = glob("*.evt");

for $e (@evts) {      
  next if ($e eq "record.evt");
  
  open(EVT, "$e") or die "${e}: $!\n";
  
  while (<EVT>) {
    $line = $_;
    
    if ($line =~ m/SimpleEvent.*file="([^"]+)"/) {
      $file = $1;      
      $base = $file;
      $base =~ s@(.*)\..*@$1.png@;
      
      next unless (-f $base);
      
      @cmd = ("convert", 
              "-background", "transparent",
              "-page", "+0+0", $base);
              
      $out = $base;
      $out =~ s@\.png@@;      
      
      %df_pos = {};
      while ($line =~ m/df(.)_pos="(\d+),(\d+)"/g) {
        $df_pos{$1} = "+$2+$3";
      }
      
      $haspart = 0;
      
      while ($line =~ m/df(.)="([^"]+)"/g) {
        $haspart = 1;
        
        $index = $1;
        $part  = $2;
                        
        if (exists($df_pos{$index})) {
          $pos = $df_pos{$index};
        } elsif (exists($posdb{$part})) {
          $pos = $posdb{$part};
        } else {
          $pos = "+0+0";
        }
               
        push(@cmd, "-page", $pos, $part);
        
        $out .= "+$part";
        $out =~ s@\.png@@;  
        
        $used{$part} = 1;
      }
      
      next unless ($haspart);
      
      push(@cmd, "-mosaic", "merged/${out}.png");      
      
      $used{$base} = 1;
       
      system(@cmd) unless (exists($done{$out}));
      $done{$out} = 1;
    }
  }
  
  close(EVT);
}

mkdir("used");
for $f (keys(%used)) {
  rename($f, "used/$f");
}
