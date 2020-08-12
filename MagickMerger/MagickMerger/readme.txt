MagickMerger是ImageMagick的合成相关命令的GUI

ImageMagick? 是一个免费的创建，编辑，合成图片的软件。

ImageMagick包括一定数量的操纵图像的命令行工具。大多数人大概习惯于使用如同gimp或Photoshop这样的图形用户界面(GUI)来一个一个地编辑图像。事实上，使用GUI经常不方便。假设你想要动态处理来自网络脚本的一张图像或使用相同的动作处理大量图像或多次重复明确的动作处理相同或不用的图像。对于这些处理，使用命令行工具更为适当。(引用自官网)



一，选项卡

程序分Limited Batch Merge(有限批量合成)和Easy Manual Merge(简单手动合成)两大功能，有限自动合成能够按照给定的正则表达式批量生成合成命令，简单手动合成能够对单个文件进行指定合成，下分Alpha Blending，Transparent Blitting，Mask Addition，Page Cropping，Canvas Creation五大功能。 



二，有限批量合成(Limited Batch Merge)

1，合成设置(Merge Setting)

在Source Directory与Destination Directory设置源目录与目标目录。

Regular Expression里选择过滤用的正则表达式，默认有六个配置，分别是Alpha Blending，Alpha Blending (Geometry)，Transparent Blitting，Transparent Blitting (Geometry)，Mask Addition，Page Cropping。选好配置后点击右边的Config会出现Profile Option窗口，具体设置见Profile Option部分。

下面的就是匹配表格，设置好Source Directory，Destination Directory，Profile Option后，选择Filter(过滤源目录下的匹配正则成功的图像，同时显示Pre-Information(预信息)，并用红色提示匹配错误的图像，而绿色代表匹配正确)。过滤完成后还可以进行后处理，Erase(剔除选中的图像，方便生成有效命令)，Clear(清空所有的图像或反选选中的图像)。

2，配置选项(Profile Option)

实际是Regular Expression(Regex/正则表达式)设置，这部分需要使用者懂得正则表达式，否则难以操作。Match Regex(匹配用正则表达式)，用来匹配源图像，这是最重要的正则表达式，是必须的，如果此正则表达式不符合实际，则配置基本无效。Catch Regex(捕获用正则表达式)，如果Match Regex能匹配到图像，将会激活Catch Regex，捕获到符合Catch Regex的合成需要的图像。Merge Regex(合成用正则表达式)， 与Catch Regex一样，如果Match Regex能匹配到图像，将会激活Merge Regex，构造出符合Match Regex的合成后的图像。Command Regex(命令用正则表达式)， 与Catch Regex和Merge Regex一样，如果Match Regex能匹配到图像，将会激活Command Regex，构造出符合Command Regex的合成命令。选中Custom(自定义)，将会新建空白配置，使用者可以完全自定义所要的正则表达式。需要注意的是，通常在正则表达式的反向引用\n，在这里需要变成$n；并且在Command Regex里可以使用\n来构造多条命令。

3， 状态栏

鉴于过滤的复杂性特别提供状态栏信息以备查看，当Filter，Erase时将会显示简单的信息。



三，简单手动合成(Easy Manual Merge)

1，Alpha Blending
Overlay指定差分图象，Background指定背景图象，Result指定结果图象。Geometry用来定位差分图象在背景图象的位置，X/Y-Offset分别指定偏移坐标。

2，Transparent Blitting
Overlay指定差分图象，Background指定背景图象，Result指定结果图象。Transparent用来确定透明颜色。Geometry用来定位差分图象在背景图象的位置，X/Y-Offset分别指定偏移坐标。

3，Mask Addition
Mask指定遮罩图象，Negate指定是否反色遮罩图象，Background指定背景图象，Result指定结果图象。

4，Page Cropping
Source指定源图象，Destination指定目标图象。Precent用来确定子图象占源图象的百分比，Width/Height分别指定子图象宽度/高度占源图象的百分比。Tile用来确定子图象对源图象的横纵数量，Row/Column分别指定源图象包含子图象的行/列数。Precent和Tile是等效的。

5，Canvas Creation
Destination指定目标图象。Size用来确定图象尺寸，Specify指定具体尺寸，Width代表宽度，Height代表高度；Source指定与选中的图象相同尺寸，这两个选项只能二选一。Backgound用来确定背景颜色。Alpha Set用来确定是否添加Alpha。

6，控制台信息(Console Info)

在Run Command(运行命令)方式里若命令正确执行，将不会显示任何信息，反之会显示错误信息。此外可灵活使用Save as Log File(另存为日志文件)或Clear all Content(清空所有信息)。



四，命令处理(Command Process)

表格显示出匹配成功的信息后，选中Generate Command，将会生成批量命令。生成好后可以Copy to Clipboard(保存到剪贴板)或Save as Batch File(另存为bat文件)或Clear all Content(清空所有命令)。

命令运行方式按照Limited Auto-Merge与Easy Manual-Merge分成两大类。Limited Auto-Merge：Silence Launch(静默运行)与Detail Launch(详细运行)，在Silence Launch方式里程序会最小化只可通过任务栏图标查看进度并对合成进行操作；而在Detail Launch方式里会新建Process Monitor进度窗口以查看合成进度与控制台错误信息并能操纵合成也可在完毕后Save as Log File(另存为日志文件)。Easy Manual-Merge：Run Command，在Run Command方式里程序会自动运行命令。



五，菜单

-Task(任务)
           -New(刷新)
                        -Current(当前)
                        -All(全部)
           -Quit(退出)
-Help(帮助)
           -ImageMagick Official(ImageMagick官网)
           -Check Version(检查版本)
           -Regular Expression(正则语法)
           -About(关于)



六，任务栏

-Suspend(暂停)
-Resume(继续)
-Terminate(终止)



七，程序清单

MagickMerger.jar	必备，Java GUI
ImageMagick		必备，核心程序
preference.xml		必备，属性文件
profiles		可选，配置目录
readme.txt		可选，说明文件
error.log		可选，错误日志



八，依赖关系

Java Runtime Environment (Java SE 7)  Windows x86，下载见http://www.oracle.com/technetwork/java/javase/downloads/index.html
ImageMagick (Windows Binary)，下载见http://www.imagemagick.org/script/binary-releases.php#windows，版本要求6.7.2或其以上
