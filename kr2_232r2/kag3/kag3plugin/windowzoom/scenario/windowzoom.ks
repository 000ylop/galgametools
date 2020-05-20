@if exp="typeof(global.windowzoom_object) == 'undefined'"
@iscript

/*
	�u��ʁv���j���[�ɁA�E�B���h�E�g��k���̃��j���[��ǉ�����v���O�C��
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

		// ���j���[�̏�����
		kag.displayMenu.add(new MenuItem(this, "-"));
		var denom = 8; // ����
		var from = 3; // ���q�̍Œ�l
		var to = 18; // ���q�̍ő�l
		for(var numer = from; numer <= to; numer++)
		{
			var caption = "%.0f%%".sprintf(numer/denom*100);
			if(numer - from < 10) caption += " (&%d)".sprintf(numer - from);
			if(numer < denom)
				caption = "�k�� - " + caption;
			else if(numer > denom)
				caption = "�g�� - " + caption;
			var item = new KAGMenuItem(kag, caption, 2, onZoomMenuItemClick, false);
			item.numer = numer;
			item.denom = denom;
			item.stopRecur = true;
			kag.displayMenu.add(item);
			if(numer == denom)
				zoom100item = item;
		}
		zoom100item.checked = true;

		// KAG ���֐��̒u������
		onWindowedMenuItemClick_org = kag.onWindowedMenuItemClick;
		onFullScreenMenuItemClick_org = kag.onFullScreenMenuItemClick;
		kag.onWindowedMenuItemClick = kag.windowedMenuItem.command =
			onWindowedMenuItemClick_new incontextof kag;
		kag.onFullScreenMenuItemClick = kag.fullScreenMenuItem.command =
			onFullScreenMenuItemClick_new incontextof kag;


		// ���Ƀt���X�N���[���ɂȂ��Ă����Ƃ��͔{���֘A�̃��j���[���g�p�s�\��
		if(kag.fullScreen)
			enableZoomMenuItems(false); // ���j���[���ڂ��g�p�s�\�ɂ���
	}

	function finalize()
	{
		super.finalize(...);
	}

	function trySetZoom(numer, denom)
	{
		// numer/denom �ɔ{����ݒ肷��
		// �ݒ�ɐ��������ꍇ�͐^�A���s�����ꍇ�͋U��Ԃ�
		var orgw = kag.innerWidth;
		var orgh = kag.innerHeight;
		var w = (int)(kag.scWidth * numer / denom);
		var h = (int)(kag.scHeight * numer / denom);
		Debug.message(@"�E�B���h�E�T�C�Y�� &w;x&h; �ɐݒ�...");
		kag.setInnerSize(w, h);
		if(kag.innerWidth != w || kag.innerHeight != h)
		{
			// �T�C�Y�̐ݒ�Ɏ��s����;
			// Windows�̓E�B���h�E�T�C�Y����ʂ̃T�C�Y�𒴂��邱��
			// ���ł��Ȃ��̂�
			// �ݒ�Ɏ��s����\��������
			// �T�C�Y�����ɖ߂�
			Debug.message("�E�B���h�E�T�C�Y�̐ݒ�Ɏ��s");
			kag.setInnerSize(orgw, orgh);
			return false;
		}

		kag.setZoom(numer, denom);
		return true;
	}

	function onZoomMenuItemClick(item)
	{
		// ���j���[�A�C�e�����N���b�N���ꂽ�ꍇ
		if(trySetZoom(item.numer, item.denom))
			item.checked = true;
	}

	function setZoomFromMenu()
	{
		// ���݃��j���[���ڂ��`�F�b�N����Ă���{����
		// �ݒ肷��
		var items = kag.displayMenu.children;
		for(var i = 0; i < items.count; i++)
		{
			if(typeof items[i].numer != "undefined" &&
				items[i].checked)
			{
				var item = items[i];
				if(!trySetZoom(item.numer, item.denom))
				{
					// �{���̐ݒ�Ɏ��s
					// 100% �ɐݒ莎�s
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
		// �E�B���h�E���[�h�Ɉڍs����ۂɌĂ΂��
		if(!kag.fullScreened) return;
		with(global.windowzoom_object)
		{
			.onWindowedMenuItemClick_org(...); // ���̃��\�b�h���Ăяo��
			.enableZoomMenuItems(true); // ���j���[���ڂ��g�p�\�ɂ���
			.setZoomFromMenu(); // ���Ƃ̔{���𕜊�������
		}
	}

	function onFullScreenMenuItemClick_new()
	{
		// �t���X�N���[���Ɉڍs����ۂɌĂ΂��
		if(kag.fullScreened) return;
		with(global.windowzoom_object)
		{
			.trySetZoom(1, 1); // 100% �{���ɐݒ��߂�
			.enableZoomMenuItems(false); // ���j���[���ڂ��g�p�s�\�ɂ���
			.onFullScreenMenuItemClick_org(...); // ���̃��\�b�h���Ăяo��
		}
	}

	function enableZoomMenuItems(b)
	{
		// �{���֘A�̃��j���[���ڂ̗L��/������؂�ւ���
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
		// �x��ۑ�����Ƃ�
	}

	function onRestore(f, clear, elm)
	{
		// �x��ǂݏo���Ƃ�
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
	// �v���O�C���I�u�W�F�N�g���쐬���A�o�^����

@endscript
@return
