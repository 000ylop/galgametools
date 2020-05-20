@if exp="typeof(global.zoom_object) == 'undefined'"
@iscript

/*
	�w�i/�O�i�摜�̊g��k�����ʂɂ��\�����s���v���O�C��
*/

class ZoomPlugin extends KAGPlugin
{
	var tempLayer; // �e���|�������C��
	var overlayLayer; // �I�[�o�[���C���C��

	var sl, st, sw, sh;
	var dl, dt, dw, dh;
	var startTick; // �J�n�e�B�b�N
	var time; // �Y�[�����s���Ă��鎞��
	var mode; // �O�i���C�����[�h
	var accel; // �����x�I�ȓ������s���� ( �� : 0 : �� )
	var storage;
	var moving = false;
	var nextstop;
	var moveFunc; // �ړ��ʒu�v�Z�p�֐�
	var targetLayerName; // �Ώۃ��C����
	var targetLayer; // �Ώۃ��C��

	function ZoomPlugin(window)
	{
		super.KAGPlugin();
		this.window = window;
	}

	function finalize()
	{
		// finalize ���\�b�h
		// ���̃N���X�̊Ǘ����邷�ׂẴI�u�W�F�N�g�𖾎��I�ɔj��
		stop();

		invalidate tempLayer if tempLayer !== void;
		invalidate overlayLayer if overlayLayer !== void;

		super.finalize(...);
	}

	function startZoom(storage, layer, mode, basestorage, sl, st, sw, sh, dl, dt, dw, dh, time, accel)
	{
		// storage : �\���摜
		// layer : �Ώۃ��C��
		// mode : �O�i���C���̓��߃��[�h
		// bgimage : �w�i�摜
		// sl st sw sh : �����ʒu
		// dl dt dw dh : �ŏI�ʒu
		// time : �Y�[�����s���Ă��鎞��
		// accel : ���������邩�ǂ���

		// �����̓�����~
		stop();

		// �Ώۃ��C��������
		if(layer == '' || layer == 'base')
			targetLayer = window.fore.base;
		else
			targetLayer = window.fore.layers[+layer];

		// �w�i�摜�̓ǂݍ���
		if(basestorage !== void)
			window.tagHandlers.image(%[ storage : basestorage, layer : layer, page : 'fore']);

		// �I�u�W�F�N�g�Ƀp�����[�^���R�s�[
		this.sl = sl; this.st = st; this.sw = sw; this.sh = sh;
		this.dl = dl; this.dt = dt; this.dw = dw; this.dh = dh;
		this.time = time;
		this.accel = accel;
		this.storage = storage;
		this.targetLayerName = layer;
		this.mode = mode;

		// tempLayer �m��
		var base = window.fore.base;
		if(tempLayer === void)
		{
			tempLayer = new Layer(window, base);
			tempLayer.loadImages(storage);
		}

		// overlayLayer �m��
		if(overlayLayer === void)
		{
			overlayLayer = new Layer(window, base);
			overlayLayer.absolute = targetLayer.absolute + 1; // �Ώۃ��C���̂�����O
			overlayLayer.hitType = htMask;
			overlayLayer.hitThreshold = 256; // �}�E�X���b�Z�[�W�͑S�擧��
			overlayLayer.face = dfBoth;
			overlayLayer.type = layer == 'base' ? ltCoverRect : (mode == 'rect' ? ltCoverRect : ltTransparent);
			// overlayLayer �� �����T�C�Y���邢�͍ŏI�T�C�Y�̂ǂ��炩�傫����
			// �̃T�C�Y�ɂȂ邪�A��ʃT�C�Y���͑傫���Ȃ�Ȃ�
			var mw = sw > dw ? sw : dw;
			var mh = sh > dh ? sh : dh;
			overlayLayer.setImageSize(
				mw < base.imageWidth ? mw : base.imageWidth,
				mh < base.imageHeight ? mh : base.imageHeight);
		}

		// �ړ��ʒu�v�Z�֐��̐ݒ�
		moveFunc = defaultMover;

		// �����ʒu�ɕ\��
		moveFunc(moveAt, 0);
		overlayLayer.visible = true;
		if(layer != 'base') targetLayer.visible = false;

		// �J�n
		startTick = System.getTickCount();
		System.addContinuousHandler(continuousHandler);
		moving = true;
		nextstop = false;
	}

	function moveAt(l, t, w, h)
	{
		// l t w h �ʒu�Ɉړ�

		// ���C���ړ�
		var base = window.fore.base;
		var oll = l < 0 ? 0 : l;
		var olt = t < 0 ? 0 : t;
		var olw = l + w > base.imageWidth ? base.imageWidth - oll : l + w - oll;
		var olh = t + h > base.imageHeight ? base.imageHeight - olt : t + h - olt;
		if(olw > 0 && olh > 0)
		{
			overlayLayer.visible = true;
			overlayLayer.setPos(oll, olt, olw, olh);

			// �g��k���]��
			overlayLayer.stretchCopy(l - oll, t - olt, w, h,
				tempLayer, 0, 0, tempLayer.imageWidth, tempLayer.imageHeight);
				// �ړ��悪�E�≺�ɂ͂ݏo��ꍇ�ɂ�����Ɩ��ʂȓ]�����N���邩��
		}
		else
		{
			overlayLayer.visible = false;
		}
	}

	/*static*/ function defaultMover(func, tm)
	{
		// �ʒu�v�Z
		// tm �� 0.0(�J�n�_) �` 1.0(�I���_) �̊Ԃŕω�����ϐ��Ȃ̂ŁA
		// ��������ɂ��Ĉʒu���v�Z����
		var l = (int)((dl - sl) * tm + sl);
		var t = (int)((dt - st) * tm + st);
		var w = (int)((dw - sw) * tm + sw);
		var h = (int)((dh - sh) * tm + sh);

		// �ړ�
		func(l, t, w, h);
	}

	function continuousHandler(tick)
	{
		// �n���h��
		if(nextstop)
		{
			// �I��
			finish();
			return;
		}

		// ���Ԃ𓾂�
		var tm = tick - startTick;
		tm /= time;
		if(tm >= 1)
		{
			nextstop = true;
			tm = 1;
		}
		else
		{
			// �����v�Z
			if(accel < 0)
			{
				// �㌷ ( �ŏ��������������A���X�ɒx���Ȃ� )
				tm = 1.0 - tm;
				tm = Math.pow(tm, -accel);
				tm = 1.0 - tm;
			}
			else if(accel > 0)
			{
				// ���� ( �ŏ��͓������x���A���X�ɑ����Ȃ� )
				tm = Math.pow(tm, accel);
			}
		}

		// �ړ�
		moveFunc(moveAt, tm);
	}

	function finish()
	{
		// �Y�[���̏I��
		if(targetLayerName == 'base')
		{
			// �w�i���C���̏ꍇ
			var base = window.fore.base;
			if(dl == 0 && dt == 0 && dw == base.imageWidth && dh == base.imageHeight &&
				tempLayer.imageWidth == base.imageWidth && tempLayer.imageHeight == base.imageHeight)
			{
				// �ŏI�ʒu����ʑS�̂𕢂��Ă��āA���A
				// �Y�[���������摜�̃T�C�Y���w�i�摜�Ɠ����ꍇ
				// �摜��ǂݍ���
				window.tagHandlers.image(%[ storage : storage, layer : 'base', page : 'fore']);
			}
			else
			{
				// �ŏI�ʒu����ʑS�̂𕢂��Ă��Ȃ�;
				// ���̏ꍇ�́A�ŏI�ʒu�̃T�C�Y�� 0 �łȂ�����
				// ���̏�Ԃ���̞x�̍ĊJ�͕s�\ ( �w�i�摜���č\���ł��Ȃ� )
				// ( �w�i�ɉ摜��ǂݍ��񂾂��ƂȂ�� OK )
				window.fore.base.face = dfBoth;
				window.fore.base.stretchCopy(dl, dt, dw, dh, tempLayer, 0, 0,
					tempLayer.imageWidth, tempLayer.imageHeight);
			}
		}
		else
		{
			// �O�i���C���̏ꍇ
			if(dw == tempLayer.imageWidth && dh == tempLayer.imageHeight)
			{
				// �ŏI�ʒu�̃T�C�Y���O�i���C���Ɠ����ꍇ
				// �摜��ǂݍ���
				window.tagHandlers.image(%[ storage : storage, layer : targetLayerName, page : 'fore',
					left : dl, top : dt, visible : true, mode : mode]);
			}
			else
			{
				// �����łȂ��ꍇ
				// ���̏ꍇ�́A�ŏI�ʒu�̃T�C�Y�� 0 �Ŗ�������A
				// ���̏�Ԃ���x�̍ĊJ�͕s�\
				// (�Ώۂ̃��C���ɉ摜��������x�ǂݒ����Ȃ�Ή�)
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
		stop(); // ��~
	}

	function stop()
	{
		// ��~
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
		// �x��ۑ�����Ƃ�
	}

	function onRestore(f, clear, elm)
	{
		// �x��ǂݏo���Ƃ�
		stop(); // ������~
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
	// �v���O�C���I�u�W�F�N�g���쐬���A�o�^����

@endscript
@endif
; �}�N���o�^
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
