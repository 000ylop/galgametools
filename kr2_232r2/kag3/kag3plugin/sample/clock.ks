@if exp="typeof(global.clock_object) == 'undefined'"
@iscript

// ���v��\������ ( KAG Plugin �T���v�� )


class ClockPlugin extends KAGPlugin // �u���v�v�v���O�C���N���X
{
	var fore, back; // �\��ʂ̃��C���Ɨ���ʂ̃��C��
	var timer; // �^�C�}
	var window; // �E�B���h�E�ւ̎Q��

	function ClockPlugin(window)
	{
		// ClockPlugin �R���X�g���N�^
		super.KAGPlugin(); // �X�[�p�[�N���X�̃R���X�g���N�^���Ă�

		fore = new Layer(window, window.fore.base);
			// �\��ʗp�̃��C����\�w�i���C����e�ɂ��č쐬
		back = new Layer(window, window.back.base);
			// ����ʗp�̃��C���𗠔w�i���C����e�ɂ��č쐬
		fore.absolute = back.absolute = 2000000-2;
			// �d�ˍ��킹�����̓��b�Z�[�W����������
			// (�u��v�v���O�C���̐ᗱ������)
		fore.hitType = back.hitType = htMask;
		fore.hitThreshold = back.hitThreshold = 256;
			// �}�E�X���b�Z�[�W�͑S�擧��

		fore.setPos(510, 0);
		back.setPos(510, 0);
			// �����ʒu

		fore.setImageSize(130, 36);
		fore.setSizeToImageSize();
		fore.fillRect(0, 0, fore.imageWidth, fore.imageHeight, 0);
			// �\�̃T�C�Y�����Ɠh��Ԃ� ( ���S���� )
		back.assignImages(fore);
		back.setSizeToImageSize();
			// ���̉摜��\����R�s�[���ė��̃T�C�Y����
		fore.visible = back.visible = fore.seen = back.seen = false;
			// �\�������\����Ԃ�
		fore.font.face = back.font.face = "�l�r �S�V�b�N";
		fore.font.height = back.font.height = 28;
			// �t�H���g�ƃT�C�Y��ݒ�

		timer = new Timer(onTimer, '');
			// �^�C�}�I�u�W�F�N�g���쐬
			// (onTimer ���C�x���g�n���h���Ƃ���)
		timer.interval = 1000;
		timer.enabled = true;
			// �����ƗL��/������ݒ�

		this.window = window; // window �ւ̎Q�Ƃ�ۑ�����
	}

	function finalize()
	{
		invalidate fore;
		invalidate back;
			// �\/���̃��C���𖳌���
		invalidate timer;
			// �^�C�}�𖳌���
		super.finalize(...);
	}

	function setOptions(elm)
	{
		// �I�v�V������ݒ�
		fore.visible = fore.seen = +elm.forevisible if elm.forevisible !== void;
			// �\�̉�/�s��
		back.visible = back.seen = +elm.backvisible if elm.backvisible !== void;
			// ���̉�/�s��
		var l = fore.left;
		var t = fore.top;
		var poschanged = false;
		(l = +elm.left, poschanged = true) if elm.left !== void;
		(t = +elm.top, poschanged = true) if elm.top !== void;
		if(poschanged)
		{
			fore.setPos(l, t);
			back.setPos(l, t);
				// �\���ʒu�̕ύX
		}
		onTimer(); // �\�����X�V
	}

	function onTimer()
	{
		// �^�C�}�̎������ƂɌĂ΂��
		if(!fore.seen && !back.seen) return;
		var current = new Date();
			// ���t�I�u�W�F�N�g���쐬
		var time = "%02d:%02d:%02d".sprintf(
			current.getHours(), current.getMinutes(),
			current.getSeconds());
			// ���t��������
		fore.fillRect(0, 0, fore.imageWidth, fore.imageHeight, 0);
			// ���n���N���A
		fore.drawText(5, 5, time, 0xffffff, 255, true, 512, 0, 5, 2, 2);
			// ������`��
		back.assignImages(fore);
			// ����ʂɉ摜���R�s�[
			// assignImages �͓����I�ɂ͉摜�����L����悤�ɂȂ邾����
			// ���s�R�X�g�̑傫�����\�b�h�ł͂Ȃ�
	}

	function onStore(f, elm)
	{
		// �x��ۑ�����Ƃ�
		var dic = f.clock = %[];
			// f.clock �Ɏ����z����쐬
		dic.foreVisible = fore.seen;
		dic.backVisible = back.seen;
		dic.left = fore.left;
		dic.top = fore.top;
			// �e���������z��ɋL�^
	}

	function onRestore(f, clear, elm)
	{
		// �x��ǂݏo���Ƃ�
		var dic = f.clock;
		if(dic === void)
		{
			// clock �̏�񂪞x�ɕۑ�����Ă��Ȃ�
			fore.visible = fore.seen = false;
			back.visible = back.seen = false;
		}
		else
		{
			// clock �̏�񂪞x�ɕۑ�����Ă���
			setOptions(%[ forevisible : dic.foreVisible, backvisible : dic.backVisible,
				left : dic.left, top : dic.top]);
				// �I�v�V������ݒ�
		}
	}

	function onStableStateChanged(stable)
	{
		// �u����v( s l p �̊e�^�O�Œ�~�� ) ���A
		// �u���s���v ( ����ȊO ) ���̏�Ԃ��ς�����Ƃ��ɌĂ΂��
	}

	function onMessageHiddenStateChanged(hidden)
	{
		// ���b�Z�[�W���C�������[�U�̑���ɂ���ĉB�����Ƃ��A�����Ƃ���
		// �Ă΂��B���b�Z�[�W���C���ƂƂ��ɕ\��/��\����؂�ւ������Ƃ���
		// �����Őݒ肷��B
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
		// ���C���̕\�������̏��̃R�s�[

		// backlay �^�O��g�����W�V�����̏I�����ɌĂ΂��

		// �����Ń��C���Ɋւ��ăR�s�[���ׂ��Ȃ̂�
		// �\��/��\���̏�񂾂�

		if(toback)
		{
			// �\����
			back.visible = fore.visible;
			back.seen = fore.seen;
		}
		else
		{
			// �����\
			fore.visible = back.visible;
			fore.seen = back.seen;
		}
	}

	function onExchangeForeBack()
	{
		// ���ƕ\�̊Ǘ���������

		// children = true �̃g�����W�V�����ł́A�g�����W�V�����I������
		// �\��ʂƗ���ʂ̃��C���\���������������ւ��̂ŁA
		// ����܂� �\��ʂ��Ǝv���Ă������̂�����ʂɁA����ʂ��Ǝv����
		// �������̂��\��ʂɂȂ��Ă��܂��B�����̃^�C�~���O�ł��̏���
		// ����ւ���΁A�����͐����Ȃ��ōςށB

		// �����ŕ\��ʁA����ʂ̃��C���Ɋւ��ĊǗ����ׂ��Ȃ̂�
		// fore �� back �̕ϐ�����
		var tmp;
		tmp = back;
		back = fore;
		fore = tmp;
	}

}

kag.addPlugin(global.clock_object = new ClockPlugin(kag));
	// �v���O�C���I�u�W�F�N�g���쐬���A�o�^����

@endscript
@endif
;
; �}�N���̓o�^
@macro name="clockopt"
@eval exp="clock_object.setOptions(mp)"
; mp ���}�N���ɓn���ꂽ���������������z��I�u�W�F�N�g
@endmacro
@return
