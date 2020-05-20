用crass从Majiro的.arc封包中解出来的bmp根据合成方式的不同大概可以分成3类：

1)人物表情图
由24位rgb人物表情基本图和8位色mask图合成

2)CG、背景图
由24位基本图和24位差分图(带有大量红色区域)合成

3)立绘图
由24位基本图和8位色mask图以及24位差分图(带有大量红色区域)合成
注意:
一组立绘图的第一张不含差分图

·Majiro_mask_me的用法
Majiro_mask_me.exe -s 基本图 -m mask图 -d 差分图 -o 保存目录 -r -b 背景色 -f 过滤色

·合成示例：
图源下载后解压到Majiro_mask_me所在的目录，然后运行test.cmd即可。
http://disk.iqho.com/mcncc.php?Mcncc=M-n0uLjXiZeP
http://www.namipan.com/d/example.7z/e87a3fe4ef711496e5917733503475cb0bf12c6a97091100
http://www.zshare.net/download/548094478de2ebfb/

人物表情合成：
Majiro_mask_me.exe -s example\ior_2max.bmp -m example\ior_2max_.bmp
CG图合成:
Majiro_mask_me.exe -s example\101_1.bmp -d example\101_2.bmp
立绘合成(第一张):
Majiro_mask_me.exe -s example\aya_1_1_01.bmp -m example\aya_1_1_01_.bmp
立绘合成(随后的第N张):
Majiro_mask_me.exe -s example\aya_1_1_01.bmp -m example\aya_1_1_02_.bmp -d example\aya_1_1_02.bmp

·其他参数
-o：指定合成结果的保存路径。
-r：不反转alpha值（默认是反转的，因为这个工具是为Majiro定制的）
-b：指定与mask合成后的背景眼色，通常是一个16进制数（默认是0xffffff即白色）
-f：过滤色，通常是一个16进制数（默认是0xff0000即红色，这是Majiro的delta图使用的眼色）
-a：不做alpha blending

ChangeLog:
2009-06-22 20:56 ver 1.1.4 修正带delta合成的错误
2009-05-26 23:36 ver 1.1.3 修正了-s,-m,-d混合使用时delta无效的问题
2009-05-01 10:48 ver 1.1.2 加入-a参数
2009-01-28 10:59 ver 1.1.1 加入-r，-b和-f参数
2009-01-28 00:40 ver 1.1.0 导出alpha
2009-01-27 18:29 ver 1.0.1 加入-o参数以允许指定保存目录
2009-01-27 14:59 ver 1.0.0 第一版发布
