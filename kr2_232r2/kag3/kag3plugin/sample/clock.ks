@if exp="typeof(global.clock_object) == 'undefined'"
@iscript

// 時計を表示する ( KAG Plugin サンプル )


class ClockPlugin extends KAGPlugin // 「時計」プラグインクラス
{
	var fore, back; // 表画面のレイヤと裏画面のレイヤ
	var timer; // タイマ
	var window; // ウィンドウへの参照

	function ClockPlugin(window)
	{
		// ClockPlugin コンストラクタ
		super.KAGPlugin(); // スーパークラスのコンストラクタを呼ぶ

		fore = new Layer(window, window.fore.base);
			// 表画面用のレイヤを表背景レイヤを親にして作成
		back = new Layer(window, window.back.base);
			// 裏画面用のレイヤを裏背景レイヤを親にして作成
		fore.absolute = back.absolute = 2000000-2;
			// 重ね合わせ順序はメッセージ履歴よりも奥
			// (「雪」プラグインの雪粒よりも奥)
		fore.hitType = back.hitType = htMask;
		fore.hitThreshold = back.hitThreshold = 256;
			// マウスメッセージは全域透過

		fore.setPos(510, 0);
		back.setPos(510, 0);
			// 初期位置

		fore.setImageSize(130, 36);
		fore.setSizeToImageSize();
		fore.fillRect(0, 0, fore.imageWidth, fore.imageHeight, 0);
			// 表のサイズ調整と塗りつぶし ( 完全透明 )
		back.assignImages(fore);
		back.setSizeToImageSize();
			// 裏の画像を表からコピーして裏のサイズ調整
		fore.visible = back.visible = fore.seen = back.seen = false;
			// 表も裏も表示状態に
		fore.font.face = back.font.face = "ＭＳ ゴシック";
		fore.font.height = back.font.height = 28;
			// フォントとサイズを設定

		timer = new Timer(onTimer, '');
			// タイマオブジェクトを作成
			// (onTimer をイベントハンドラとする)
		timer.interval = 1000;
		timer.enabled = true;
			// 周期と有効/無効を設定

		this.window = window; // window への参照を保存する
	}

	function finalize()
	{
		invalidate fore;
		invalidate back;
			// 表/裏のレイヤを無効化
		invalidate timer;
			// タイマを無効化
		super.finalize(...);
	}

	function setOptions(elm)
	{
		// オプションを設定
		fore.visible = fore.seen = +elm.forevisible if elm.forevisible !== void;
			// 表の可視/不可視
		back.visible = back.seen = +elm.backvisible if elm.backvisible !== void;
			// 裏の可視/不可視
		var l = fore.left;
		var t = fore.top;
		var poschanged = false;
		(l = +elm.left, poschanged = true) if elm.left !== void;
		(t = +elm.top, poschanged = true) if elm.top !== void;
		if(poschanged)
		{
			fore.setPos(l, t);
			back.setPos(l, t);
				// 表示位置の変更
		}
		onTimer(); // 表示を更新
	}

	function onTimer()
	{
		// タイマの周期ごとに呼ばれる
		if(!fore.seen && !back.seen) return;
		var current = new Date();
			// 日付オブジェクトを作成
		var time = "%02d:%02d:%02d".sprintf(
			current.getHours(), current.getMinutes(),
			current.getSeconds());
			// 日付を書式化
		fore.fillRect(0, 0, fore.imageWidth, fore.imageHeight, 0);
			// 下地をクリア
		fore.drawText(5, 5, time, 0xffffff, 255, true, 512, 0, 5, 2, 2);
			// 文字を描画
		back.assignImages(fore);
			// 裏画面に画像をコピー
			// assignImages は内部的には画像を共有するようになるだけで
			// 実行コストの大きいメソッドではない
	}

	function onStore(f, elm)
	{
		// 栞を保存するとき
		var dic = f.clock = %[];
			// f.clock に辞書配列を作成
		dic.foreVisible = fore.seen;
		dic.backVisible = back.seen;
		dic.left = fore.left;
		dic.top = fore.top;
			// 各情報を辞書配列に記録
	}

	function onRestore(f, clear, elm)
	{
		// 栞を読み出すとき
		var dic = f.clock;
		if(dic === void)
		{
			// clock の情報が栞に保存されていない
			fore.visible = fore.seen = false;
			back.visible = back.seen = false;
		}
		else
		{
			// clock の情報が栞に保存されている
			setOptions(%[ forevisible : dic.foreVisible, backvisible : dic.backVisible,
				left : dic.left, top : dic.top]);
				// オプションを設定
		}
	}

	function onStableStateChanged(stable)
	{
		// 「安定」( s l p の各タグで停止中 ) か、
		// 「走行中」 ( それ以外 ) かの状態が変わったときに呼ばれる
	}

	function onMessageHiddenStateChanged(hidden)
	{
		// メッセージレイヤがユーザの操作によって隠されるとき、現れるときに
		// 呼ばれる。メッセージレイヤとともに表示/非表示を切り替えたいときは
		// ここで設定する。
		if(hidden)
		{
			fore.visible = back.visible = false;
		}
		else
		{
			fore.visible = fore.seen;
			back.visible = back.seen;
		}
	}

	function onCopyLayer(toback)
	{
		// レイヤの表←→裏の情報のコピー

		// backlay タグやトランジションの終了時に呼ばれる

		// ここでレイヤに関してコピーすべきなのは
		// 表示/非表示の情報だけ

		if(toback)
		{
			// 表→裏
			back.visible = fore.visible;
			back.seen = fore.seen;
		}
		else
		{
			// 裏→表
			fore.visible = back.visible;
			fore.seen = back.seen;
		}
	}

	function onExchangeForeBack()
	{
		// 裏と表の管理情報を交換

		// children = true のトランジションでは、トランジション終了時に
		// 表画面と裏画面のレイヤ構造がそっくり入れ替わるので、
		// それまで 表画面だと思っていたものが裏画面に、裏画面だと思って
		// いたものが表画面になってしまう。ここのタイミングでその情報を
		// 入れ替えれば、矛盾は生じないで済む。

		// ここで表画面、裏画面のレイヤに関して管理すべきなのは
		// fore と back の変数だけ
		var tmp;
		tmp = back;
		back = fore;
		fore = tmp;
	}

}

kag.addPlugin(global.clock_object = new ClockPlugin(kag));
	// プラグインオブジェクトを作成し、登録する

@endscript
@endif
;
; マクロの登録
@macro name="clockopt"
@eval exp="clock_object.setOptions(mp)"
; mp がマクロに渡された属性を示す辞書配列オブジェクト
@endmacro
@return
