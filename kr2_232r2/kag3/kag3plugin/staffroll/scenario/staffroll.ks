@if exp="typeof(global.staffroll_object) == 'undefined'"
@iscript

// �X�^�b�t���[���v���O�C��


class StaffRollPlugin extends KAGPlugin
{
	var foreLayers = [];
	var backLayers = [];
	var currentPos;
	var moveHeight;
	var moveTime;
	var moveStartTick;
	var moving = false;
	var vertical = false;

	function StaffRollPlugin()
	{
		super.KAGPlugin();
	}

	function finalize()
	{
		uninit();
		super.finalize(...);
	}


	function init()
	{
		// �X�^�b�t���[���̏�����
		uninit();
		vertical = kag.current.vertical;
		if(!vertical)
			currentPos = 0;
		else
			currentPos = kag.fore.base.width;
	}

	function addText(opt)
	{
		// �X�^�b�t���[���Ƀe�L�X�g��ǉ�
		var fore = new Layer(kag, kag.fore.base);
		var back = new Layer(kag, kag.back.base);
		fore.absolute = 2000000-5;
		back.absolute = fore.absolute;

		var ff = fore.font;
		var ml = kag.current;

		ml.changeLineSize() if ml.sizeChanged;

		var ref = kag.current.lineLayer;
		var reff = ref.font;
		ff.bold = reff.bold;
		ff.face = reff.face;
		ff.height = reff.height;
		ff.italic = reff.italic;
		ff.angle = reff.angle;
		ff.strikeout = reff.strikeout;
		ff.underline = reff.underline;

		var cx, cy;
		var text = opt.text;
		cx = reff.getTextWidth(text);
		cy = reff.getTextHeight(text);

		var tx, ty;

		if(!vertical)
		{
			fore.setImageSize(cx + 8, cy + 8);
			tx = 4; ty = 4;
		}
		else
		{
			fore.setImageSize(cy + 8, cx + 8);
			tx = cy + 8 - 4;
			ty = 4;
		}

		fore.face = dfBoth;
		fore.fillRect(0, 0, fore.imageWidth, fore.imageHeight, 0);

		if(ml.edge)
			fore.drawText(tx, ty, text, ml.chColor, 255, ml.antialiased, 512, ml.edgeColor, 1, 0, 0); // ����
		else if(ml.shadow)
			fore.drawText(tx, ty, text, ml.chColor, 255, ml.antialiased, 255, ml.shadowColor, 0, 2, 2); // ����
		else
			fore.drawText(tx, ty, text, ml.chColor, 255, ml.antialiased); // ����

		back.setImageSize(fore.imageWidth, fore.imageHeight);
		back.assignImages(fore);

		fore.setSizeToImageSize();
		back.setSizeToImageSize();

		var x, y;
		if(!vertical)
		{
			x = +opt.x;
			y = currentPos += +opt.y;
		}
		else
		{
			x = (currentPos -= +opt.x) - cy;
			y = +opt.y;
		}

		fore.setPos(x, y);
		back.setPos(x, y);
		if(!vertical)
		{
			fore.orgY = y;
			back.orgY = y;
		}
		else
		{
			fore.orgX = x;
			back.orgX = x;
		}

		foreLayers.add(fore);
		backLayers.add(back);
	}

	function addImage(opt)
	{
		// �X�^�b�t���[���ɉ摜��ǉ�
		var fore = new Layer(kag, kag.fore.base);
		var back = new Layer(kag, kag.back.base);
		fore.absolute = 2000000-5;
		back.absolute = fore.absolute;

		fore.loadImages(opt.storage);
		back.assignImages(fore);

		fore.setSizeToImageSize();
		back.setSizeToImageSize();

		var x, y;
		if(!vertical)
		{
			x = +opt.x;
			y = currentPos += +opt.y;
		}
		else
		{
			x = (currentPos -= +opt.x) - fore.imageWidth;
			y = +opt.y;
		}

		fore.setPos(x, y);
		back.setPos(x, y);
		if(!vertical)
		{
			fore.orgY = y;
			back.orgY = y;
		}
		else
		{
			fore.orgX = x;
			back.orgX = x;
		}

		foreLayers.add(fore);
		backLayers.add(back);
	}

	function startMove(height, time)
	{
		// �ړ����J�n

		if(moving) return;

		moveStartTick = System.getTickCount();
		moveHeight = height;
		moveTime = time;

		for(var i = 0; i < foreLayers.count; i ++)
		{
			foreLayers[i].visible = true;
			backLayers[i].visible = true;
		}

		System.addContinuousHandler(moveHandler);
		moving = true;
	}

	function uninit()
	{
		// ��~�ƃN���[���A�b�v

		if(moving)
		{
			System.removeContinuousHandler(moveHandler);
			moving = false;
		}

		for(var i = 0; i < foreLayers.count; i ++)
		{
			invalidate foreLayers[i];
			invalidate backLayers[i];
		}

		foreLayers.count = 0;
		backLayers.count = 0;
	}

	function moveHandler()
	{
		// �ړ��n���h��
		var current = System.getTickCount() - moveStartTick;
		var current = moveHeight * current \ moveTime;

		var laycount = foreLayers.count;
		var f = foreLayers;
		var b = backLayers;
		if(!vertical)
		{
			for(var i = laycount - 1; i >= 0; i--)
			{
				var fl = f[i], bl = b[i];
				fl.top = bl.top = fl.orgY - current;
			}
		}
		else
		{
			for(var i = laycount - 1; i >= 0; i--)
			{
				var fl = f[i], bl = b[i];
				fl.left = bl.left = fl.orgX + current;
			}
		}
	}
}

kag.addPlugin(global.staffroll_object = new StaffRollPlugin());
	// �v���O�C���I�u�W�F�N�g���쐬���A�o�^����

@endscript
@endif
; �}�N����`
@macro name=staffrollinit
@eval exp="staffroll_object.init()"
@endmacro
@macro name=staffrolltext
@eval exp="staffroll_object.addText(mp)"
@endmacro
@macro name=staffrollimage
@eval exp="staffroll_object.addImage(mp)"
@endmacro
@macro name=staffrollstart
@eval exp="staffroll_object.startMove(mp.width !== void ? +mp.width : +mp.height, +mp.time)"
@endmacro
@macro name=staffrolluninit
@eval exp="staffroll_object.uninit()"
@endmacro
@return
