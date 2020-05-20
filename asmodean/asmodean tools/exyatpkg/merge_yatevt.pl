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
mkdir("done");

# Fix a corrupt image so IM will handle it...
system("pngcrush", "-fix", "vg48_00_0_l1.png", "temp.png");
if (-f "temp.png") {
  unlink("vg48_00_0_l1.png");
  rename("temp.png", "vg48_00_0_l1.png");
}

@evts = glob("*.evt");
#@evts = ("sce01_01_1.evt");

for $e (@evts) {
  print "$e\n";
  
  $insert_no = -1;
  $liquid_no = -1;
  
  INSERT: for ($insert_no = -1; $insert_no < 10; $insert_no++) {
  LIQUID: for ($liquid_no = -1; $liquid_no < 10; $liquid_no++) {
  
  open(EVT, "$e") or die "${e}: $!\n";
  
  while (<EVT>) {
    if (m/SimpleEvent.*file="([^"]+)"/) {
      $file = $1;
      
      next if ($file !~ /\./);
      
      if (!-f $file) {
        print "${e}: Reference to missing base skipped: $file\n";
        next;
      }     
            
      $file =~ m/^(.*)\./;
      $base = $1;
      
      $file =~ m/^(.*?)_/;
      $prefix = $1;           
      
      @layers = ("${base}.png");
      push (@layers, "${base}_l1.png") if (-f "${base}_l1.png");
      push (@layers, "${base}_l2.png") if (-f "${base}_l2.png");     
      
      while (m/df(\d+)="([^"]+)"/g) {
        $type  = $1;
        $index = $2;
        
        $is_insert = $index =~ 'E_LAST_INSERT_NO';
        $is_liquid = $index =~ 'E_LAST_LIQUID_NO';
        
        $index = $insert_no if ($is_insert);
        $index = $liquid_no if ($is_liquid);
        
        next if ($index == -1);
        
        $index =~ s/h$/_h/;
        
        $layer_file = sprintf("${prefix}_%02d_${index}.png", $type);
        
        next INSERT if (!-f $layer_file && $is_insert);
        next LIQUID if (!-f $layer_file && $is_liquid);
        
        if (!-f $layer_file) {
          print "${e}: Reference to missing layer (composite skipped): $layer_file\n";
          next;
        }
                
        push(@layers, $layer_file);
      }
      
      @cmd = ("convert", "-background", "transparent");
      $out = "";
              
      for $l (@layers) {
        $out =~ s@\.png@@;
        
        if ($out eq "") {
          $out = $l;
        } else {
          $out .= "+$l";
        }
        
        $pos_base = $l;
        $pos_base =~ s/.png//;
        $pos_base =~ s/_h$//;
        
        if (exists($posdb{$pos_base})) {
          $pos = $posdb{$pos_base};
        } else {
          $pos = "+0+0";
        }
        
        push(@cmd, "-page", $pos, $l);
        $used{$l} = 1;
      }
      
      push(@cmd, "-flatten", "merged/$out");
       
      system(@cmd) unless (exists($done{$out}));
      $done{$out} = 1;
    }
  }
  
  close(EVT);
  }}
}

mkdir("used");
for $f (keys(%used)) {
  rename($f, "used/$f");
}
