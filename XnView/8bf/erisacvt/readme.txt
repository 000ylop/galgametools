
 -----------------------------------------------------------------------------
				ERISA �R���o�[�^�[�� PSD ���C�u����
 -----------------------------------------------------------------------------


�� �n�߂�
�@���̓x�́A�uERISA �摜�R���o�[�^�[�v���_�E�����[�h���������܂��Ă��肪�Ƃ��������܂��B
�@�{�\�t�g�E�F�A�͖����Ŕz�z���Ă���܂����A���쌠�� Leshade Entis �ɋA�����Ă��܂��̂Ŗ��f�ł̍Ĕz�z�͂��������������B
�@������ PSD ���C�u�����̗��p�ɂ��܂��ẮA����𗘗p�����\�t�g�E�F�A�̔z�z�̍ۂɉ��炩�̒��쌠�ҏ���\������������Ύ��R�ɗ��p���Ă��������č\���܂���B


�� ����
�@ERISA �摜�R���o�[�^�[�́A�e��摜�t�@�C����ǂݍ��݁AERI �`���A���邢�͎w�肳�ꂽ�t�H�[�}�b�g�ɕϊ����ċL�^���邱�Ƃ��ł��܂��B
�@�ǂݍ��݂Ə����o���ɂ͈ȉ��̃t�H�[�}�b�g�ɑΉ����Ă��܂��B

	ERI, MEI  (Entis Rasterized Image format) *
	BMP  (Windows Bitmap) *
	AVI  (Windows AVI file)
	PSD  (Adobe Photoshop data file)
	PNG, TIFF, JPEG ��  (GDI+ �𗘗p) *

�@* ��̂���t�H�[�}�b�g�͏����o���ɂ��Ή����Ă���t�H�[�}�b�g�ł��B
�@MEI �`���́A�A�j���[�V���� ERI �`���̊g���q�ŁA�t�H�[�}�b�g���̂ɈႢ�͂���܂���B�i�g���q�� ERI �ł��\���܂���j
�@GDI+ �́AWindowsXP �ȍ~�ł���Ζ��Ȃ����p�ł��܂��B
�@Windows98,Me, Windows2000 �ł� gdiplus.dll �� erisacvt.exe �Ɠ����f�B���N�g���ɒu�����Ƃɂ���ė��p�ł���悤�ɂȂ�܂��B
�@gdiplus.dll �̓}�C�N���\�t�g���疳���Ŕz�z����Ă��܂��B

	http://www.microsoft.com/downloads/details.aspx?FamilyID=bfc0b436-9015-43e2-81a3-54938b6f4614

�@ERI �`���� AVI �`���A���邢�� GIF �� PNG �`���ł̓A�j���[�V�����̓ǂݍ��݂ɑΉ����Ă��܂��B�i���݁AGDI+ ���g�����A�j���[�V�����̏����o���ɂ͑Ή����Ă��܂���j
�@�܂��APSD �t�@�C���ł́A���C���[�ɑΉ����Ă��܂��B�i���C���[�̓����}�X�N�ɂ��Ή����Ă��܂��̂ŁAPSD �t�@�C���Ō����܂܂̉摜��ʂ̃t�H�[�}�b�g�ɕϊ����邱�Ƃ��ł��܂��j
�@�X�N���v�g���L�q���邱�Ƃɂ���āA���C���[����񋓂��邱�Ƃ��ł��܂��B
�@�X�ɁAerisacvt �ł́A���`���l���ւ̑Ή��A�X�ɁA���������������s���������݂̂̎����؂�o���A����ѐ؂�o�����ʂ̏o�͋@�\���L���Ă��܂��B
�@���̋@�\�ɂ���āAPSD �t�@�C�������ʃp�[�c��؂�o�������Ȃǂ��قƂ�ǎ������ł��܂��B�i�؂�o�����摜����ׂ�1�̉摜�ɂ܂Ƃ߂邱�Ƃ��ł��܂��j


�� �g����
�@erisacvt /? �ŏ������\���ł��܂��B
�@erisacvt /help �ŏ����t�@�C���̏�����\���ł��܂��B
�@�ȉ��ɃR�}���h���C���̗�������܂��B

> erisacvt sample.psd sample.eri
�@sample.psd �t�@�C���� sample.eri �t�@�C���ɕϊ����܂��B

> erisacvt sample.avi sample.mei
�@sample.avi �t�@�C���� sample.mei �t�@�C���ɕϊ����܂��B

> erisacvt /mime:image/png sample.eri sample.png
�@sample.eri �� sample.png �t�@�C���ɕϊ����܂��B

> erisacvt /l sample.xml /r report.xml /o output.txt
�@sample.xml �ɋL�q���ꂽ�����t�@�C���̓��e�ɂ��������ď������s���܂��B
�@�摜�̐؂�o�����s��ꂽ�ꍇ�ɂ́Areport.xml �ɐ؂�o���ꂽ���W���o�͂���܂��B
�@output.txt �ɂ̓X�N���v�g�� output() �֐��ŏo�͂������ʂ������o����܂��B�i���ʂ̃e�L�X�g�G�f�B�^�ŊJ���ɂ́A���s�R�[�h�� "\r\n" �ŏo�͂���悤�ɂ����ق����ǂ��ł��傤�j


�� �����t�@�C����
�@�P�� src �f�B���N�g���ɂ���t�@�C���� dst �f�B���N�g���� ERI �`���ɕϊ����ďo�͂��鏈���͈ȉ��̂悤�ɂȂ�܂��B

<?xml version="1.0" encoding="Shift_JIS"?>
<erisa>
	<env srcdir="src" dstdir="dst"/>
	<file src="*.*" dst=".\"/>
</erisa>


�@�e PSD �t�@�C���̊e���C���[�����o���āA��������ɕ��ׂ�1���̉摜�t�@�C���ɕϊ����ďo�͂��鏈���͈ȉ��̂悤�ɂȂ�܂��B

<?xml version="1.0" encoding="Shift_JIS"?>
<erisa>
	<env srcdir="src" dstdir="dst"/>
	<file src="*.psd" dst=".\">
		<arrange way="horz"/>
	</file>
</erisa>


�@�A���A���̏ꍇ�APSD �̃��C���[��`��񂪗L���Ȃ̂Łi���C���[���ɐ؂�o���ꂽ�T�C�Y�ŕ��ׂ���̂Łj�����T�C�Y�̉摜�Ƃ��ĕ��ׂ����ꍇ�ɂ͎��̂悤�ɂ��܂��B

<?xml version="1.0" encoding="Shift_JIS"?>
<erisa>
	<env srcdir="src" dstdir="dst" cut_align="1"/>
	<file src="*.psd" dst=".\">
		<cut/>
		<arrange way="horz"/>
	</file>
</erisa>

�@���̏����̈Ⴂ�́A���ۂɕ����̃��C���[����Ȃ� PSD �t�@�C����ϊ����Ă݂�Ε����邩�Ǝv���܂��B
�i���݂ɁAERI �t�@�C���� Photoshop ����ǂݍ��ނɂ́A������ bin �t�H���_���� erichan.8bi �t�@�C���� Photoshop ���C���X�g�[�����Ă���f�B���N�g���� "Plug-Ins\File Format" �t�H���_�ɃR�s�[���Ă��������j

�@PSD �t�@�C���̊e���C���[��A�Ԃŏo�͂���͎̂��̂悤�ȏ����ɂȂ�܂��B

<?xml version="1.0" encoding="Shift_JIS"?>
<erisa>
	<env srcdir="src" dstdir="dst"/>
	<file src="*.psd">
		<write dst="*_%%:0,1"/>
	</file>
</erisa>

�@��������ƁA���� PSD �t�@�C�����̌��� _00, _01,... ���t�����ꂽ�`���ŘA�ԃt�@�C������������܂��B

�@���݂ɁA����Ɠ����l�ȏ������X�N���v�g���g���Ď�������Ǝ��̂悤�ɂȂ�܂��B

<?xml version="1.0" encoding="Shift_JIS"?>
<erisa>
	<env srcdir="src" dstdir="dst"/>
	<cotopha><!-- ���t�X�N���v�g��
		function sample_func( reference imgctx )
			;
			; �t�@�C���^�C�g���̒��o
			string	file_title := imgctx.filename
			integer	title_len := file_title.Find( "." )
			if	title_len >= 0
				file_title := file_title.Left( title_len )
			endif
			;
			; �t�@�C���^�C�g���̒��o�́A���邢��
			;	Array	aParam
			;	if file_title.IsMatchUsage( "(<-.>*)", aParam ) == ""
			;		file_title := aParam[0]
			;	endif
			; �ł��\���܂���
			;
			; ���C���[���̏����o��
			for layer_img | imgctx[i]
				ImageContext	ictxSub
				print( "���C���[ ", i, ' : "', layer_img.name, '"\n' )
				;
				; ���C���[�̒��o
				ictxSub.merge( imgctx, i, 1 )
				;
				; merge �֐��̑����
				;	ictxSub.size := imgctx.size
				;	ictxSub[0] := Image( layer_img )
				; �Ƃ��Ă������ł�
				;
				ictxSub.save( file_title + "_" + string(i) )
			next
			imgctx.Remove()
		endfunc
	--></cotopha>
	<file src="*.psd" script="sample_func"/>
</erisa>

�@���Ӗ��ɔώG�̂悤�ɂ������邩������܂��񂪁A���C���[���Ȃǂ��珈���̓��e�����肵���肷��悤�ȁA�����������s���ėp�I�ȏ����t�@�C�����L�q����̂ɁA�X�N���v�g���L���Ȃ��Ƃ�����܂��B�i�܂��A�؂�o�����W�Ȃǂ�C�ӂ̏����Ńt�@�C���ɏo�͂��Ă����āA�ʂ̃v���O�����ŏ��������邱�Ƃ��\�ł��j
�@���t�X�N���v�g�̏�����d�l�Ɋւ��ẮAEntisGLS �ɓ�������Ă���u���t�X�N���v�g�d�l��.doc�v���Q�Ƃ��Ă��������B


�� PSD �ǂݍ��݃��C�u�����ɂ��ĕ⑫
�@������PSD �ǂݍ��݃��C�u�����𗘗p����ɂ́AEntisGLS ���K�v�ƂȂ�܂����A�債�����Ƃ͂���Ă��Ȃ��̂ŁA�ʂ̃��C�u�����p�ɊȒP�ɏ����������邱�Ƃł��傤�B


�� �Ō��
�@���ӌ�������Ȃǂ��C�y�ɂ��񂹂��������B

	E-mail : leshade@entis.jp
	URL    : http://www.entis.jp/

