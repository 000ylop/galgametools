@if exp="typeof(global.gvolume_object) == 'undefined'"
@iscript

// 音量設定メニュープラグイン

class GVolumePlugin extends KAGPlugin
{
	var window; // ウィンドウへの参照
	var volumeMenu; // 「音量(&O)」メニュー

	var menuInsertPoint = 3; // メニュー挿入位置
	var insertToRoot = false;
		// ↑ true に設定すると gvolmenu タグで挿入する音量のメニューが、
		// メニューバーに直接並びます。挿入位置は menuInsertPoint で
		// 指定できます。

	function GVolumePlugin(window)
	{
		super.KAGPlugin();
		this.window = window;

		if(!insertToRoot)
		{
			// 「音量」メニューアイテムの作成
			window.menu.insert(volumeMenu = new KAGMenuItem(window, "音量(&O)", 0, void, false),
				menuInsertPoint);
				// ここで、window.menu はメニューの挿入先、最後の 3 はメニューの挿入位置
			volumeMenu.stopRecur = true; // このメニューの先の再帰検索を行わせない
		}

	}

	function finalize()
	{
		super.finalize(...);
	}

	function createMenu(name, control)
	{
		// 音量調節メニューを作成する
		// name = メニュー名
		// control = コントロール先 ('bgm' または 効果音番号 0 ～ )
		var menu = new KAGMenuItem(window, name, 0, void, false);
		var currentvol; // 現在のボリューム
		if(control == 'bgm')
		{
			// BGM
			currentvol = int(window.bgm.buf1.volume2 / 1000);
			menu.control = control;
		}
		else
		{
			// 効果音は ',' で区切って複数指定できるが
			// ここで音量を得るのは最初のバッファのみ
			var buffers = [].split(',', control);
			currentvol = int(window.se[+(buffers[0])].volume2 / 1000);
			menu.control = buffers;
			dm(currentvol);
		}

		if(!insertToRoot)
			volumeMenu.add(menu);
		else
			window.menu.insert(menu, menuInsertPoint);

		menu.stopRecur = true; // このメニューの先の再帰検索を行わせない
		var checked = false;
		for(var i = 100; i >= 0; i-=5)
		{
			var name = i ? (i + "%" ) : "ミュート(&M)";
			var submenu = new KAGMenuItem(window, name, 1, onMenuClick, false);
			menu.add(submenu);
			submenu.control = control;
			submenu.volume = i;
			if(!checked && i <= currentvol) submenu.checked = true, checked = true;
		}
	}

	function onMenuClick(menu)
	{
		// メニューが選択された
		if(menu.control === 'bgm')
		{
			// bgm の音量を設定
			window.bgm.setOptions(%[gvolume:menu.volume]);
		}
		else
		{
			// 効果音の音量を設定
			var buffers = menu.parent.control;
			dm(buffers);
			for(var i = buffers.count - 1; i >= 0; i--)
			{
				window.se[+(buffers[i])].setOptions(%[gvolume:menu.volume]);
			}
		}
		menu.checked = true;
	}
}

kag.addPlugin(global.gvolume_object = new GVolumePlugin(kag));
	// プラグインオブジェクトを作成し、登録する

@endscript
@endif
@macro name="gvolmenu"
@eval exp="gvolume_object.createMenu(mp.name, mp.control)"
@endmacro
@return
