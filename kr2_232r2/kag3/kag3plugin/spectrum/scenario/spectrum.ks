@if exp="typeof(global.spectrum_object) == 'undefined'"
@iscript

Plugins.link("fftgraph.dll"); // fftgraph.dll ��ǂݍ���

// �X�y�A�i��\��


class SpectrumPlugin extends KAGPlugin // �u�X�y�A�i�v�v���O�C���N���X
{
	var timer = void; // �^�C�}
	var left = 0;
	var top = 0;
	var width = 64;
	var height = 32;
	var options = %[];

	function SpectrumPlugin()
	{
		// SpectrumPlugin �R���X�g���N�^
		super.KAGPlugin(); // �X�[�p�[�N���X�̃R���X�g���N�^���Ă�
	}

	function finalize()
	{
		stop();
		invalidate timer if timer !== void;
		super.finalize(...);
	}

	function setOptions(elm)
	{
		// �I�v�V������ݒ�
		left = +elm.left if elm.left !== void;
		top = +elm.top if elm.left !== void;
		width = +elm.width if elm.width !== void;
		height = +elm.height if elm.height !== void;
		(Dictionary.assign incontextof options)(elm);
	}

	function start()
	{
		// �\�����J�n
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
		// �\�����~
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
		var buf; // �Ώۂ� BGM �o�b�t�@
		if(kag.bgm.buf2 !== void && kag.bgm.buf2.status == "playing")
			buf = kag.bgm.buf2;
		else
			buf = kag.bgm.buf1;

		drawFFTGraph(kag.fore.base, buf, left, top, width, height, options);
	}

	function onRestore(f, clear, elm)
	{
		// �x��ǂݏo���Ƃ�
		stop(); // ������~
	}
}

kag.addPlugin(global.spectrum_object = new SpectrumPlugin(kag));
	// �v���O�C���I�u�W�F�N�g���쐬���A�o�^����

@endscript
@endif
;
; �}�N���̓o�^
@macro name="spectrumopt"
@eval exp="spectrum_object.setOptions(mp)"
; mp ���}�N���ɓn���ꂽ���������������z��I�u�W�F�N�g
@endmacro
@macro name="spectrumstart"
@eval exp="spectrum_object.start()"
@endmacro
@macro name="spectrumstop"
@eval exp="spectrum_object.stop()"
@endmacro
@return
