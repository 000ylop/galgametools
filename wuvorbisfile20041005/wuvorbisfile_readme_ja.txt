wuvorbisfile �̎g����

�� �T�v -------------------------------------------------------------------

�@wuvorbisfile �́A�g���g���p�� OggVorbis �f�R�[�_ �v���O�C���ł���
wuvorbis.dll ���A��ʂ̃v���O��������g�p�\�ɂ��邽�߂̃C���^�[�t�F�[�X
���C�u���� (�X�^�u���C�u����) �ł��Bwuvorbis.dll �� SSE �� 3DNow! �����ɍ�
�K�����ꂽ�����\�� OggVorbis �f�R�[�_�ł��B
�@wuvorbis.dll �ł́A�ʏ�� libvorbis (���t�@�����X�f�R�[�_) �� VC++ 6.0
�Łu���s���x�v�œK���ɂ����ăR���p�C�����s�����f�R�[�_�Ɣ�ׁA
Pentium4 + SSE �g�p���� �� 1.4 �` 1.8 �{�AK6-3 + 3DNow! �g�p����
�� 1.5 �` 1.9 �{�̑��x�Ńf�R�[�h���s�����Ƃ��o���܂��B


�� ����� ---------------------------------------------------------------

�@wuvorbis.dll �� 486 ������ CPU (�܂� i386) �ł͓��삵�܂���̂Œ��ӂ��K
�v�ł����A������ Windows �}�V���� i386 ���g�p���Ă���R���s���[�^�͂܂��Ȃ�
�Ǝv���܂��Bwuvorbis.dll ���g���ꍇ�� i386 �𓮍�Ώۂ���O���K�v�������
���B


�� �ΏۊJ���� -----------------------------------------------------------

�@wuvorbisfile �́ABorland C++ Builder 5 ����� Visual C++ 6.0 �œ�����m�F
���Ă��܂��B�����炭 �����Œ񋟂���Ă��� Borland �� C++ �R���p�C����
Visual Studio .NET �ł͂��̂܂܂����܂��Bgcc �ł������̏C���͕K�v��������
�܂��񂪁Awuvorbisfile �� wuvorbis.dll �ւ̃C���^�[�t�F�[�X��񋟂��邾����
���̂ŁA�ȒP�ɏC���ł���Ǝv���܂��B


�� vorbisfile API ---------------------------------------------------------

�@��{�I�� wuvorbisfile �Ŏg�p�\�ɂȂ�͖̂{�Ƃ� vorbisfile (Ogg Vorbis ��
�ȒP�Ɉ������߂̃��C�u����) �� API �Ɠ����ł��Bvorbisfile �ɂ��Ă�

http://www.xiph.org/ogg/vorbis/doc/vorbisfile/

�@�Ȃǂ��Q�Ƃ��Ă��������B

�@�������A�ȉ��̒��ӓ_������܂��B

�E�萔��֐����A�\���̖��̑O�� "WU_" �܂��� "wu_" �̃v���t�B�N�X�����Ă�
  �܂��B
�EFILE �\���̂��󂯓n������悤�� API �͎g���܂���Bwu_ov_open_callbacks ��
  �g�p���ăt�@�C�����J���K�v������܂��B
�Ewu_ov_read �́Abigendianp=0�Asgned=1�Aword �� 2 (16bit) ���邢��
  4 (float) �݂̂��g���܂��Bfloat �ɂ��ǂݏo���� (ov_read �̋@�\�Ƃ��Ă�)
  �{�Ƃ̃��C�u�����ɂ͂Ȃ��@�\�ł��B

�@wuvorbisfile �́A�{�Ƃ� libvorbis/libogg ���C�u�����̃w�b�_�t�@�C���Ɉˑ�
���Ă��܂���B�܂�A�{�Ƃ̃w�b�_�t�@�C�����g�p���邱�ƂȂ��A�e�@�\���g�p
�ł��܂��B


�� WuVorbisInit -----------------------------------------------------------

�@WuVorbisInit �́Awuvorbis.dll ��ǂݍ��݁A�e�֐����g�p�\�ɂ��܂��Bwu_*
�e�֐����g�p����O�ɕK���Ăяo���K�v������܂��B

�E��`

  int WuVorbisInit(const char *dll_file_name_can_be_null);

�E����
  dll_file_name_can_be_null
    �@wuvorbis.dll �̃t�@�C������\���[���I��������ւ̃|�C���^�ł��BNULL ��
    �w�肷��ƃf�t�H���g�̌����p�X��� wuvorbisfile.dll ���g���܂��B
    �\�t�g�E�F�A�̃��C���̎��s�\�t�@�C���Ɠ����ꏊ�� wuvorbis.dll ��u����
    ���� NULL ���w�肷�邱�Ƃ��ł��܂��B�ڂ����� Win32 Platform SDK ��
    LoadLibrary �̍����Q�Ƃ��Ă��������B

�E�߂�l
  0 : ����
  1 : DLL ��������Ȃ����ǂݍ��߂Ȃ�
  2 : DLL �͓ǂݍ��߂����֐��̃C���|�[�g�Ɏ��s�����A���邢�͓ǂݍ��� DLL ��
      wuvorbis.dll �ł͂Ȃ�
  3 : DLL �� wuvorbisfile �̊Ԃŕs��������������


�� WuVorbisUninit -----------------------------------------------------------

�@WuVorbisUninit �́Awuvorbis.dll ��������܂��B

�E��`

  int WuVorbisUninit(void);

�E�߂�l
  0 : ����
  1 : �ُ�

�� wu_DetectCPU -----------------------------------------------------------

�@wu_DetectCPU �́ACPU �̎�ނ�ACPU �ŗ��p�\�Ȋe�@�\�����o���܂��B

�E��`

  unsigned __int32 wu_DetectCPU(void)

�E�߂�l
  CPU �@�\�t���O

  �@CPU �@�\�t���O�͈ȉ��̒l�̃r�b�g���Ƃ̘_���a�ł��B��������O�Ƃ��� 586
  ������ CPU�A���邢�� 586 �̖����t���Ă��Ă����g�� 486 �����̈ꕔ�� CPU ��
  �ꍇ�� 0 ���߂�܂��B

  #define WU_CPU_HAS_FPU 0x000010000
  #define WU_CPU_HAS_MMX 0x000020000
  #define WU_CPU_HAS_3DN 0x000040000
  #define WU_CPU_HAS_SSE 0x000080000
  #define WU_CPU_HAS_CMOV 0x000100000
  #define WU_CPU_HAS_E3DN 0x000200000
  #define WU_CPU_HAS_EMMX 0x000400000
  #define WU_CPU_HAS_SSE2 0x000800000
  #define WU_CPU_HAS_TSC 0x001000000

  �@���Ƃ��΁ACPU �� SSE �������Ă���ꍇ�́AWU_CPU_HAS_SSE �̕\���r�b�g��
  1 �ɂȂ��Ă��܂��B

  �@WU_CPU_VENDOR_MASK �Ƃ̃r�b�g���Ƃ̘_���ς��Ƃ�ƁACPU �̃x���_�[�𓾂�
  ���Ƃ��o���܂��B�x���_�[�͈ȉ��̂悤�ɒ�`����Ă��܂��B

  #define WU_CPU_IS_INTEL 0x000000010
  #define WU_CPU_IS_AMD 0x000000020
  #define WU_CPU_IS_IDT 0x000000030
  #define WU_CPU_IS_CYRIX 0x000000040
  #define WU_CPU_IS_NEXGEN 0x000000050
  #define WU_CPU_IS_RISE 0x000000060
  #define WU_CPU_IS_UMC 0x000000070
  #define WU_CPU_IS_TRANSMETA 0x000000080
  #define WU_CPU_IS_UNKNOWN 0x000000000

  �@���Ƃ��΁A(�߂�l & WU_CPU_VENDOR_MASK) �� WU_CPU_IS_INTEL �Ȃ�΁A����
  CPU �� Intel ���ł��B


�� wu_SetCPUType ----------------------------------------------------------

�@wu_SetCPUType �́Awuvorbis.dll ���g�p���� CPU �@�\���w�肵�܂��B���̊֐�
���Ăяo���Ȃ��ꍇ�� SSE �Ȃǂ� CPU �g���@�\���g��Ȃ�����ɂȂ�܂��B
�@�ʏ�́Awu_DetectCPU �̖߂�l�����̂܂܂��̊֐��̈����Ɏw�肷�邱�ƂŁA
CPU �ɉ������@�\�������I�Ɏg�p���邱�Ƃ��o���܂��B
�@���̊֐��́A�ʏ�AWuVorbisInit �� DLL ��ǂݍ��񂾒���ɌĂяo���܂��B

�E��`

  void wu_SetCPUType(unsigned __int32 type);

�E����
  type
    �@CPU �@�\�t���O���w�肵�܂��Bwu_setCPUType �́A�����Ɏw�肳�ꂽ�l�̂�
    ���A�ȉ��̃r�b�g�����Q�Ƃ��܂���B���̃r�b�g�͖������܂��B

    #define WU_CPU_HAS_MMX 0x000020000
    #define WU_CPU_HAS_3DN 0x000040000
    #define WU_CPU_HAS_SSE 0x000080000

    �������A�����I�ɂ͑��̃r�b�g���Q�Ƃ���悤�ɂȂ�\��������ׁA���ʂȗ�
    �R���Ȃ�����Awu_DetectCPU �̖߂�l�����̂܂܎w�肵�Ă��������B


�� wu_ScaleOutput ----------------------------------------------------------

�@wu_ScaleOutput �́A�o�͂̑��������w�肵�܂��B���̊֐��Ŏw�肵������������
�Z���ꂽ�o�͂𓾂邱�Ƃ��o���܂��B
�@���̊֐��͂Q��ȏ�Ăяo�����ꍇ�A�Q��ڈȍ~�̑������̎w��́A�ȑO�Ɏw��
�����������Ɋ|�����킳���`�Őݒ肳��܂��B���Ƃ��΁A

    wu_ScaleOutput(0.5f);
    wu_ScaleOutput(4.0f);

�@�Ƃ���ƁA�ŏI�I�ȑ������� 0.5 * 4.0 �܂� 2.0 �ɂȂ�܂��B
�@���̊֐���������Ăяo���ƁA�Ăяo�����ƂɌ덷���~�ς����\��������A
�D�܂�������܂���B

�@wu_ov_read �� float �`���� PCM �𓾂�Ƃ��A���̂܂܂ł� ����� 32768.0
�ŁA������ -32768.0 �� PCM �������܂��Bfloat �� PCM �Ƃ��Ĉ�ʓI�Ȓl�̔�
�͂ł��� -1.0 �` 0 �` 1.0 �͈̔͂ɂ��邽�߂ɂ́A���̊֐��̈�����
1.0 / 32768.0 ���w�肷��K�v������܂��B�ȉ��̂悤�ɂȂ�܂��B

    wu_ScaleOutput((float)(1.0 / 32768.0));

�E��`

  void  wu_ScaleOutput(float scale);

�E����
  scale
    �@�o�͂̑�������{���Ŏw�肵�܂��BdB ��p�[�Z���g�w��ł͂���܂���B
    1.0f ���w�肷��Ƒ������͈ȑO�̂܂܂ƂȂ�܂��B


�� �g���� -----------------------------------------------------------------

�@�܂��Alib/wuvorbisfile.c ���v���W�F�N�g�ɒǉ����Ă��������B
�@include/wuvorbisfile.h �ɂ̓C���N���[�h �p�X��ʂ��Ă��������B

�@DLL ���g�����߂ɂ� WuVorbisInit ���Ăяo���܂��B�ʏ�A���̒���
wu_SetCPUType(wu_DetectCPU()); ���Ăяo���܂��B

�@wuvorbis.dll �͒ʏ�A�\�t�g�E�F�A�̃��C���̎��s�\�t�@�C���Ɠ����ꏊ�ɔz
�u���܂��BWindows �� System32 �� System �f�B���N�g���ɃR�s�[����K�v�͂����
����B

�@�g�����̃T���v���� test �f�B���N�g���ɂ���܂��B���̃T���v���́AOggVorbis
�t�@�C�����f�R�[�h���A raw PCM �t�@�C���ɏo�͂��镨�ł��B


�� ���� -------------------------------------------------------------------

�@�X���b�h�Z�[�t�ł����A���� wu_OggVorbis_File �I�u�W�F�N�g�ɑ΂��� API ���
�񂵂ČĂяo���ėǂ��Ƃ����킯�ł͂���܂���B���� wu_OggVorbis_File �I�u
�W�F�N�g�ɑ΂��ẮAAPI �̌Ăяo���͑O�� API �̌Ăяo�����I����Ă���ɂ���
�K�v������܂��B�قȂ�I�u�W�F�N�g�ɑ΂��� API ����񂵂ČĂяo�����Ƃ͏o��
�܂��B

�@�Ȃ�ׂ��Awuvorbis.dll �͂���Ɠ������� wuvorbisfile ���g�p���Ă��������B
�����Ȃ��ƁA�o�[�W�����Ԃ̕s��������������\��������܂��B

�@wu_ov_read �ɂ�� float �`���� PCM �̓N���b�s���O���s���܂���B�܂�A�l
�͈̔͂� -1.0 �` 0 �` 1.0�Ɋ��҂����Ƃ��Ă��A�g�`�̃s�[�N�����͈̔͂��͂ݏo
��\��������܂��B


�@�ŐV�� wuvorbis.dll �͋g���g���Q SDK ��

/kirikiri2/plugin/

�@wuvorbisfile �͋g���g���Q�̃\�[�X�z�z�t�@�C����

/environ/win32/wuvorbis/decapi/

�@�ɂ���܂��B


�� ���C�Z���X -------------------------------------------------------------

�@wuvorbis.dll (�f�R�[�_ DLL) �� wuvorbisfile (�X�^�u���C�u����) �́A
wuvorbis.dll �̂��ƂƂȂ��Ă��� libvorbis / libogg �̃��C�Z���X�Ɠ������A
BSD ���C�N�ȃ��C�Z���X�Ƃ������Ƃɂ��܂��B�܂�A���ۏ؁E���ӔC�ł��B
�@wuvorbis.dll ���g���\�t�g�E�F�A���L���z�z�ł��ꖳ���z�z�ł���A�܂��A�I�[
�v���\�[�X�ł���N���[�Y�h�\�[�X�ł���A�����Ŏg�p�ł��܂��B
wuvorbis.dll ���g���\�t�g�E�F�A�̃\�[�X�����J����K�v�͂���܂���B
�@�ǂ̏ꍇ���A�ȉ��̃��C�Z���X�\�L���悭���ǂ݂ɂȂ��Ă��g�p���������B

�@�g�p�񍐂̋`���͂���܂��񂪁A�񍐂��Ă��������Ă����\�ł��B

�@libvorbis / libogg �̃��C�Z���X�́A�ȉ��̃��C�Z���X�\�L���h�L�������g����
�ǔz�z���ɋL�q���邱�Ƃ�v�����Ă��܂��B�������Awuvobis.dll �ɂ͂���
�o�[�W�������ɂ��̕\�L���܂߂Ă���̂ŁA���߂ăh�L�������g�Ȃǂł��̕\�L
���L�q����K�v�͂Ȃ����Ǝv���܂��B�S�z�ȕ��͕ʓr�L�q���Ă��������B


Copyright (c) 2002-2004, Xiph.org Foundation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of the Xiph.org Foundation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


�@�ǋL�ŁAwuvorbis.dll ���邢�� SSE/3DNow! �p�b�`�̒��쌠���ł����A

SSE & 3DNow! patch Copyright (c) 2002-2004, W.Dee

�@�̕\�L���h�L�������g���Ȃǂ̔z�z���ɋL�q���邱�Ƃ��D�܂����ł��B�������A
����� wuvorbis.dll �̃o�[�W�������Ɋ܂܂�Ă���̂ŁA�ʓr�A�h�L�������g
���Ȃǂł��̕\�L���L�q����K�v�͂���܂���B

�@wuvorbis.dll �̃\�[�X�� SSE/3DNow! �p�b�`�͋g���g���Q�̃\�[�X�z�z�t�@�C��
���Ɋ܂܂�Ă��܂��B


�� �X�V���� ---------------------------------------------------------------

�E2004/10/5

�@libvorbis 1.1.0 �x�[�X�Ƀo�[�W�����A�b�v�B���̃o�[�W������ wuvorbisfile
�́A�ȑO�� wuvorbis.dll �Ƃ͐���Ƀ����N�ł��܂���B
�@wuvorbis.dll �𓯍����܂����B�ȑO�̃o�[�W�����ɔ�ׁADLL �̃T�C�Y�� 27%
����A 10% �قǃf�R�[�h�̃X�s�[�h�����サ�Ă��܂��B

�E2003/07/5

�@Query_sizeof_OggVorbis_File �̃O���[�o����`�����������p�ł��Ȃ������̂�
�@�C���B

�E2003/06/23

�@wu_OggVorbis_File �\���̂Ɍ���݊����p�̗]�T�����������B
�@wu_ScaleOutput �֐��ǉ��B


�� ��ҘA���� -------------------------------------------------------------

   W.Dee <dee@kikyou.info>
