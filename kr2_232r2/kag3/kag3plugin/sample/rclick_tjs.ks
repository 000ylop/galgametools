@if exp="typeof(global.rclickconfig_object) == 'undefined'"
@iscript
// �E�N���b�N�ł̐ݒ��ʂ� TJS2 �Ŏ�������T���v��

// �x�� 28 ���p�\�Ȃ̂ŁAConfig.tjs �̐ݒ�� 28 �ɂ��Ă�������

// 2002/4/10 3.05 beta 5 �t��  ����
// 2002/4/23                   ���C�����j���[����u�x�����ށv��I�����Ă�
//                             �X�i�b�v�V���b�g���X�V�ł���悤�ɂ���
//                             ( RClickConfigLayer.saveSnapshot �ǉ�
//                               RClickConfigLayer.onLoadOrSave �ύX
//                               �Ō�̕��́ukag �� saveBookMark ��u��������v
//                               ��ǉ� )
// 2002/6/14                   KAG3 �{�̞̂x�֘A�̎d�l�ύX�ɔ������낢��ύX��
//                             �܂��� ( diff �Ƃ��Ă������� )
//                             Config.tjs �ɂ� freeSaveDataMode �� false �ɁA
//                             saveThumbnail �� true �ɐݒ肷��K�v������܂�


class RButtonLayer extends ButtonLayer
	// parent �� onClick �C�x���g�𑗂�悤�ɂ����{�^�����C��
{
	var tag;

	function RButtonLayer(window, parent)
	{
		super.ButtonLayer(window, parent);
		focusable = false;
	}

	function finalize()
	{
		super.finalize(...);
	}

	function onClick()
	{
		super.onClick(...);
		if(enabled)
			parent.onButtonClick(this);
	}
}

class SaveDataItemLayer extends KAGLayer
{
	// �x���ɑΉ����郌�C��
	var num; // �x�ԍ�
	var bgColor = 0xa0ffffff; // �w�i�F ( 0xAARRGGBB )
	var focusedColor = 0xffffffff;

	var commentEdit; // �R�����g�̃G�f�B�b�g
	var protectCheckBox; // �u�f�[�^�ی�v�`�F�b�N�{�b�N�X

	function SaveDataItemLayer(window, parent, num)
	{
		super.KAGLayer(window, parent);

		this.num = num;

		setImageSize(500, 112); // �T�C�Y
		face = dfBoth;
		fillRect(0, 0, imageWidth, imageHeight, bgColor);
		setSizeToImageSize();

		hitType = htMask;
		hitThreshold = 0; // �S��s����

		cursor = kag.cursorPointed;

		focusable = true; // �t�H�[�J�X�͎󂯎���

		protectCheckBox = new CheckBoxLayer(window, this);
		protectCheckBox.width = 16;
		protectCheckBox.height = 16;
		protectCheckBox.color = 0xffffff;
		protectCheckBox.opacity = 64;
		protectCheckBox.textColor = 0x000000;
		protectCheckBox.setPos(398, 34);
		protectCheckBox.visible = true;

		protectCheckBox.checked = kag.bookMarkProtectedStates[num];


		commentEdit = new EditLayer(window, this);

		commentEdit.setPos(180, 79);
		commentEdit.width = 310;
		commentEdit.height = 18;
		commentEdit.color = 0xffffff;
		commentEdit.opacity = 64;
		commentEdit.textColor = 0x000000;
		commentEdit.visible = true;

		if(kag.scflags.bookMarkComments !== void)
			commentEdit.text = kag.scflags.bookMarkComments[num];


		font.height = 14;
		face = dfBoth;

		drawText(420, 35, "�f�[�^�ی�", 0x000000);

		// �ԍ���\��
		var str = string (num + 1);
		font.height = 20;
		var ty = font.getTextHeight(str);
		drawText(6, ( imageHeight - ty ) \ 2, str, 0);

		// �T���l�C���摜��ǂݍ���
		var tmplayer = new global.Layer(window, parent);

 		var tnname = kag.getBookMarkFileNameAtNum(num);

		if(Storages.isExistentStorage(tnname))
		{
			tmplayer.loadImages(tnname);
		}
		else
		{
			tmplayer.setImageSize(133, 100);
			var str = "�f�[�^�Ȃ�";
			var tx = tmplayer.font.getTextWidth(str);
			tmplayer.drawText((tmplayer.imageWidth - tx) \ 2, 40,
				str, 0xffffff);
		}

		copyRect(32, 6, tmplayer, 0, 0, tmplayer.imageWidth, tmplayer.imageHeight);

		invalidate tmplayer;

		// �x�̕ۑ�����\��
		font.height = 14;

		var str = kag.bookMarkNames[num];
		if(str == '') str = '�f�[�^�Ȃ�';
		drawText(180, 15, str, 0x000000);

		// ���t��\��
		if(kag.bookMarkDates[num] == '')
			str = "���t : ----/--/-- --:--", commentEdit.enabled = false;
		else
			str = "���t : " + kag.bookMarkDates[num];

		drawText(180, 35, str, 0x000000);

		// �R�����g : 
		drawText(180, 63, "Comments :", 0x000000);
	}

	function finalize()
	{
		invalidate commentEdit;
		invalidate protectCheckBox;
		super.finalize(...);
	}

	function saveToSystemVariable()
	{
		// ��Ԃ��V�X�e���ϐ��ɋL�^����
		if(kag.scflags.bookMarkComments === void)
			kag.scflags.bookMarkComments = [];
		kag.scflags.bookMarkComments[num] = commentEdit.text;
		kag.bookMarkProtectedStates[num] = protectCheckBox.checked;
	}

	function onPaint()
	{
		super.onPaint(...);

		// update() ���Ă΂ꂽ��A�`��̒��O�ɌĂ΂��
		face = dfBoth;
		if(focused)
		{
			fillRect(0, 0, imageWidth, 2, focusedColor);
			fillRect(0, 2, 2, imageHeight - 2, focusedColor);
			fillRect(imageWidth - 2, 2, 2, imageHeight - 2, focusedColor);
			fillRect(2, imageHeight - 2, imageWidth - 4, 2, focusedColor);
		}
		else
		{
			fillRect(0, 0, imageWidth, 2, bgColor);
			fillRect(0, 2, 2, imageHeight - 2, bgColor);
			fillRect(imageWidth - 2, 2, 2, imageHeight - 2, bgColor);
			fillRect(2, imageHeight - 2, imageWidth - 4, 2, bgColor);
		}
	}

	function onFocus()
	{
		// �t�H�[�J�X�𓾂�
		super.onFocus(...);
		update();
	}

	function onBlur()
	{
		// �t�H�[�J�X��������
		super.onBlur(...);
		update();
	}

	function onHitTest(x, y, process)
	{
		if(process)
		{
			// �E�{�^����������Ă����Ƃ��ɃC�x���g�𓧉�
			if(System.getKeyState(VK_RBUTTON))
				super.onHitTest(x, y, false);
			else
				super.onHitTest(x, y, true);
		}
	}

	function onKeyDown(key, shift, process)
	{
		// �L�[�������ꂽ
		if(process && key == VK_RETURN || key == VK_SPACE)
		{
			// �X�y�[�X�L�[�܂��̓G���^�[�L�[
			super.onKeyDown(key, shift, false);
			saveToSystemVariable();
			parent.onLoadOrSave(num);
		}
		else
		{
			// process �� false �̏ꍇ�͏����͍s��Ȃ�
			super.onKeyDown(...);
		}
	}

	function onMouseDown(x, y, button, shift)
	{
		super.onMouseDown(...);
		if(button == mbLeft)
		{
			focus();
			saveToSystemVariable();
			parent.onLoadOrSave(num);
		}
	}
}

class RClickConfigLayer extends Layer // �ݒ��ʃ��C��
{
	var closeButton; // �u����v�{�^��
	var saveButton; // �Z�[�u �{�^��
	var loadButton; // ���[�h �{�^��
	var hideMessageButton; // ���b�Z�[�W������ �{�^��
	var saveDataItems;
	var state = -1; // 0 = ���C�����j���[  1 = ���[�h��� 2 = �Z�[�u���
	var owner; // RClickConfigPlugin �I�u�W�F�N�g�ւ̎Q��
	var currentPage = 0; // �Z�[�u�f�[�^�I�𒆂ɕ\�����̃y�[�W
	var returnButton; // ���ǂ� �{�^��
	var pageButtons; // �Z�[�u�f�[�^�̃y�[�W�{�^��

	function RClickConfigLayer(win, par, owner)
	{
		super.Layer(win, par);
		this.owner = owner;

		// ���C���̏�Ԃ�������
		setImageSize(640, 480);
		clearBase();
		setSizeToImageSize();
		setPos(0, 0);
		hitType = htMask;
		hitThreshold = 0; // �S��s����
	}

	function finalize()
	{
		clear();

		invalidate closeButton if closeButton !== void;
		invalidate saveButton if saveButton !== void;
		invalidate loadButton if loadButton !== void;
		invalidate hideMessageButton if hideMessageButton !== void;

		invalidate returnButton if returnButton !== void;

		super.finalize(...);
	}

	function clearBase()
	{
		// ���n��h�蒼��
		fillRect(0, 0, imageWidth, imageHeight, 0xc0000000);
	}


	function makeMainMenu()
	{
		// ���C�����j���[�̕\��
		if(state != 0)
		{
			clear();
			state = 0;

			/*
				�{�^�����쐬���Ă��܂�
				�����ł͕����{�^�����쐬���Ă܂����A
				�摜�{�^������肽���Ƃ���
			if(closeButton === void)
			{
				closeButton = new RButtonLayer(window, this);
				closeButton.showFocusImage = true;
				closeButton.loadImages("�{�^���摜�t�@�C����");
				closeButton.left = 270;
				closeButton.top = 350;
				closeButton.focusable = true;
			}
				�݂����Ȋ����ŋL�q���܂�

				�����Ŏw�肷��{�^���͕��ʂ̃{�^���摜
				( �R�̏�Ԃ����ɕ���ł���摜 ) �ł͂Ȃ��āA
				�S�̏�� ( �ʏ�A�����ꂽ���A�}�E�X����ɂ���Ƃ��A
				�t�H�[�J�X�����鎞 ) �����ɕ��񂾉摜�ł���K�v��
				����܂��B�Ƃ��ɍŌ�́u�t�H�[�J�X������Ƃ��v��
				��Ԃ́A�L�[�{�[�h�ő��������l�̂��߂ɕK�v�ł�
				( showFocusImage=true �ɂ���ƂS�����ɂȂ�ׂ�
				  �摜���g�p����悤�ɂȂ�܂�;���� ) 
			*/

			// �u����v�{�^�����쐬
			if(closeButton === void)
			{
				closeButton = new RButtonLayer(window, this);
				closeButton.width = 100;
				closeButton.height = 25;
				closeButton.caption = "����";
				closeButton.captionColor = 0xffffff;
				closeButton.left = 270;
				closeButton.top = 350;
				closeButton.focusable = true;
			}
			closeButton.visible = true;

			// �u�Z�[�u�v�{�^�����쐬
			if(saveButton === void)
			{
				saveButton = new RButtonLayer(window, this);
				saveButton.width = 100;
				saveButton.height = 25;
				saveButton.caption = "�Z�[�u";
				saveButton.captionColor = 0xffffff;
				saveButton.left = 270;
				saveButton.top = 100;
				saveButton.focusable = true;
			}
			saveButton.enabled = kag.canStore();
			saveButton.visible = true;

			// �u���[�h�v�{�^�����쐬
			if(loadButton === void)
			{
				loadButton = new RButtonLayer(window, this);
				loadButton.width = 100;
				loadButton.height = 25;
				loadButton.caption = "���[�h";
				loadButton.captionColor = 0xffffff;
				loadButton.left = 270;
				loadButton.top = 150;
				loadButton.focusable = true;
			}
			loadButton.enabled = kag.canRestore();
			loadButton.visible = true;

			// �u���b�Z�[�W�������v�{�^�����쐬
			if(hideMessageButton === void)
			{
				hideMessageButton = new RButtonLayer(window, this);
				hideMessageButton.width = 100;
				hideMessageButton.height = 25;
				hideMessageButton.caption = "���b�Z�[�W������";
				hideMessageButton.captionColor = 0xffffff;
				hideMessageButton.left = 270;
				hideMessageButton.top = 200;
				hideMessageButton.focusable = true;
			}
			hideMessageButton.visible = true;
		}
	}

	function clearSaveDataItems()
	{
		// �Z�[�u�f�[�^�\���̃N���A
		if(saveDataItems !== void)
		{
			for(var i = 0; i < saveDataItems.count; i++)
			{
				saveDataItems[i].saveToSystemVariable();
				invalidate saveDataItems[i];
			}
			saveDataItems = void;
			kag.setBookMarkMenuCaptions();
		}
	}

	function makeSaveDataItems()
	{
		// �Z�[�u�f�[�^�̕\��
		clearSaveDataItems();
		saveDataItems = [];
		for(var i = 0; i < 4; i++)
		{
			var obj = new SaveDataItemLayer(window, this, currentPage * 4 + i);
			saveDataItems[i] = obj;
			obj.setPos(140 - 4, i * 120 + 4);
			obj.visible = true;
		}
	}

	function clearPageButtons()
	{
		// �y�[�W�{�^���̃N���A
		if(pageButtons !== void)
		{
			for(var i = 0; i < pageButtons.count; i++)
			{
				invalidate pageButtons[i];
			}
			pageButtons = void;
		}
	}

	function makePageButtons()
	{
		// �y�[�W�{�^�����쐬����
		clearPageButtons();
		pageButtons = [];
		for(var i = 0; i < 7; i++)
		{
			// �y�[�W�{�^���͉摜�ł��悢��������܂���
			// ���̏ꍇ�͌��݂̃y�[�W��\���{�^���摜�𑼂�
			// �ς���Ƃ悢����
			var obj = new RButtonLayer(window, this);
			pageButtons[i] = obj;
			obj.width = 100;
			obj.height = 25;
			obj.color = currentPage == i ? 0xff0000 : 0x000000;
			obj.caption = "�f�[�^ " + (i*4+1) + "�`" + ((i+1)*4);
			obj.captionColor = 0xffffff;
			obj.top = i * 30 + 100;
			obj.left = 10;
			obj.focusable = true;
			obj.enabled = currentPage != i;
			obj.visible = true;
			obj.tag = 'page';
			obj.page = i;
		}
	}

	function changePage(newpage)
	{
		// �y�[�W��ύX����Ƃ�
		if(pageButtons !== void)
		{
			pageButtons[currentPage].color = 0x000000;
			pageButtons[currentPage].enabled = true;
			pageButtons[newpage].color = 0xff0000;
			pageButtons[newpage].enabled = false;
			currentPage = newpage;
			makeSaveDataItems();
		}
	}

	function makeReturnButton()
	{
		// �߂� �{�^�����쐬
		if(returnButton === void)
		{
			returnButton = new RButtonLayer(window, this);
			returnButton.width = 100;
			returnButton.height = 25;
			returnButton.caption = "�߂�";
			returnButton.captionColor = 0xffffff;
			returnButton.left = 10;
			returnButton.top = 440;
			returnButton.focusable = true;
		}
		returnButton.visible = true;
	}

	function makeLoadMenu()
	{
		// �u���[�h�v���j���[
		if(state != 1)
		{
			clear();
			state = 1;
			makeSaveDataItems(currentPage);
			makeReturnButton();
			makePageButtons();
			font.height = 24;
			drawText(30, 30, "���[�h", 0xffffff);
		}
	}

	function makeSaveMenu()
	{
		// �u�Z�[�u�v���j���[
		if(state != 2)
		{
			clear();
			state = 2;
			makeSaveDataItems(currentPage);
			makeReturnButton();
			makePageButtons();
			font.height = 24;
			drawText(30, 30, "�Z�[�u", 0xffffff);
		}
	}

	function clear()
	{
		// ��ʏ�̃{�^���ނ����ׂĔ�\���ɂ��邩�A����������
		clearBase();

		closeButton.visible = false if closeButton !== void;
		saveButton.visible = false if saveButton !== void;
		loadButton.visible = false if loadButton !== void;
		hideMessageButton.visible = false if hideMessageButton !== void;

		returnButton.visible = false if returnButton !== void;

		clearSaveDataItems();
		clearPageButtons();
	}

	function saveToSystemVariable()
	{
		// �V�X�e���ϐ��Ƀf�[�^��ۑ�����K�v������Ƃ�
		if(saveDataItems !== void)
		{
			for(var i = 0; i < saveDataItems.count; i++)
				saveDataItems[i].saveToSystemVariable();
			kag.setBookMarkMenuCaptions();
		}
	}

	function onButtonClick(sender)
	{
		// �{�^���������ꂽ�Ƃ�
		switch(sender)
		{
		case closeButton: // �u����v�{�^��
			owner.onConfigClose();
			break;
		case saveButton: // �u�Z�[�u�v�{�^��
			makeSaveMenu();
			break;
		case loadButton: // �u���[�h�v�{�^��
			makeLoadMenu();
			break;
		case hideMessageButton: // �u���b�Z�[�W�������v�{�^��
			owner.closeConfig();
			kag.process('', '*hidemessage');
			break;
		case returnButton: // �u�߂�v�{�^��
			makeMainMenu();
			break;
		default:
			if(sender.tag == 'page')
			{
				// page �{�^��
				changePage(sender.page);
			}
		}
	}

	function onLoadOrSave(num)
	{
		// �ԍ� num ���Z�[�u�܂��̓��[�h
		if(state == 1)
		{
			// ���[�h
			kag.loadBookMarkWithAsk(num);
		}
		else
		{
			// �Z�[�u
			if(kag.saveBookMarkWithAsk(num))
			{
				clearSaveDataItems();
				if(kag.scflags.bookMarkComments !== void)
					kag.scflags.bookMarkComments[num] = ''; // �R�����g�͈ꉞ�N���A
				makeSaveDataItems(); // �\�����X�V
			}
		}
	}

	function show()
	{
		// ���C����\������
		visible = true;
		setMode();
		focus();
	}

	function hide()
	{
		// ���C�����B��
		removeMode();
		visible = false;
		clear();
		state = -1;
	}

	function onKeyDown(key)
	{
		super.onKeyDown(...);
		if(key == VK_ESCAPE)
			owner.onConfigClose(); // ESC �L�[�������ꂽ�烌�C�����B��
	}

	function onMouseDown(x, y, button, shift)
	{
		if(button == mbRight)
		{
			if(state == 0)
			{
				// �E�N���b�N���ꂽ�炱�̃��C�����B���悤��
				owner.onConfigClose();
			}
			else if(state == 1 || state == 2)
			{
				// �Z�[�u�E���[�h�̉�ʂ̏ꍇ�̓��C�����j���[��
				makeMainMenu();
			}
		}
	}
}


class RClickConfigPlugin extends KAGPlugin // �u�E�N���b�N�ݒ�v�v���O�C���N���X
{
	var window; // �E�B���h�E�ւ̎Q��
	var config; // �ݒ背�C��

	function RClickConfigPlugin(window)
	{
		// RClickPlugin �R���X�g���N�^
		super.KAGPlugin(); // �X�[�p�[�N���X�̃R���X�g���N�^���Ă�
		this.window = window; // window �ւ̎Q�Ƃ�ۑ�����
	}

	function finalize()
	{
		invalidate config if config !== void;
		super.finalize(...);
	}

	function show()
	{
		// �\��
		if(config === void)
			config = new RClickConfigLayer(window, kag.fore.base, this);
		config.parent = window.fore.base;
			// �e���Đݒ肷��
			// (�g�����W�V�����ɂ���ĕ\�w�i���C���͕ς�邽��)
		config.makeMainMenu();
		config.show();
	}

	function onConfigClose()
	{
		// �ݒ背�C��������Ƃ�
		closeConfig();
		window.trigger('config'); // 'config' �g���K�𔭓�����
	}

	function closeConfig()
	{
		// �ݒ背�C�������
		config.hide() if config !== void;
	}

	// �ȉ��AKAGPlugin �̃��\�b�h�̃I�[�o�[���C�h

	function onStore(f, elm)
	{
	}

	function onRestore(f, clear, elm)
	{
		// �x��ǂݏo���Ƃ�
		closeConfig();
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

	function onSaveSystemVariables()
	{
		// �Z�[�u�f�[�^�̃R�����g�͂��̃^�C�~���O�ł� scflags ��
		// �ۑ����Ȃ���΂Ȃ�Ȃ�
		if(config !== void) config.saveToSystemVariable();
	}
}


kag.addPlugin(global.rclickconfig_object = new RClickConfigPlugin(kag));


@endscript
@endif
; �E�N���b�N�ݒ�
@rclick call=true storage="rclick_tjs.ks" target="*rclick" enabled=true name="���j���[(&S) ..." enabled=true
@return


*rclick
; �E�N���b�N�ŌĂ΂��T�u���[�`��
@locksnapshot
*rclick_2
@eval exp="rclickconfig_object.show()"
; show ���\�b�h���Ă�
@waittrig name="config"
; �� 'config' �g���K��҂�
@unlocksnapshot
@return

*hidemessage
; ���b�Z�[�W�������Ƃ��ɌĂ΂��
@hidemessage
@jump target=*rclick_2
