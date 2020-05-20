XP3 Dumper GUI

作者：clowwindy

本程序是对Resty做的xp3dumper等一系列xp3提取工具的GUI封装，提供简单的GUI调用功能。这些工具在xp3dumper目录中。

使用说明：
	1.选择游戏启动exe和执行exe，它们的区别在于前者可能只是一个引导用的破解补丁（如sinsemilla_boot.exe），后者才会是游戏运行时的进程（如sinsemilla.exe）。如果没有这样的引导程序，它们就是同一个exe文件。
	2.选择要解包的xp3文件和保存路径。
	3.如果游戏需要转区才能运行，选择SoraApp可以调用SoraApp（今后版本会支持NTLEA）转区运行游戏，前提是你安装了SoraApp或NTLEA。
	4.点击“提取”，稍等几秒，就会开始提取。如果没有开始提取，请点击取消，再按照5操作。
	5.如果发现没有开始提取，这是因为游戏启动花的时间较长，请将载入插件延时调大一些再尝试。
	
已在中文win xp上测试『黄昏のシンセミア』和『G線上の魔王』的解包。
	
clowwindy
2010-9-24

BUG反馈:
http://bbs.sumisora.org/read.php?tid=10975710

History:
	0.2 在xp3dumper0.12d的基础上，实现判断提取结束功能
	0.1 第一个公开版本
