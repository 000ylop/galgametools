@if exp="typeof(global.zoom_object) == 'undefined'"
@iscript

/*
	背景/前景画像の拡大縮小効果による表示を行うプラグイン
*/

class ZoomPlugin extends KAGPlugin
{
	var tempLayer; // テンポラリレイヤ
	var overlayLayer; // オーバーレイレイヤ

	var sl, st, sw, sh;
	var dl, dt, dw, dh;
	var startTick; // 開始ティック
	var time; // ズームを行っている時間
	var mode; // 前景レイヤモード
	var accel; // 加速度的な動きを行うか ( 負 : 0 : 正 )
	var storage;
	var moving = false;
	var nextstop;
	var moveFunc; // 移動位置計算用関数
	var targetLayerName; // 対象レイヤ名
	var targetLayer; // 対象レイヤ

	function ZoomPlugin(window)
	{
		super.KAGPlugin();
		this.window = window;
	}

	function finalize()
	{
		// finalize メソッド
		// このクラスの管理するすべてのオブジェクトを明示的に破棄
		stop();

		invalidate tempLayer if tempLayer !== void;
		invalidate overlayLayer if overlayLayer !== void;

		super.finalize(...);
	}

	function startZoom(storage, layer, mode, basestorage, sl, st, sw, sh, dl, dt, dw, dh, time, accel)
	{
		// storage : 表示画像
		// layer : 対象レイヤ
		// mode : 前景レイヤの透過モード
		// bgimage : 背景画像
		// sl st sw sh : 初期位置
		// dl dt dw dh : 最終位置
		// time : ズームを行っている時間
		// accel : 加速をつけるかどうか

		// 既存の動作を停止
		stop();

		// 対象レイヤを決定
		if(layer == '' || layer == 'base')
			targetLayer = window.fore.base;
		else
			targetLayer = window.fore.layers[+layer];

		// 背景画像の読み込み
		if(basestorage !== void)
			window.tagHandlers.image(%[ storage : basestorage, layer : layer, page : 'fore']);

		// オブジェクトにパラメータをコピー
		this.sl = sl; this.st = st; this.sw = sw; this.sh = sh;
		this.dl = dl; this.dt = dt; this.dw = dw; this.dh = dh;
		this.time = time;
		this.accel = accel;
		this.storage = storage;
		this.targetLayerName = layer;
		this.mode = mode;

		// tempLayer 確保
		var base = window.fore.base;
		if(tempLayer === void)
		{
			tempLayer = new Layer(window, base);
			tempLayer.loadImages(storage);
		}

		// overlayLayer 確保
		if(overlayLayer === void)
		{
			overlayLayer = new Layer(window, base);
			overlayLayer.absolute = targetLayer.absolute + 1; // 対象レイヤのすぐ手前
			overlayLayer.hitType = htMask;
			overlayLayer.hitThreshold = 256; // マウスメッセージは全域透過
			overlayLayer.face = dfBoth;
			overlayLayer.type = layer == 'base' ? ltCoverRect : (mode == 'rect' ? ltCoverRect : ltTransparent);
			// overlayLayer は 初期サイズあるいは最終サイズのどちらか大きい方
			// のサイズになるが、画面サイズよりは大きくならない
			var mw = sw > dw ? sw : dw;
			var mh = sh > dh ? sh : dh;
			overlayLayer.setImageSize(
				mw < base.imageWidth ? mw : base.imageWidth,
				mh < base.imageHeight ? mh : base.imageHeight);
		}

		// 移動位置計算関数の設定
		moveFunc = defaultMover;

		// 初期位置に表示
		moveFunc(moveAt, 0);
		overlayLayer.visible = true;
		if(layer != 'base') targetLayer.visible = false;

		// 開始
		startTick = System.getTickCount();
		System.addContinuousHandler(continuousHandler);
		moving = true;
		nextstop = false;
	}

	function moveAt(l, t, w, h)
	{
		// l t w h 位置に移動

		// レイヤ移動
		var base = window.fore.base;
		var oll = l < 0 ? 0 : l;
		var olt = t < 0 ? 0 : t;
		var olw = l + w > base.imageWidth ? base.imageWidth - oll : l + w - oll;
		var olh = t + h > base.imageHeight ? base.imageHeight - olt : t + h - olt;
		if(olw > 0 && olh > 0)
		{
			overlayLayer.visible = true;
			overlayLayer.setPos(oll, olt, olw, olh);

			// 拡大縮小転送
			overlayLayer.stretchCopy(l - oll, t - olt, w, h,
				tempLayer, 0, 0, tempLayer.imageWidth, tempLayer.imageHeight);
				// 移動先が右や下にはみ出る場合にちょっと無駄な転送が起こるかも
		}
		else
		{
			overlayLayer.visible = false;
		}
	}

	/*static*/ function defaultMover(func, tm)
	{
		// 位置計算
		// tm は 0.0(開始点) ～ 1.0(終了点) の間で変化する変数なので、
		// これを元にして位置を計算する
		var l = (int)((dl - sl) * tm + sl);
		var t = (int)((dt - st) * tm + st);
		var w = (int)((dw - sw) * tm + sw);
		var h = (int)((dh - sh) * tm + sh);

		// 移動
		func(l, t, w, h);
	}

	function continuousHandler(tick)
	{
		// ハンドラ
		if(nextstop)
		{
			// 終了
			finish();
			return;
		}

		// 時間を得る
		var tm = tick - startTick;
		tm /= time;
		if(tm >= 1)
		{
			nextstop = true;
			tm = 1;
		}
		else
		{
			// 加速計算
			if(accel < 0)
			{
				// 上弦 ( 最初が動きが早く、徐々に遅くなる )
				tm = 1.0 - tm;
				tm = Math.pow(tm, -accel);
				tm = 1.0 - tm;
			}
			else if(accel > 0)
			{
				// 下弦 ( 最初は動きが遅く、徐々に早くなる )
				tm = Math.pow(tm, accel);
			}
		}

		// 移動
		moveFunc(moveAt, tm);
	}

	function finish()
	{
		// ズームの終了
		if(targetLayerName == 'base')
		{
			// 背景レイヤの場合
			var base = window.fore.base;
			if(dl == 0 && dt == 0 && dw == base.imageWidth && dh == base.imageHeight &&
				tempLayer.imageWidth == base.imageWidth && tempLayer.imageHeight == base.imageHeight)
			{
				// 最終位置が画面全体を覆っていて、かつ、
				// ズームさせた画像のサイズが背景画像と同じ場合
				// 画像を読み込む
				window.tagHandlers.image(%[ storage : storage, layer : 'base', page : 'fore']);
			}
			else
			{
				// 最終位置が画面全体を覆っていない;
				// この場合は、最終位置のサイズが 0 でない限り
				// この状態からの栞の再開は不可能 ( 背景画像を再構成できない )
				// ( 背景に画像を読み込んだあとならば OK )
				window.fore.base.face = dfBoth;
				window.fore.base.stretchCopy(dl, dt, dw, dh, tempLayer, 0, 0,
					tempLayer.imageWidth, tempLayer.imageHeight);
			}
		}
		else
		{
			// 前景レイヤの場合
			if(dw == tempLayer.imageWidth && dh == tempLayer.imageHeight)
			{
				// 最終位置のサイズが前景レイヤと同じ場合
				// 画像を読み込む
				window.tagHandlers.image(%[ storage : storage, layer : targetLayerName, page : 'fore',
					left : dl, top : dt, visible : true, mode : mode]);
			}
			else
			{
				// そうでない場合
				// この場合は、最終位置のサイズが 0 で無い限り、
				// この状態から栞の再開は不可能
				// (対象のレイヤに画像をもう一度読み直すならば可)
				if(dw && dh)
				{
					targetLayer.setImageSize(dw < 0 ? -dw : dw, dh < 0 ? -dh : dh);
					targetLayer.setSizeToImageSize();
					targetLayer.setPos(dl, dt);
					targetLayer.face = dfBoth;
					targetLayer.stretchCopy(dw < 0 ? dw : 0, dh < 0 ? dh : 0, dw, dh, tempLayer, 0, 0,
						tempLayer.imageWidth, tempLayer.imageHeight);
					targetLayer.visible = true;
					targetLayer.type = ltTransparent;
				}
				else
				{
					targetLayer.visible = false;
				}
			}
		}
		stop(); // 停止
	}

	function stop()
	{
		// 停止
		if(moving)
		{
			window.trigger('zoom');
			System.removeContinuousHandler(continuousHandler);
			moving = false;
		}
		invalidate tempLayer if tempLayer !== void;
		tempLayer = void;
		invalidate overlayLayer if overlayLayer !== void;
		overlayLayer = void;
		targetLayer = void;
	}

	function onStore(f, elm)
	{
		// 栞を保存するとき
	}

	function onRestore(f, clear, elm)
	{
		// 栞を読み出すとき
		stop(); // 動作を停止
	}

	function onStableStateChanged(stable)
	{
	}

	function onMessageHiddenStateChanged(hidden)
	{
	}

	function onCopyLayer(toback)
	{
	}

	function onExchangeForeBack()
	{
	}
}

kag.addPlugin(global.zoom_object = new ZoomPlugin(kag));
	// プラグインオブジェクトを作成し、登録する

@endscript
@endif
; マクロ登録
@macro name="bgzoom"
@eval exp="zoom_object.startZoom(mp.storage, 'base', 'rect', mp.basestorage, +mp.sl, +mp.st, +mp.sw, +mp.sh, +mp.dl, +mp.dt, +mp.dw, +mp.dh, +mp.time, +mp.accel)"
@endmacro
@macro name="wbgzoom"
@if exp="zoom_object.moving"
@waittrig * name="zoom" onskip="zoom_object.finish()"
@endif
@endmacro
@macro name="fgzoom"
@eval exp="zoom_object.startZoom(mp.storage, mp.layer, mp.mode, void, +mp.sl, +mp.st, +mp.sw, +mp.sh, +mp.dl, +mp.dt, +mp.dw, +mp.dh, +mp.time, +mp.accel)"
@endmacro
@macro name="wfgzoom"
@if exp="zoom_object.moving"
@waittrig * name="zoom" onskip="zoom_object.finish()"
@endif
@endmacro
@return
