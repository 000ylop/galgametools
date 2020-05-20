@if exp="typeof(global.rain_object) == 'undefined'"
@iscript

/*
	�J���ӂ点��v���O�C��
*/

class RainGrain
{
	// �J���̃N���X

	var fore; // �\��ʂ̉J���I�u�W�F�N�g
	var back; // ����ʂ̉J���I�u�W�F�N�g
	var xvelo; // �����x
	var yvelo; // �c���x
	var l, t; // ���ʒu�Əc�ʒu
	var ownwer; // ���̃I�u�W�F�N�g�����L���� RainPlugin �I�u�W�F�N�g
	var spawned = false; // �J�����o�����Ă��邩
	var window; // �E�B���h�E�I�u�W�F�N�g�ւ̎Q��

	function RainGrain(window, n, owner)
	{
		// RainGrain �R���X�g���N�^
		this.owner = owner;
		this.window = window;

		fore = new Layer(window, window.fore.base);
		back = new Layer(window, window.back.base);

		fore.absolute = 2000000-1; // �d�ˍ��킹�����̓��b�Z�[�W����������
		back.absolute = fore.absolute;

		fore.hitType = htMask;
		fore.hitThreshold = 256; // �}�E�X���b�Z�[�W�͑S�擧��
		back.hitType = htMask;
		back.hitThreshold = 256;

		fore.loadImages("rain_0" + "_" + intrandom(0, 3)); // �摜��ǂݍ���
		back.assignImages(fore);
		var h = int(fore.imageHeight * (1.0 - n * 0.1));
		fore.setSizeToImageSize(); // ���C���̃T�C�Y���摜�̃T�C�Y�Ɠ�����
		back.setSizeToImageSize();
		fore.height = h;
		fore.imageTop = -(fore.imageHeight - h);
		back.height = h;
		back.imageTop = -(fore.imageHeight - h);
		var opa = 255 - n * 20;
		fore.opacity = opa;
		back.opacity = opa;
		xvelo = 0; //Math.random() - 0.5; // ���������x
		yvelo = (5 - n) * 30 + 75 + Math.random() * 15; // �c�������x
	}

	function finalize()
	{
		invalidate fore;
		invalidate back;
	}

	function spawn()
	{
		// �o��
		l = Math.random() * window.primaryLayer.width; // �������ʒu
		t = -fore.height; // �c�����ʒu
		spawned = true;
		fore.setPos(l, t);
		back.setPos(l, t); // ����ʂ̈ʒu��������
		fore.visible = owner.foreVisible;
		back.visible = owner.backVisible;
	}

	function resetVisibleState()
	{
		// �\���E��\���̏�Ԃ��Đݒ肷��
		if(spawned)
		{
			fore.visible = owner.foreVisible;
			back.visible = owner.backVisible;
		}
		else
		{
			fore.visible = false;
			back.visible = false;
		}
	}

	function move()
	{
		// �J���𓮂���
		if(!spawned)
		{
			// �o�����Ă��Ȃ��̂ŏo������@�����������
			if(Math.random() < 0.002) spawn();
		}
		else
		{
			l += xvelo;
			t += yvelo;
			if(t >= window.primaryLayer.height)
			{
				t = -int(fore.height * Math.random());
				l = Math.random() * window.primaryLayer.width;
			}
			fore.setPos(l, t);
			back.setPos(l, t); // ����ʂ̈ʒu��������
		}
	}

	function exchangeForeBack()
	{
		// �\�Ɨ��̊Ǘ�������������
		var tmp = fore;
		fore = back;
		back = tmp;
	}
}

class RainPlugin extends KAGPlugin
{
	// �J��U�炷�v���O�C���N���X

	var rains = []; // �J��
	var timer; // �^�C�}
	var window; // �E�B���h�E�ւ̎Q��
	var foreVisible = true; // �\��ʂ��\����Ԃ��ǂ���
	var backVisible = true; // ����ʂ��\����Ԃ��ǂ���

	function RainPlugin(window)
	{
		super.KAGPlugin();
		this.window = window;
	}

	function finalize()
	{
		// finalize ���\�b�h
		// ���̃N���X�̊Ǘ����邷�ׂẴI�u�W�F�N�g�𖾎��I�ɔj��
		for(var i = 0; i < rains.count; i++)
			invalidate rains[i];
		invalidate rains;

		invalidate timer if timer !== void;

		super.finalize(...);
	}

	function init(num, options)
	{
		// num �̉J�����o��������
		if(timer !== void) return; // ���łɉJ���͂łĂ���

		// �J�����쐬
		for(var i = 0; i < num; i++)
		{
			var n = intrandom(0, 5); // �J���̑傫�� ( �����_�� )
			rains[i] = new RainGrain(window, n, this);
		}
		rains[0].spawn(); // �ŏ��̉J�������͍ŏ�����\��

		// �^�C�}�[���쐬
		timer = new Timer(onTimer, '');
		timer.interval = 80;
		timer.enabled = true;

		foreVisible = true;
		backVisible = true;
		setOptions(options); // �I�v�V������ݒ�
	}

	function uninit()
	{
		// �J��������
		if(timer === void) return; // �J���͂łĂ��Ȃ�

		for(var i = 0; i < rains.count; i++)
			invalidate rains[i];
		rains.count = 0;

		invalidate timer;
		timer = void;
	}

	function setOptions(elm)
	{
		// �I�v�V������ݒ肷��
		foreVisible = +elm.forevisible if elm.forevisible !== void;
		backVisible = +elm.backvisible if elm.backvisible !== void;
		resetVisibleState();
	}

	function onTimer()
	{
		// �^�C�}�[�̎������ƂɌĂ΂��
		var raincount = rains.count;
		for(var i = 0; i < raincount; i++)
			rains[i].move(); // move ���\�b�h���Ăяo��
	}

	function resetVisibleState()
	{
		// ���ׂẲJ���� �\���E��\���̏�Ԃ��Đݒ肷��
		var raincount = rains.count;
		for(var i = 0; i < raincount; i++)
			rains[i].resetVisibleState(); // resetVisibleState ���\�b�h���Ăяo��
	}

	function onStore(f, elm)
	{
		// �x��ۑ�����Ƃ�
		var dic = f.rains = %[];
		dic.init = timer !== void;
		dic.foreVisible = foreVisible;
		dic.backVisible = backVisible;
		dic.rainCount = rains.count;
	}

	function onRestore(f, clear, elm)
	{
		// �x��ǂݏo���Ƃ�
		var dic = f.rains;
		if(dic === void || !+dic.init)
		{
			// �J�͂łĂ��Ȃ�
			uninit();
		}
		else if(dic !== void && +dic.init)
		{
			// �J�͂łĂ���
			init(dic.rainCount, %[ forevisible : dic.foreVisible, backvisible : dic.backVisible ] );
		}
	}

	function onStableStateChanged(stable)
	{
	}

	function onMessageHiddenStateChanged(hidden)
	{
	}

	function onCopyLayer(toback)
	{
		// ���C���̕\���������̃R�s�[
		// ���̃v���O�C���ł̓R�s�[���ׂ����͕\���E��\���̏�񂾂�
		if(toback)
		{
			// �\����
			backVisible = foreVisible;
		}
		else
		{
			// �����\
			foreVisible = backVisible;
		}
		resetVisibleState();
	}

	function onExchangeForeBack()
	{
		// ���ƕ\�̊Ǘ���������
		var raincount = rains.count;
		for(var i = 0; i < raincount; i++)
			rains[i].exchangeForeBack(); // move ���\�b�h���Ăяo��
	}
}

kag.addPlugin(global.rain_object = new RainPlugin(kag));
	// �v���O�C���I�u�W�F�N�g���쐬���A�o�^����

@endscript
@endif
; �}�N���o�^
@macro name="raininit"
@eval exp="rain_object.init(17, mp)"
@endmacro
@macro name="rainuninit"
@eval exp="rain_object.uninit()"
@endmacro
@macro name="rainopt"
@eval exp="rain_object.setOptions(mp)"
@endmacro
@return
