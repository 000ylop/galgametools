@if exp="typeof(global.windowzoom_object) == 'undefined'"
@iscript

/*
	「画面」メニューに、ウィンドウ拡大縮小のメニューを追加するプラグイン
*/


class WindowZoomPlugin extends KAGPlugin
{
	var onWindowedMenuItemClick_org;
	var onFullScreenMenuItemClick_org;

	var zoom100item;

	function WindowZoomPlugin(window)
	{
		super.KAGPlugin();
		this.window = window;

		// メニューの初期化
		kag.displayMenu.add(new MenuItem(this, "-"));
		var denom = 8; // 分母
		var from = 3; // 分子の最低値
		var to = 18; // 分子の最大値
		for(var numer = from; numer <= to; numer++)
		{
			var caption = "%.0f%%".sprintf(numer/denom*100);
			if(numer - from < 10) caption += " (&%d)".sprintf(numer - from);
			if(numer < denom)
				caption = "縮小 - " + caption;
			else if(numer > denom)
				caption = "拡大 - " + caption;
			var item = new KAGMenuItem(kag, caption, 2, onZoomMenuItemClick, false);
			item.numer = numer;
			item.denom = denom;
			item.stopRecur = true;
			kag.displayMenu.add(item);
			if(numer == denom)
				zoom100item = item;
		}
		zoom100item.checked = true;

		// KAG 内関数の置き換え
		onWindowedMenuItemClick_org = kag.onWindowedMenuItemClick;
		onFullScreenMenuItemClick_org = kag.onFullScreenMenuItemClick;
		kag.onWindowedMenuItemClick = kag.windowedMenuItem.command =
			onWindowedMenuItemClick_new incontextof kag;
		kag.onFullScreenMenuItemClick = kag.fullScreenMenuItem.command =
			onFullScreenMenuItemClick_new incontextof kag;


		// 既にフルスクリーンになっていたときは倍率関連のメニューを使用不可能に
		if(kag.fullScreen)
			enableZoomMenuItems(false); // メニュー項目を使用不可能にする
	}

	function finalize()
	{
		super.finalize(...);
	}

	function trySetZoom(numer, denom)
	{
		// numer/denom に倍率を設定する
		// 設定に成功した場合は真、失敗した場合は偽を返す
		var orgw = kag.innerWidth;
		var orgh = kag.innerHeight;
		var w = (int)(kag.scWidth * numer / denom);
		var h = (int)(kag.scHeight * numer / denom);
		Debug.message(@"ウィンドウサイズを &w;x&h; に設定...");
		kag.setInnerSize(w, h);
		if(kag.innerWidth != w || kag.innerHeight != h)
		{
			// サイズの設定に失敗した;
			// Windowsはウィンドウサイズが画面のサイズを超えること
			// ができないので
			// 設定に失敗する可能性がある
			// サイズを元に戻す
			Debug.message("ウィンドウサイズの設定に失敗");
			kag.setInnerSize(orgw, orgh);
			return false;
		}

		kag.setZoom(numer, denom);
		return true;
	}

	function onZoomMenuItemClick(item)
	{
		// メニューアイテムがクリックされた場合
		if(trySetZoom(item.numer, item.denom))
			item.checked = true;
	}

	function setZoomFromMenu()
	{
		// 現在メニュー項目がチェックされている倍率に
		// 設定する
		var items = kag.displayMenu.children;
		for(var i = 0; i < items.count; i++)
		{
			if(typeof items[i].numer != "undefined" &&
				items[i].checked)
			{
				var item = items[i];
				if(!trySetZoom(item.numer, item.denom))
				{
					// 倍率の設定に失敗
					// 100% に設定試行
					item = zoom100item;
					if(trySetZoom(item.numer, item.denom))
						item.checked = true;
				}
				break;
			}
		}
	}

	function onWindowedMenuItemClick_new()
	{
		// ウィンドウモードに移行する際に呼ばれる
		if(!kag.fullScreened) return;
		with(global.windowzoom_object)
		{
			.onWindowedMenuItemClick_org(...); // 元のメソッドを呼び出す
			.enableZoomMenuItems(true); // メニュー項目を使用可能にする
			.setZoomFromMenu(); // もとの倍率を復活させる
		}
	}

	function onFullScreenMenuItemClick_new()
	{
		// フルスクリーンに移行する際に呼ばれる
		if(kag.fullScreened) return;
		with(global.windowzoom_object)
		{
			.trySetZoom(1, 1); // 100% 倍率に設定を戻す
			.enableZoomMenuItems(false); // メニュー項目を使用不可能にする
			.onFullScreenMenuItemClick_org(...); // 元のメソッドを呼び出す
		}
	}

	function enableZoomMenuItems(b)
	{
		// 倍率関連のメニュー項目の有効/無効を切り替える
		var items = kag.displayMenu.children;
		for(var i = 0; i < items.count; i++)
		{
			if(typeof items[i].numer != "undefined")
			{
				items[i].enabled = b;
			}
		}
	}

	function onStore(f, elm)
	{
		// 栞を保存するとき
	}

	function onRestore(f, clear, elm)
	{
		// 栞を読み出すとき
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

kag.addPlugin(global.windowzoom_object = new WindowZoomPlugin(kag));
	// プラグインオブジェクトを作成し、登録する

@endscript
@return
