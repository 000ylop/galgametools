bmp extracted with crass from Majiro's .arc archive, according to difference of blending mode probably can be distinguished to three types:

1)character face image
blending by 24-bit rgb character face basic image and 8-bit mask image

2)CG, background image
blending by 24-bit basic image and 24-bit delta image(with amounts of red area)

3)stand image
blending by 24-bit basic image, 8-bit mask image and 24-bit delta image(with amounts of red area)
notice:
first image of a group stand image isn't delta image

·usage of Majiro_mask_me.exe
Majiro_mask_me.exe -s basic image -m mask image -d delta image -o save directory

·blending sample: 
download the example images from the following links, then unpack it to Majiro_mask_me folder, then run test.cmd to see the result.
http://disk.iqho.com/mcncc.php?Mcncc=M-n0uLjXiZeP
http://www.namipan.com/d/example.7z/e87a3fe4ef711496e5917733503475cb0bf12c6a97091100
http://www.zshare.net/download/548094478de2ebfb/

blending character face:
Majiro_mask_me.exe -s example\ior_2max.bmp -m example\ior_2max_.bmp
blending CG image:
Majiro_mask_me.exe -s example\101_1.bmp -d example\101_2.bmp
blending stand image(first image):
Majiro_mask_me.exe -s example\aya_1_1_01.bmp -m example\aya_1_1_01_.bmp
blending stand image(following N images):
Majiro_mask_me.exe -s example\aya_1_1_01.bmp -m example\aya_1_1_02_.bmp -d example\aya_1_1_02.bmp

·other parameter
-o: specify save path for blending result
-r: no inverted alpha(default is inverted, because the tool is custom for Majiro)
-b: specify background color after blending with mask, usually is a hexadecimal number(default is 0xffffff, aka white)
-f: filter color, usually is a hexadecimal number(default is 0xff0000, aka red, this is the color used by Majiro's delta image)
-a: disable alpha blending

ChangeLog:
2009-06-22 20:56 ver 1.1.4 fix incorrect image composition with delta
2009-05-26 23:36 ver 1.1.3 fix invalid delta while specify -s,-m,-d together
2009-05-01 10:48 ver 1.1.2 add -a parameter
2009-01-28 10:59 ver 1.1.1 added -r, -b and -f parameter
2009-01-28 00:40 ver 1.1.0 export alpha
2009-01-27 18:29 ver 1.0.1 added -o parameter to allow specify save folder
2009-01-27 14:59 ver 1.0.0 1st version released
