DirectMuxer是当前最强大的多组合成工具。



一，属性选项(Preference Option)

第一次运行会弹出Preference Option窗口，共有General(通用)，Muxing(合成)两个属性可供设置。
	General：
		子选项卡Image Access可以设置图象存取路径，默认是当前目录。
		子选项卡Image Preview可以设置是否激活图象预览，默认是激活。
		子选项卡Group Permutation可以设置是否激活合成组排列，即是否保留直积合成过程产生的图象，默认设置是激活。
		子选项卡Image Naming可以通过定义前缀，分隔符，后缀来设置源文件名的连接格式，默认设置是Prefix，Suffix为空，Delimiter为_。
	Muxing共有Build-In(内建)属性可供设置。
		Build-In：
			子选项卡Image Output可以设置合成图的保存路径，默认设置为空，即当前目录。
			子选项卡Coordinate Dependence可以设置是否激活坐标文件名自动捕获，对应的匹配设置(asmodean, Crass(RioShiina), Crass(AI6WIN))，是否激活重设背景尺寸(限制基本图或差分图含Alpha)，默认设置是激活Auto Capture，选择asmodean作为Match Pattern，激活Resize Canvas。
			子选项卡Concurrent Computation可以设置是否激活CPU加速并设置多线程数量，默认设置是激活CPU Accelerate和设置Thread Number最大。
		此外Build-In共有Alpha Blending，Transparent Blitting，Opaque Override，Complex Composite四个属性可供设置。
			子选项卡Naming Trimming可以设置是否剔除合成图文件名上的标识，默认设置是禁用。
			Alpha Blending：
				子选项卡Alpha Preservation可以设置合成图Alpha保存模式，共分成Opaque(合成图不含Alpha)，Transparent(合成图含Alpha)，Adaptive(自适应，若基本图或差分图含Alpha则合成图含Alpha，若基本图和差分图不含Alpha则合成图不含Alpha)三种模式，默认设置是Adaptive。
				子选项卡Premultiplied Alpha可以设置是否把基本和差分图当作预乘Alpha来处理，默认设置是禁用。
			Transparent Blitting(限制差分图不含Alpha并小于基本图尺寸)：
				子选项卡Transparent Color可以设置差分图对比基本图的透明色，默认设置是黑色。
			Opaque Override(限制差分图不含Alpha并小于基本图尺寸)：
				子选项卡Offset Prediction可以设置是否激活坐标预测；Photoshop Preprocessing可以设置是否激活Photoshop预处理，可创建JSX用于生成PSD来手动修正图象，默认设置双双激活。
			Complex Composite(限制基本图和差分图含Alpha)：
				子选项卡Layer Composition可以设置每组图象默认的图层合成类型，共分为Normal(正常), LinearDodge(线性减淡), LinearBurn(线性加深), Multiply(正片叠底), Screen(滤色), Overlay(叠加), HardLight(强光), SoftLight(柔光), ColorDodge(颜色减淡), ColorBurn(颜色加深), Lighten(变亮), Darken(变暗), Difference(差值), Exclusion(排除)14种。默认是Normal，等价Alpha Blending。
以后在使用过程中可随时对这些属性进行修改，也可还原成Default默认设置。



二，主视图

左边的列表框用来显示要合成的图象列表，右边的群组用来分组图象集合，中间的按钮用来操纵群组，下面的状态栏用来提供操作列表框和群组产生的简单信息。

1，列表框

在列表框上有文本框，可以根据输入的正则表达式显示旁边的"."按钮指定的目录里的PNG/BMP/TGA/JPG图象。

在列表框显示项上右键会弹出菜单，具有Insert to(插入), Delete(删除)，Deselect(反选), Sort(排序)，Clear(清空)，Auto Grouping(自动分组)六个菜单项。Insert to把列表框里的选中项目插入到指定的表格，其子菜单显示当前群组的表格序号，由此确定插入到哪个表格。Delete删除列表框的选中项目。Deselect反选列表框的选中项目。Sort排序列表框的所有项目。Clear清空列表框的所有项目。Auto Grouping根据选择的规则(CatSystem2)自动分组到群组表格。

2，群组

群组默认有两组图象集合，在任意表格的显示项上右键同样也会弹出菜单，具有Move to(移动), Delete(删除), Deselect(反选), Sort(排序)，Clear(清空)五个菜单项。Move to移动当前表格的选中项目到指定的表格，其子菜单显示当前群组的除本身的表格序号，由此确定移动到哪个表格。Delete删除当前表格的选中项目。Deselect反选当前表格的选中项目。Sort排序当前表格的所有项目。Clear清空当前表格的所有项目。每个表格下面有三个特别控件：文本框使用正则表达式过滤该组表格的项目；图层类型下拉框，用于选择在Complex Composite模式下的合成算法；按钮确认是否激活该组为必选，即不保留该组项目为合成中间结果。

3，操作按钮

下拉框选择当前合成模式，A, T, O, X分别表示Alpha Blending, Transparent Blitting, Opaque Override, Complex Composite。

点号按钮按照下拉框选中的模式直接合成图像。

等号按钮按照下拉框选中的模式列队合成任务。

Ps按钮按照下拉框选中的模式创建Photoshop脚本。

群组(Group)按钮被点击后回会弹出菜单，具有Create(新建), Delete from(删除), Clear all(清空群组)，Restore all(还原群组)，Statistics(统计), Start Computation(开始合成), Enqueue Task(列队任务)，Photoshop Scripting(生成脚本)六个菜单项。Create新建空白表格，递增表格序号。Delete from删除指定的表格，其子菜单显示当前群组的表格序号，由此确定删除哪个表格。Clear all清空群组的所有表格的所有项目。Restore all还原群组的所有表格的所有项目。Statistics统计群组信息包括合成总数和存储为位图时大小。Graphic Computation直接合成图像，其子菜单显示支持的合成模式。Enqueue Task列队合成任务，其子菜单显示支持的合成模式，其中不支持激活Photoshop Preprocessing的Opaque Override。Photoshop Scripting创建Photoshop脚本，可用于生成存PSD，其中不支持Transparent Blitting和激活Offset Prediction或Photoshop Preprocessing的Opaque Override，同时要求图像必须为PNG。

叉号按钮清空群组的所有表格的所有项目。

值得一提的是，列表框与群组都可支持本地拖拽和相互拖拽(复制)，而群组间也可相互拖拽(移动)。(要求PNG/BMP/TGA/JPG属于同一目录)



三，预览视图(Preview)

选中任意组的任意单幅图像，主视图右上角将会出现预览窗口，并在Pre-Composite(预合成)标签页上显示被选中的图像。若选中任意组的任意多幅图像，Post-Composite(后合成)标签页上显示这些图像的最终合成结果。在Post-Composite标签页上，可以通过右键菜单选择不同的合成模式：Alpha Blending, Transparent Blitting, Opaque Override, ComplexComposite，并可选择Save as另存合成图。预览视图支持鼠标滚轮做最适缩放。



四，合成(Mux)

1，内部方法(Build-In Method)
(1)，Alpha Blending
[1]，图像合成(Graphic Computation)
程序会读取群组信息，按照Alpha Blending模式，直接合成图象。
[2]，列队任务(Enqueue Task)
程序会读取群组信息，按照Alpha Blending模式，列队合成任务。
[3]，生成脚本(Photoshop Scripting)
程序会读取群组信息，按照Alpha Blending模式，创建Photoshop脚本。
(2)，Transparent Blitting
[1]，图像合成(Graphic Computation)
程序会读取群组信息，按照Transparent Blitting，直接合成图象。
[2]，列队任务(Enqueue Task)
程序会读取群组信息，按照Transparent Blitting模式，列队合成任务。
(3)，Opaque Override
[1]，图像合成(Graphic Computation)
程序会读取群组信息，按照Opaque Override模式，直接合成图象。
[2]，列队任务(Enqueue Task)
程序会读取群组信息，按照Opaque Override模式，列队合成任务。
[3]，生成脚本(Photoshop Scripting)
程序会读取群组信息，按照Opaque Override模式，创建Photoshop脚本。
(4)，Complex Composite
[1]，图像合成(Graphic Computation)
程序会读取群组信息，按照Complex Composite模式，直接合成图象。
[2]，列队任务(Enqueue Task)
程序会读取群组信息，按照Complex Composite模式，列队合成任务。
[3]，生成脚本(Photoshop Scripting)
程序会读取群组信息，按照Complex Composite模式，创建Photoshop脚本。
(5)，输出路径(Output Directory)
[1]，默认(Default)
[2]，浏览(Browse...)
2，组间排列(Group Permutation)
设置是否激活组间排列。
3，清空群组(Clear Group)
清空群组的所有表格的所有项目。
4，还原群组(Restore Group)
还原群组的所有表格的所有项目。


五，队列(Queue)

所有的合成任务将显示在表格中。主要信息有Method(合成模式), Source(源目录), Destination(目标目录), Progress(进度), Duration(耗时), Status(状态), Log(日志)。

1，操作按钮
开始(Start)：执行表格中的所有状态为等待的合成任务。
清空(Clear)：清空表格中的所有的合成任务。
合成完成(After Computation)：默认是什么都不做(Nothing)，若选择推出(Quit)，则完成所有的合成任务后关闭程序，若选择关机(Shutdown)，则完成所有的合成任务后开始10秒倒计时关机。

2，右键菜单
组信息(Group)：显示表格中选中的合成任务的组信息。
删除(Delete)：清除表格中选中的合成任务。
上移(Up)：上移表格中选中的合成任务。
下移(Down)：下移表格中选中的合成任务。
反选(Deselect)：反选表格中选中的合成任务。
状态(Status)：改变表格中选中的合成任务的状态。若选择等待，则执行；若选择延迟，则不执行。



六，主菜单

-Task(任务)
	-New(刷新)
	-Preference(属性)
	-Quit(退出)
-Mux(合成)
	-Build-In Method(内部方法)
		-Alpha Blending
			-Graphic Computation(图像合成)
			-Enqueue Task(队列任务)
			-Photoshop Scripting(创建脚本)
		-Transparent Blitting
			-Graphic Computation(图像合成)
			-Enqueue Task(队列任务)
		-Opaque Override
			-Graphic Computation(图像合成)
			-Enqueue Task(队列任务)
			-Photoshop Scripting(创建脚本)
		-Complex Composite
			-Graphic Computation(图像合成)
			-Enqueue Task(队列任务)
			-Photoshop Scripting(创建脚本)
		-Output Directory(输出目录)
			-Default(默认)
			-Browse...(浏览)
        	-Group Permutation(组间排列)
        	-Clear Group(清空群组)
		-Restore Group(还原群组)
-Queue(队列)
-Help(帮助)
	-Regular Expression(正则语法)
	-Check Version(检查版本)
	-About(关于)



七，任务栏

-Suspend(暂停)
-Resume(继续)
-Terminate(终止)



八，程序清单

DirectMuxer.jar		必备，Java GUI
preference.xml		必备，属性文件
assistant.jar		可选，助手工具
readme.txt		可选，说明文件
error.log		可选，错误日志
tasks		可选，任务文件夹



六，依赖关系

Java Runtime Environment (Java SE 7)  Windows x86，下载见http://www.oracle.com/technetwork/java/javase/downloads/index.html
Adobe Photoshop CS5，可选
