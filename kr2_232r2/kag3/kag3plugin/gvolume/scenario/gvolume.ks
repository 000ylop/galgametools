@if exp="typeof(global.gvolume_object) == 'undefined'"
@iscript

// ���ʐݒ胁�j���[�v���O�C��

class GVolumePlugin extends KAGPlugin
{
	var window; // �E�B���h�E�ւ̎Q��
	var volumeMenu; // �u����(&O)�v���j���[

	var menuInsertPoint = 3; // ���j���[�}���ʒu
	var insertToRoot = false;
		// �� true �ɐݒ肷��� gvolmenu �^�O�ő}�����鉹�ʂ̃��j���[���A
		// ���j���[�o�[�ɒ��ڕ��т܂��B�}���ʒu�� menuInsertPoint ��
		// �w��ł��܂��B

	function GVolumePlugin(window)
	{
		super.KAGPlugin();
		this.window = window;

		if(!insertToRoot)
		{
			// �u���ʁv���j���[�A�C�e���̍쐬
			window.menu.insert(volumeMenu = new KAGMenuItem(window, "����(&O)", 0, void, false),
				menuInsertPoint);
				// �����ŁAwindow.menu �̓��j���[�̑}����A�Ō�� 3 �̓��j���[�̑}���ʒu
			volumeMenu.stopRecur = true; // ���̃��j���[�̐�̍ċA�������s�킹�Ȃ�
		}

	}

	function finalize()
	{
		super.finalize(...);
	}

	function createMenu(name, control)
	{
		// ���ʒ��߃��j���[���쐬����
		// name = ���j���[��
		// control = �R���g���[���� ('bgm' �܂��� ���ʉ��ԍ� 0 �` )
		var menu = new KAGMenuItem(window, name, 0, void, false);
		var currentvol; // ���݂̃{�����[��
		if(control == 'bgm')
		{
			// BGM
			currentvol = int(window.bgm.buf1.volume2 / 1000);
			menu.control = control;
		}
		else
		{
			// ���ʉ��� ',' �ŋ�؂��ĕ����w��ł��邪
			// �����ŉ��ʂ𓾂�͍̂ŏ��̃o�b�t�@�̂�
			var buffers = [].split(',', control);
			currentvol = int(window.se[+(buffers[0])].volume2 / 1000);
			menu.control = buffers;
			dm(currentvol);
		}

		if(!insertToRoot)
			volumeMenu.add(menu);
		else
			window.menu.insert(menu, menuInsertPoint);

		menu.stopRecur = true; // ���̃��j���[�̐�̍ċA�������s�킹�Ȃ�
		var checked = false;
		for(var i = 100; i >= 0; i-=5)
		{
			var name = i ? (i + "%" ) : "�~���[�g(&M)";
			var submenu = new KAGMenuItem(window, name, 1, onMenuClick, false);
			menu.add(submenu);
			submenu.control = control;
			submenu.volume = i;
			if(!checked && i <= currentvol) submenu.checked = true, checked = true;
		}
	}

	function onMenuClick(menu)
	{
		// ���j���[���I�����ꂽ
		if(menu.control === 'bgm')
		{
			// bgm �̉��ʂ�ݒ�
			window.bgm.setOptions(%[gvolume:menu.volume]);
		}
		else
		{
			// ���ʉ��̉��ʂ�ݒ�
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
	// �v���O�C���I�u�W�F�N�g���쐬���A�o�^����

@endscript
@endif
@macro name="gvolmenu"
@eval exp="gvolume_object.createMenu(mp.name, mp.control)"
@endmacro
@return
