
; ----------------------------------------------------------------------------
;	��ʓI�Ȓ萔
; ----------------------------------------------------------------------------

Constant	NULL := 0
Constant	true := -1, false := 0

Constant	eslErrSuccess		:= 0,
			eslErrNotSupported	:= -1,
			eslErrGeneral		:= 1,
			eslErrAbort			:= 2,
			eslErrInvalidParam	:= 3,
			eslErrTimeout		:= 4


; ----------------------------------------------------------------------------
;	�\����
; ----------------------------------------------------------------------------

; ���W
Structure	Point
	Integer	x
	Integer	y
EndStruct

; �T�C�Y
Structure	Size
	Integer	w
	Integer	h
EndStruct

; ��`
Structure	Rect
	Integer	left
	Integer	top
	Integer	right
	Integer	bottom
EndStruct

; �X�v���C�g���
Structure	SpriteParam
	Integer		nFlags					; �`��t���O
	Point		ptDstPos				; �o�͊���W
	Point		ptRevCenter				; ��]���S���W
	Real		rHorzUnit := 1.0		; �����g�嗦
	Real		rVertUnit := 1.0		; �����g�嗦
	Real		rRevAngle				; ��]�p�x [deg]
	Real		rCrossingAngle := 90.0	; �����������p [deg]
	Integer		rgbDimColor				; ���`��F
	Integer		rgbLightColor := 00FFFFFFH
	Integer		nTransparency			; �����x
	Real		rZOrder					; ���l
	Integer		rgbColorParam1			; �F�p�����[�^
EndStruct

; �`��t���O
Constant	EGL_DRAW_BLEND_ALPHA	:= 0001H
Constant	EGL_DRAW_GLOW_LIGHT		:= 0002H
Constant	EGL_WITH_Z_ORDER		:= 0004H
Constant	E3DRP_RENDER_REV_IMAGE	:= 0010H

Constant	EGL_APPLY_C_ADD			:= 00800000H
Constant	EGL_APPLY_C_MUL			:= 00820000H
Constant	EGL_APPLY_A_MUL			:= 00880000H
Constant	EGL_DRAW_F_ADD			:= 80000000H
Constant	EGL_DRAW_F_SUB			:= 81000000H
Constant	EGL_DRAW_F_MUL			:= 82000000H
Constant	EGL_DRAW_F_DIV			:= 83000000H
Constant	EGL_DRAW_A_MOVE			:= 88000000H




; �摜���\�[�X���
Structure	ImageInfo
	Integer	nFormatType
	Integer	nImageWidth
	Integer	nImageHeight
	Integer	nBitsPerPixel	; �ʏ� 32 �Œ�
	Integer	nFrameCount		; �Î~��̏ꍇ 1
	Integer	xHotSpot
	Integer	yHotSpot
EndStruct

; �摜�t�H�[�}�b�g�t���O
Constant	EIF_RGB_BITMAP		:= 00000001H
Constant	EIF_RGBA_BITMAP		:= 04000001H
Constant	EIF_GRAY_BITMAP		:= 00000002H
Constant	EIF_YUV_BITMAP		:= 00000004H
Constant	EIF_HSB_BITMAP		:= 00000006H
Constant	EIF_Z_BUFFER_R4		:= 00002005H
Constant	EIF_TYPE_MASK		:= 00FFFFFFH
Constant	EIF_WITH_PALETTE	:= 01000000H
Constant	EIF_WITH_CLIPPING	:= 02000000H
Constant	EIF_WITH_ALPHA		:= 04000000H

; ������`��p�����[�^
Structure	DrawTextParam
	Rect	rcArea				; �`��L����
	Point	ptCurPos			; �`����W
	Integer	nFlags				; �@�\�t���O
	Integer	rgbColor			; �����F
	Integer	nTransparency		; �����x
	Integer	nLineHeight			; �s��
	Integer	nIndentWidth		; �C���f���g��
	Integer	nFontSize			; �t�H���g�T�C�Y
	String	strFontFace			; �t�H���g�t�F�[�X��
EndStruct

; ������`��@�\�t���O
Constant	DTPF_VERTICAL		:= 0001H,	; �c����
			DTPF_NOSMOOTHING	:= 0002H,	; �A���`�G�C���A�X������
			DTPF_LEFT			:= 0000H,	; ���񂹁i�f�t�H���g�j
			DTPF_CENTER			:= 0010H,	; �����񂹁i�P��s�j
			DTPF_RIGHT			:= 0020H,	; �E�񂹁i�P��s�j
			DTPF_ACCORDING		:= 0030H,	; ���E�̕������i�P��s�j
			DTPF_ALIGN_MASK		:= 0030H

; �X�L���R�}���h
Structure	WndSpriteCmd
	String	strID ;			; �A�C�e�����ʎq
	Integer	nNotification	; �ʒm�R�[�h
	Integer	nParameter		; �p�����[�^
EndStruct

;  2�����x�N�g��
Structure	Vector2D
	Real	x
	Real	y
EndStruct

; 3 �����x�N�g��
Structure	Vector
	Real	x
	Real	y
	Real	z
EndStruct

; �x�W�F�Ȑ�
Structure	BezierR64
	Array	cp
	this.cp[0] := Real
	this.cp[1] := Real
	this.cp[2] := Real
	this.cp[3] := Real
EndStruct

Structure	Bezier2D
	Array	cp
	this.cp[0] := Vector2D
	this.cp[1] := Vector2D
	this.cp[2] := Vector2D
	this.cp[3] := Vector2D
EndStruct

; ����
Structure	Time
	Integer	nYear
	Integer	nMonth
	Integer	nDay
	Integer	nWeek
	Integer	nHour
	Integer	nMinute
	Integer	nSecond
EndStruct

; �\�ʑ���
Structure	SurfaceAttribute
	Integer		nShadingFlags
	Integer		rgbColorMul := 0FFFFFFH
	Integer		rgbColorAdd
	Integer		nAmbient
	Integer		nDiffusion
	Integer		nSpecular
	Integer		nTransparency
	Integer		nDeepness
EndStruct

Constant	E3DSAF_NO_SHADING		:= 00000000H,	; �V�F�[�f�B���O����
			E3DSAF_FLAT_SHADE		:= 00000001H,	; �t���b�g�V�F�[�f�B���O�i���g�p�j
			E3DSAF_GOURAUD_SHADE	:= 00000002H,	; �O�[���[�V�F�[�f�B���O
			E3DSAF_PHONG_SHADE		:= 00000003H,	; �t�H���V�F�[�f�B���O�i���g�p�j
			E3DSAF_SHADING_MASK		:= 000000FFH,
			E3DSAF_TEXTURE_TILING	:= 00000100H,	; �e�N�X�`�����^�C�����O
			E3DSAF_TEXTURE_TRIM		:= 00000200H,	; �g���~���O�e�N�X�`��
			E3DSAF_TEXTURE_SMOOTH	:= 00000400H,	; �e�N�X�`���⊮�g��
			E3DSAF_TEXTURE_MAPPING	:= 00001000H,	; �e�N�X�`���}�b�s���O
			E3DSAF_ENVIRONMENT_MAP	:= 00002000H,	; ���}�b�s���O
			E3DSAF_GENVIRONMENT_MAP	:= 00004000H,	; �O���[�o�����}�b�s���O
			E3DSAF_SINGLE_SIDE_PLANE:= 00010000H,	; �Жʃ|���S��
			E3DSAF_NO_ZBUFFER		:= 00020000H	; ����r���s��Ȃ��ŕ`��

; �F���
Structure	E3DColor
	Integer	rgbMul := 0FFFFFFH
	Integer	rgbAdd
EndStruct

; ����
Structure	LightEntry
	Integer		nLightType
	Integer		rgbColor
	Real		rBrightness
	Vector		vecLight
EndStruct

Constant	E3D_FOG_LIGHT		:= 00000000H,		; �[���t�H�b�O
			E3D_AMBIENT_LIGHT	:= 00000001H,		; ����
			E3D_VECTOR_LIGHT	:= 00000002H,		; ����������
			E3D_POINT_LIGHT		:= 00000004H		; �_����

; �����_�����O�@�\�t���O
Constant	E3D_FLAG_ANTIALIAS_SIDE_EDGE	:= 0001H,
			E3D_FLAG_TEXTURE_SMOOTHING		:= 0002H

; �\�[�g�t���O
Constant	E3D_SORT_TRANSPARENT	:= 0001H,
			E3D_SORT_OPAQUE			:= 0002H

; ���̓C�x���g
Structure	InputEvent
	Integer	idType		; �f�o�C�X�^�C�v
	Integer	iDevNum		; �f�o�C�X�ԍ��i�W���C�X�e�B�b�N�̂݁j
	Integer	iKeyNum		; ���z�L�[�R�[�h�^���z�W���C�X�e�B�b�N�{�^��
EndStruct

; �f�o�C�X�^�C�v
Constant	idKeyboard	:= 0,
			idMouse		:= 1,
			idJoyStick	:= 2

; ���z�L�[�R�[�h
; '0'�`'9'�A'A'�`'Z' �� ASCII �R�[�h�Ɠ���
Constant	VK_LBUTTON        := 01H
Constant	VK_RBUTTON        := 02H
Constant	VK_CANCEL         := 03H
Constant	VK_MBUTTON        := 04H
Constant	VK_BACK           := 08H
Constant	VK_TAB            := 09H
Constant	VK_CLEAR          := 0CH
Constant	VK_RETURN         := 0DH
Constant	VK_SHIFT          := 10H
Constant	VK_CONTROL        := 11H
Constant	VK_MENU           := 12H
Constant	VK_PAUSE          := 13H
Constant	VK_CAPITAL        := 14H
Constant	VK_KANA           := 15H
Constant	VK_HANGEUL        := 15H
Constant	VK_HANGUL         := 15H
Constant	VK_JUNJA          := 17H
Constant	VK_FINAL          := 18H
Constant	VK_HANJA          := 19H
Constant	VK_KANJI          := 19H
Constant	VK_ESCAPE         := 1BH
Constant	VK_CONVERT        := 1CH
Constant	VK_NONCONVERT     := 1DH
Constant	VK_ACCEPT         := 1EH
Constant	VK_MODECHANGE     := 1FH
Constant	VK_SPACE          := 20H
Constant	VK_PRIOR          := 21H
Constant	VK_NEXT           := 22H
Constant	VK_END            := 23H
Constant	VK_HOME           := 24H
Constant	VK_LEFT           := 25H
Constant	VK_UP             := 26H
Constant	VK_RIGHT          := 27H
Constant	VK_DOWN           := 28H
Constant	VK_SELECT         := 29H
Constant	VK_PRINT          := 2AH
Constant	VK_EXECUTE        := 2BH
Constant	VK_SNAPSHOT       := 2CH
Constant	VK_INSERT         := 2DH
Constant	VK_DELETE         := 2EH
Constant	VK_HELP           := 2FH
Constant	VK_LWIN           := 5BH
Constant	VK_RWIN           := 5CH
Constant	VK_APPS           := 5DH
Constant	VK_SLEEP          := 5FH
Constant	VK_NUMPAD0        := 60H
Constant	VK_NUMPAD1        := 61H
Constant	VK_NUMPAD2        := 62H
Constant	VK_NUMPAD3        := 63H
Constant	VK_NUMPAD4        := 64H
Constant	VK_NUMPAD5        := 65H
Constant	VK_NUMPAD6        := 66H
Constant	VK_NUMPAD7        := 67H
Constant	VK_NUMPAD8        := 68H
Constant	VK_NUMPAD9        := 69H
Constant	VK_MULTIPLY       := 6AH
Constant	VK_ADD            := 6BH
Constant	VK_SEPARATOR      := 6CH
Constant	VK_SUBTRACT       := 6DH
Constant	VK_DECIMAL        := 6EH
Constant	VK_DIVIDE         := 6FH
Constant	VK_F1             := 70H
Constant	VK_F2             := 71H
Constant	VK_F3             := 72H
Constant	VK_F4             := 73H
Constant	VK_F5             := 74H
Constant	VK_F6             := 75H
Constant	VK_F7             := 76H
Constant	VK_F8             := 77H
Constant	VK_F9             := 78H
Constant	VK_F10            := 79H
Constant	VK_F11            := 7AH
Constant	VK_F12            := 7BH
Constant	VK_F13            := 7CH
Constant	VK_F14            := 7DH
Constant	VK_F15            := 7EH
Constant	VK_F16            := 7FH
Constant	VK_F17            := 80H
Constant	VK_F18            := 81H
Constant	VK_F19            := 82H
Constant	VK_F20            := 83H
Constant	VK_F21            := 84H
Constant	VK_F22            := 85H
Constant	VK_F23            := 86H
Constant	VK_F24            := 87H

; ���z�W���C�X�e�B�b�N�{�^��
Constant	jbUp		:= 0,
			jbDown		:= 1,
			jbLeft		:= 2,
			jbRight		:= 3,
			jbButton1	:= 4,
			jbButton2	:= 5,
			jbButton3	:= 6,
			jbButton4	:= 7,
			jbButtonMax	:= jbButton1 + 32

; �g�[���t�B���^�[�t���O
Constant	EGL_TONE_BRIGHTNESS	:= 0000H,
			EGL_TONE_INVERSION	:= 0001H,
			EGL_TONE_LIGHT		:= 0002H
Constant	tffFixZero			:= 0001H,
			tffMaskWithAlpha	:= 0002H,
			tffYUVFilter		:= 0004H


; ----------------------------------------------------------------------------
;	�\���̑��l�\�z
; ----------------------------------------------------------------------------

Function	IPoint( Integer x, Integer y )
	Point	point
	point.x := x
	point.y := y
	Return	point
EndFunc

Function	ISize( Integer w, Integer h )
	Size	size
	size.w := w
	size.h := h
	Return	size
EndFunc

Function	IRect( Integer left, Integer top, Integer right, Integer bottom )
	Rect	rect
	rect.left := left
	rect.top := top
	rect.right := right
	rect.bottom := bottom
	Return	rect
EndFunc

Function	IVector2D( Real x, Real y )
	Vector2D	v
	v.x := x
	v.y := y
	Return	v
EndFunc

Function	IVector( Real x, Real y, Real z )
	Vector	v
	v.x := x
	v.y := y
	v.z := z
	Return	v
EndFunc


; ----------------------------------------------------------------------------
;	�x�N�g������
; ----------------------------------------------------------------------------

Function	Vector2D::Add( Reference v )
	Return	IVector2D( this.x + v.x, this.y + v.y )
EndFunc

Function	Vector2D::Sub( Reference v )
	Return	IVector2D( this.x - v.x, this.y - v.y )
EndFunc

Function	Vector2D::Mul( Real r )
	Return	IVector2D( this.x * r, this.y * r )
EndFunc

Function	Vector2D::Revolve( Real r )
	Vector2D	v
	Real	rad
	rad := r * 3.141592653 / 180
	v.x := this.x * rad.Cos() - this.y * rad.Sin()
	v.y := this.x * rad.Sin() + this.y * rad.Cos()
	Return	v
EndFunc

Function	Vector2D::Abs( )
	Return	(this.x * this.x + this.y * this.y).Sqrt( )
EndFunc

Function	Vector2D::RoundTo1( )
	Real	r
	r := Abs( )
	this.x /= r
	this.y /= r
	Return	this
EndFunc


; ----------------------------------------------------------------------------
;	�x�W�F�Ȑ�����
; ----------------------------------------------------------------------------

Function	BezierR64::SetLinear( Real r0, Real r1 )
	Real	rDelta
	rDelta := (r1 - r0) / 3.0
	this.cp[0] := r0
	this.cp[1] := r0 + rDelta
	this.cp[2] := r1 - rDelta
	this.cp[3] := r1
EndFunc

Function	BezierR64::SetAcceleration( Real a0, Real a1 )
	this.cp[1] := (this.cp[1] - this.cp[0]) * a0 + this.cp[0]
	this.cp[2] := (this.cp[2] - this.cp[3]) * a1 + this.cp[3]
EndFunc

Function	BezierR64::pt( Real t )
	Array	f
	f[0] := 1.0		; 6 / (6 * 1)
	f[1] := 3.0		; 6 / (2 * 1)
	f[2] := 3.0		; 6 / (1 * 2)
	f[3] := 1.0		; 6 / (1 * 6)
	;
	Real	r := 0.0
	Integer	i := 0
	Repeat
		Real	b
		b := f[3] * t.Power(i) * Real(1.0 - t).Power(3 - i)
		r += this.cp[i] * b
		i += 1
	Until	i >= 4
	;
	Return	r
EndFunc

Function	Bezier2D::SetLinear( Real x0, Real y0, Real x1, Real y1 )
	Vector2D	vDelta
	vDelta.x := (x1 - x0) / 3.0
	vDelta.y := (y1 - y0) / 3.0
	this.cp[0].x := x0
	this.cp[0].y := y0
	this.cp[1].x := x0 + vDelta.x
	this.cp[1].y := y0 + vDelta.y
	this.cp[2].x := x1 - vDelta.x
	this.cp[2].y := y1 - vDelta.y
	this.cp[3].x := x1
	this.cp[3].y := y1
EndFunc

Function	Bezier2D::SetAcceleration( Real a0, Real a1 )
	this.cp[1] := this.cp[1].Sub( this.cp[0] ).Mul( a0 ).Add( this.cp[0] )
	this.cp[2] := this.cp[2].Sub( this.cp[3] ).Mul( a1 ).Add( this.cp[3] )
EndFunc

Function	Bezier2D::SetCurve( Real r0, Real r1 )
	this.cp[1] := this.cp[1].Sub( this.cp[0] ).Revolve( r0 ).Add( this.cp[0] )
	this.cp[2] := this.cp[2].Sub( this.cp[3] ).Revolve( r1 ).Add( this.cp[3] )
EndFunc

Function	Bezier2D::pt( Real t )
	Array	f
	f[0] := 1.0		; 6 / (6 * 1)
	f[1] := 3.0		; 6 / (2 * 1)
	f[2] := 3.0		; 6 / (1 * 2)
	f[3] := 1.0		; 6 / (1 * 6)
	;
	Vector2D	v
	Integer	i := 0
	Repeat
		Real	b
		b := f[3] * t.Power(i) * Real(1.0 - t).Power(3 - i)
		v.x += this.cp[i].x * b
		v.y += this.cp[i].y * b
		i += 1
	Until	i >= 4
	;
	Return	v
EndFunc


; ----------------------------------------------------------------------------
;	�g���^�錾
; ----------------------------------------------------------------------------

DeclareType	Resource
	DeclareType	Sprite
		DeclareType	Window
		DeclareType	MessageSprite
DeclareType	ResourceManager
DeclareType	ToneFilter
DeclareType	InputFilter
DeclareType	PolygonModel
DeclareType	ModelJoint
DeclareType	RenderSprite


; ----------------------------------------------------------------------------
;	�g���@�\�萔
; ----------------------------------------------------------------------------

; �����Đ����[�h
Constant	ptfMusic :=0, ptfSE :=1, ptfVoice :=2, ptfSystem := 3

; �ҋ@����
Constant	INFINITE := -1

; �X�L���ʒm�R�[�h
Constant	ncGeneric := 0,
			ncLineUp := 1,
			ncLineDown := 2,
			ncClickColumn := 3,
			ncTracking := 4,
			ncEndTracking := 5
Constant	ncChange := 0,
			ncKillFocus := 1

; �������x��
Constant	levelWindow		:= 0000H,
			levelNormal		:= 0001H,
			levelFullScreen	:= 0003H,
			levelExclusive	:= 0007H

; �I�v�V���i���@�\�t���O
Constant	optfUseDblClick			:= 00000001H,
			optfAllowClose			:= 00000002H,
			optfBlackBack			:= 00000004H,
			optfEnableIME			:= 00000008H,
			optfAllowMinimize		:= 00000010H,
			optfGrantScreenSave		:= 00000020H,
			optfGrantMonitorSave	:= 00000040H,
			optfGrantPowerSuspend	:= 00000080H,
			optfOpenIME				:= 00010000H


