@call storage="snow.ks"
; �� �� plug-in ��ǂݍ���
@wait time=200

@eval exp="f.bg=''"
; �����ݓǂݍ��܂�Ă���w�i���L�����Ă���

; �}�N�� changebg_and_clear �̒�`
@macro name="changebg_and_clear"
@if exp="mp.storage != f.bg"
; �����ɓǂݍ��܂�Ă���w�i�Ɠ����Ȃ�ΐ؂�ւ������͍s��Ȃ�
@eval exp="f.bg = mp.storage"
@backlay
@layopt layer=message0 page=back visible=false
@trans method=crossfade time=300
@wt
@image * layer=base page=back
@trans method=crossfade time=300
@wt
@cm
@layopt layer=message0 page=back visible=true
@trans method=crossfade time=300
@wt
@endif
@endmacro

; �}�N�� loadbg �̒�`
@macro name="loadbg"
@image * layer=base
@eval exp="f.bg = mp.storage"
@endmacro

; �}�N�� start_select �̒�`
@macro name="start_select"
@backlay
@nowait
@history output=false
@current page=back
@endmacro

; �}�N�� end_select �̒�`
@macro name="end_select"
@trans method=crossfade time=400
@wt
@endnowait
@history output=true
@current page=fore
@endmacro

*syokai_start|�g���g����KAG�̏Љ� - ���j���[
@startanchor

; �w�i�ɉ摜��ǂݍ��݁A���b�Z�[�W���C���Ƀ��j���[��`��
@backlay
@loadbg storage="_24_5" page=back
@current page=back
@cm
@layopt layer=message0 page=back visible=true
@nowait
@history output=false
@style align=center
[font size=40 color=0x00ffff]�g���g����KAG�̏Љ�[resetfont][r]
[r]
[link target="*about_kirikiri"]�g���g���Ƃ�[endlink][r]
[link target="*about_kag"]KAG�Ƃ�[endlink][r]
[r]
[r]
[r]
[r]
[r]
[r]
[r]
[link exp="kag.shutdown()" color=0xff0000 hint="�g���g��/KAG�̏Љ���I�����܂�"]�I��[endlink]
@endnowait
@history output=true
@current page=fore

; ���b�Z�[�W���C���̃g�����W�V����
@trans method=crossfade time=800
@wt

; �ʉߋL�^
@record

; �I�������I�������܂Œ�~
@s

*to_syokai_start
; syokai_start �ɖ߂�
@backlay
@layopt layer=message0 page=back visible=false
@trans method=crossfade time=300
@wt
@jump target=*syokai_start


*about_kirikiri|�g���g���Ƃ�
@changebg_and_clear storage="_24_4"
�@�g���g���́ATJS�Ƃ����X�N���v�g������g���Ă��낢��Ȏ������邽�߂̃\�t�g�E�F�A�ł��B[l][r]
�@TJS��Java��JavaScript�𑫂��ĂR�Ŋ������悤�Ȍ���ŁAC��C++�ɔ�ׂ�ΏK�����₷�����ꂾ�Ǝv���܂��B[l][r]
�@�g���g���ł͂���TJS�Ŗ{�̂𐧌䂷�邱�Ƃɂ��A���܂��܂ȃA�v���P�[�V�������쐬���邱�Ƃ��ł��܂��B[l][r]
�@���Ƀ}���`���f�B�A�n�̋@�\�������A��r�I�ÓI�ȕ\����p����Q�c�Q�[���ɓK���Ă��܂��B[p]
*about_kirikiri2|
@cm
�@�g���g���́A���C���ƌĂ΂���ʂ��������d�ˍ��킹�ĉ�ʂ��\�����܂��B[l]���C���̓A���t�@�u�����h�ɂ��d�ˍ��킹���\�ŁA�K�w�\�����̂邱�Ƃ��ł��܂��B[l][r]
�@���C���ɂ͕W����PNG/JPEG/ERI/BMP��ǂݍ��݉\�ŁASusie-plugin�œǂݍ��݉\�Ȍ`�����g�����邱�Ƃ��ł��܂��B[l][r]
�@�`��͂��܂蕡�G�Ȃ��Ƃ͂ł��܂��񂪁A��������`�̕`���A���`�G�C���A�X�\�ȕ����\���A�摜�̊g��k����ό`���s�������ł��܂��B[l][r]
�@AVI/MPEG��SWF(Macromedia Flash)�����[�r�[�Ƃ��čĐ������邱�Ƃ��ł��܂��B[p]
*about_kirikiri3|
@cm
�@�g���g���ł�CD-DA�AMIDI�V�[�P���X�f�[�^�APCM���Đ������邱�Ƃ��ł��A���ꂼ�ꉹ�ʒ��߂��\�ł��B[l]PCM�͖����k��.WAV�t�@�C���̂ق��A�v���O�C���ōĐ��\�Ȍ`�����g���ł��AOggVorbis���Đ����邱�Ƃ��ł��܂��B[l][r]
�@PCM�͕����𓯎��ɍĐ����邱�Ƃ��ł��܂��B[l]CD-DA��MIDI�V�[�P���X�f�[�^�ł��������낤�Ǝv���Ε��������Đ��ł��܂��B[p]
*about_kirikiri4
@cm
�@���̑��A���Ӄc�[���Ƃ��āA
�����̃t�@�C������ɂ܂Ƃ߂���A�P�̂Ŏ��s�\�ȃt�@�C�����쐬���邱�Ƃ��ł���[font color=0xffff00]Releaser[resetfont]�A[l]
�g���g���{�̂̐ݒ���s��[font color=0xffff00]�g���g���ݒ�[resetfont]�A[l]
����ґ��Ńt�H���g��p�ӂ��A�v���[�����Ƀt�H���g���C���X�g�[������Ė����Ă��g����悤�ɂ���[font color=0xffff00]�����_�����O�ς݃t�H���g�쐬�c�[��[resetfont]�A[l]
�����x���������摜�t�H�[�}�b�g�Ԃ̑��ݕϊ����s��[font color=0xffff00]���߉摜�t�H�[�}�b�g�R���o�[�^[resetfont]������܂��B[l]
[r]
[r]
@start_select
[link target=*to_syokai_start]���j���[�ɖ߂�[endlink]
@end_select
[s]

*about_kag|KAG�Ƃ�
@changebg_and_clear storage="_24_4"
�@KAG�́A�r�W���A���m�x����T�E���h�m�x���̂悤�ȃm�x���n�Q�[����A�I������I��ŃX�g�[���[���i�ނ悤�ȕ����x�[�X�̃A�h�x���`���[�Q�[�����쐬���邽�߂̃L�b�g�ł��B[l][r]
�@KAG�͋g���g�����Q�[���G���W���Ƃ��ē��삳���邽�߂̃X�N���v�g�ŁA���ꎩ�̂�TJS�X�N���v�g�ŏ�����Ă��܂��B[l]KAG�p�̃X�N���v�g�́u�V�i���I�v�ƌĂ΂�ATJS�X�N���v�g�Ƃ͂܂��ʂ̂��̂ł��B[l]TJS�X�N���v�g�̓v���O���~���O�̒m�������Ȃ�K�v�ɂȂ�܂����A�V�i���I�͂��ȒP�ŋL�q���₷�����̂ł��B[l][r]
�@KAG�͋g���g���̏�ɐ��藧�V�X�e���Ȃ̂ŁA�g���g���̋@�\�̂قƂ�ǂ�KAG�Ŏg�p�ł��܂��B[p]
*about_kag3|
@cm
�@KAG�̕����\���́A�����̒ʂ�̃A���`�G�C���A�X�����\���ɉ����A[l][r]
[font size=60]�傫�ȕ���[resetfont]��\��������A[l][r]
[ruby text="��"]��[ruby text="��"]�r[ruby text="��"]��[ruby text="��"]�U[ruby text="��"]��[ruby text="��"]��[ruby text="��"]��A[l][font shadow=false edge=true edgecolor=0xff0000]����蕶���ɂ�����[resetfont]�A[l][r]
[style align=center]�Z���^�����O���Ă݂���[r]
[style align=right]�E�l�߂��Ă݂���[r][resetstyle]
[l]
[graph storage="ExQuestion.png" alt="!?"]�̂悤�ȓ���L����\�����Ă݂���A[l][r]
�ƁA���낢��ł��܂��B[p]
*about_kag4|
@position vertical=true
�@�܂��A�c�����\�������邱�Ƃ��ł��܂��B[l][r]
�@�c�����ł��������ƑS�������@�\���g�p���邱�Ƃ��ł��܂��B[p]
@layopt layer=message0 visible=false
@layopt layer=message1 visible=true
@current layer=message1
@position frame=messageframe left=20 top=280 marginl=16 margint=16 marginr=0 marginb=16 draggable=true vertical=false
�@���̂悤�Ƀ��b�Z�[�W�g�̒��Ƀ��b�Z�[�W��\�������邱�Ƃ��ł��܂��B[l]�A�h�x���`���[�Q�[���ŗǂ�����^�C�v�ł��B[p]
@layopt layer=message1 visible=false
@layopt layer=message0 visible=true
@current layer=message0
@position vertical=false
*about_kag5|
@cm
�@�����G�͂��̂悤��(��������炸[ruby text="�E"]��[ruby text="�E"]�ɂł��݂܂���)
@backlay
@image storage=uni page=back layer=0 visible=true opacity=255
@trans method=crossfade time=1000
@wt
�A���t�@�u�����h�ɂ��d�ˍ��킹���\�ł��B[l][r]
�@���̂悤��
@backlay
@layopt page=back layer=0 opacity=128
@trans method=crossfade time=1000
@wt
�����\�����邱�Ƃ��ł��܂��B[l][r]
�@�W���̏�ԂłR�܂ŏd�ˍ��킹�ĕ\���ł��܂��B[p]
@backlay
@layopt page=back layer=0 visible=false
@trans method=crossfade time=300
@wt
*about_kag6|
@cm
�@�g�����W�V����(��ʐ؂�ւ�)�ɂ͕W���łR�̎�ނ�����܂��B[l][r]
�@��͒P���ȃN���X�t�F�[�h�A[l]
@backlay
@layopt page=back layer=message0 visible=false
@trans method=crossfade time=300
@wt
@backlay
@image storage="_24" page=back layer=base
@trans method=crossfade time=3000
@wt
@backlay
@image storage="_24_4" page=back layer=base
@trans method=crossfade time=3000
@wt
@backlay
@layopt page=back layer=message0 visible=true
@trans method=crossfade time=300
@wt
[l][r]
�@������̓X�N���[�����ʂ��o�����Ƃ̂ł���X�N���[���g�����W�V�����A[l]
@backlay
@layopt page=back layer=message0 visible=false
@trans method=crossfade time=300
@wt
@backlay
@image storage="_24" page=back layer=base
@trans method=scroll from=right stay=stayfore time=3000
@wt
@backlay
@image storage="_24_4" page=back layer=base
@trans method=scroll from=bottom stay=nostay time=3000
@wt
@backlay
@layopt page=back layer=message0 visible=true
@trans method=crossfade time=300
@wt
[l][r]
�@�����čŌ�͐���҂����R�Ƀp�^�[�����쐬�ł��郆�j�o�[�T���g�����W�V�����ł��B[l][r]
�@���j�o�[�T���g�����W�V�����̓��[���摜�ƌĂ΂��O���[�X�P�[���̉摜��p�ӂ��A���̉摜�̈Â��Ƃ��납���葁���؂�ւ����n�܂���̂ł��B[l][r]
�@���Ƃ��΁A[l]
@image layer=base page=fore storage="trans1"
���̂悤�ȃ��[���摜�ł���΁E�E�E[l]
@backlay
@layopt page=back layer=message0 visible=false
@image storage="_24_4" page=back layer=base
@trans method=crossfade time=300
@wt
@backlay
@image storage="_24" page=back layer=base
@trans method=universal rule="trans1" vague=64 time=3000
@wt
@backlay
@image storage="_24_4" page=back layer=base
@trans method=universal rule="trans1" vague=64 time=3000
@wt
@backlay
@layopt page=back layer=message0 visible=true
@trans method=crossfade time=300
@wt
[r]
�@���Ƃ��΁A[l]
@image layer=base page=fore storage="nami"
���̂悤�ȃ��[���摜�ł���΁E�E�E[l]
@backlay
@layopt page=back layer=message0 visible=false
@image storage="_24_4" page=back layer=base
@trans method=crossfade time=300
@wt
@backlay
@image storage="_24" page=back layer=base
@trans method=universal rule="nami" vague=64 time=3000
@wt
@backlay
@image storage="_24_4" page=back layer=base
@trans method=universal rule="nami" vague=64 time=3000
@wt
@backlay
@layopt page=back layer=message0 visible=true
@trans method=crossfade time=300
@wt
[r]
�@�Ƃ��������ŁA���낢���邱�Ƃ��ł��܂��B[p]
*about_kag7|KAG�Ƃ�
@cm
�@BGM�Ƃ��Ă�CD-DA�AMIDI�APCM�̂����ꂩ���g�p�ł��܂��B[l]���ʉ��ɂ�PCM���g�p�ł��܂��B[l]��������t�F�[�h�Ȃǂ̉��ʐ��䂪�ł��܂��B[l][r]
�@PCM�͕W���Ŗ����k��.WAV���Đ��ł��܂��B[l]�܂��A�v���O�C���ōĐ��\�Ȍ`�����g���ł��AOggVorbis���Đ��ł��܂��B[l][r]
�@���[�r�[��AVI/MPEG/SWF���Đ��ł��܂��B[p]
*about_kag8|
@cm
�@KAG�̕ϐ��͕�����ł����l�ł�����邱�Ƃ��ł��A�ϐ��̐��͖������A������̒������������A���l�͐��������łȂ��������������Ƃ��ł��܂��B[l]�����KAG�̕ϐ��̎d�l�Ƃ������AKAG�̃x�[�X�ƂȂ��Ă���TJS�̎d�l�ł��B[l][r]
�@�ϐ��ɂ̓Q�[���ϐ��ƃV�X�e���ϐ��̂Q��ނ����āA�Q�[���ϐ��͞x�ƂƂ��ɓǂݍ��܂ꂽ��ۑ����ꂽ�肵�܂����A�V�X�e���ϐ��͞x�Ƃ͊֌W�Ȃ��A��ɓ������e��ۂ��Ƃ��ł�����̂ł��B[l][r]
�@�ϐ����g������������܂��E�E�E[p]
@eval exp="f.v1 = intrandom(1, 9)"
@eval exp="f.v2 = intrandom(1, 9)"
@eval exp="f.ans = f.v1 * f.v2"
@eval exp="f.input = ''"
*about_kag_var_0|�v�Z���
@cm
�@�v�Z���ł��B[emb exp="f.v1"] �~ [emb exp="f.v2"] �́H[r]
[font size=20](���̓��͗��ɓ��͂�����A�悱�́uOK�v���N���b�N���Ă�������)[resetfont][r]
[r]
@start_select
�@[edit name="f.input" length=200 opacity=80 bgcolor=0x000000 color=0xffffff] [link target=*about_kag_var_1]�@�@�@OK�@�@�@[endlink][r]
[r]
�@[link target=*about_kag_9]�ʓ|�Ȃ̂łƂ΂�[endlink]
@end_select
@eval exp="kag.fore.messages[0].links[0].object.focus()"
; �����͗��Ƀt�H�[�J�X��ݒ肷��
; �u�V�X�e�� - �O�ɖ߂�v�ł��̈ʒu�ɖ߂��悤�ɂ����ŒʉߋL�^���s��
@record
[s]

*about_kag_var_1
@commit
@jump cond="str2num(f.input) == f.ans" target=*about_kag_var_correct
@cm
�@�s�����I[l][r]
�@������x���͂��Ă��������B[p]
@jump target=*about_kag_var_0

*about_kag_var_correct
@cm
�@�����ł��I[p]
@jump target=*about_kag_9

*about_kag_9|
@cm
@snowinit forevisible=true backvisible=false
�@KAG�̑傫�ȓ����Ƃ��āA���̍����g�����ƃJ�X�^�}�C�Y�����������܂��B[l]KAG�����ł͎����ł��Ȃ��悤�ȋ@�\���ATJS���g���Ē��ڋg���g���𐧌䂷��΂��낢��Ȏ����ł��܂��B[l][r]
�@���Ƃ��΁AKAG�p�v���O�C���Ƃ��āu��v��\��������v���O�C����ǂݍ��߂΁A���̂悤�ɐ��\�������邱�Ƃ��ł��܂��B[l]�ق��ɂ��g�����W�V�����̎�ނ𑝂₷�v���O�C���Ȃǂ�����܂��B[l][r]
�@�܂��AKAG���̂��̂�TJS�X�N���v�g�ŏ�����Ă��邽�߁A�X�N���v�g��ύX����΋��X�ɂ킽���ē�����J�X�^�}�C�Y���鎖���ł��܂��B[p]
@backlay
@snowopt backvisible=false
@trans method=crossfade time=1000
@wt
@snowuninit
*about_kag_fin|KAG�̏Љ���܂�
@cm
�@KAG�̏Љ�͂���ł����܂��ł��B[l][r]
�@�݂Ȃ��������g���g��/KAG���g���Ă��΂炵���Q�[��������Ă��������B[l][r]
[r]
@start_select
[link exp="System.shellExecute('http://www.piass.com/kpc/')" hint="�g���g��/KAG���i�ψ�����J���܂�"]�g���g��/KAG���i�ψ���[endlink][r]
[link exp="System.shellExecute('http://kikyou.info/tvp/')" hint="�_�E�����[�h�y�[�W���J���܂�"]�g���g���_�E�����[�h�y�[�W[endlink][r]
[r]
[link target=*to_syokai_start]���j���[�ɖ߂�[endlink]
@end_select
[s]
