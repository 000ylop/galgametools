@if exp="typeof(global.spectrum_object) == 'undefined'"
@iscript

Plugins.link("fftgraph.dll"); // fftgraph.dll を読み込む

// スペアナを表示


class SpectrumPlugin extends KAGPlugin // 「スペアナ」プラグインクラス
{
	var timer = void; // タイマ
	var left = 0;
	var top = 0;
	var width = 64;
	var height = 32;
	var options = %[];

	function SpectrumPlugin()
	{
		// SpectrumPlugin コンストラクタ
		super.KAGPlugin(); // スーパークラスのコンストラクタを呼ぶ
	}

	function finalize()
	{
		stop();
		invalidate timer if timer !== void;
		super.finalize(...);
	}

	function setOptions(elm)
	{
		// オプションを設定
		left = +elm.left if elm.left !== void;
		top = +elm.top if elm.left !== void;
		width = +elm.width if elm.width !== void;
		height = +elm.height if elm.height !== void;
		(Dictionary.assign incontextof options)(elm);
	}

	function start()
	{
		// 表示を開始
		if(timer === void)
		{
			kag.bgm.buf1.useVisBuffer = true if kag.bgm.buf1 !== void;
			kag.bgm.buf2.useVisBuffer = true if kag.bgm.buf2 !== void;

			timer = new Timer(onTimer, '');
			timer.interval = 43;
			timer.enabled = true;
		}
	}

	function stop()
	{
		// 表示を停止
		if(timer !== void)
		{
			kag.bgm.buf1.useVisBuffer = true if kag.bgm.buf1 !== void;
			kag.bgm.buf2.useVisBuffer = true if kag.bgm.buf2 !== void;
			invalidate timer;
			timer = void;
		}
	}

	function onTimer()
	{
		var buf; // 対象の BGM バッファ
		if(kag.bgm.buf2 !== void && kag.bgm.buf2.status == "playing")
			buf = kag.bgm.buf2;
		else
			buf = kag.bgm.buf1;

		drawFFTGraph(kag.fore.base, buf, left, top, width, height, options);
	}

	function onRestore(f, clear, elm)
	{
		// 栞を読み出すとき
		stop(); // 動作を停止
	}
}

kag.addPlugin(global.spectrum_object = new SpectrumPlugin(kag));
	// プラグインオブジェクトを作成し、登録する

@endscript
@endif
;
; マクロの登録
@macro name="spectrumopt"
@eval exp="spectrum_object.setOptions(mp)"
; mp がマクロに渡された属性を示す辞書配列オブジェクト
@endmacro
@macro name="spectrumstart"
@eval exp="spectrum_object.start()"
@endmacro
@macro name="spectrumstop"
@eval exp="spectrum_object.stop()"
@endmacro
@return
