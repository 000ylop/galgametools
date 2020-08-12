//---------------------------------------------------------------------------

#pragma hdrstop

#include <stdio.h>
#include "wuvorbisfile.h"



//---------------------------------------------------------------------------
// �e�R�[���o�b�N�֐��̏���
//---------------------------------------------------------------------------
static size_t _cdecl cb_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE*)datasource);
}
//---------------------------------------------------------------------------
static int _cdecl cb_seek_func(void *datasource, wu_ogg_int64_t offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}
//---------------------------------------------------------------------------
static int _cdecl cb_close_func(void *datasource)
{
	return fclose((FILE*)datasource);
}
//---------------------------------------------------------------------------
static long _cdecl cb_tell_func(void *datasource)
{
	return ftell((FILE*)datasource);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// main �֐�
//---------------------------------------------------------------------------
#pragma argsused
//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	// �����`�F�b�N
	if(argc < 3)
	{
		fprintf(stderr, "Usage: wu_decapi_test <input_vorbis_stream.ogg> <output_raw_pcm_file> [<path_to_wuvorbis.dll>]\n");
	}

	const char *wuvorbis_dll = argc >= 4 ? argv[3] : NULL;

	// WuVorbis ��������
	// WuVorbisInit �� wuvorbis.dll �̃t�@�C�����ŁANULL �Ȃ��
	// �f�t�H���g�̌����p�X��̃t�@�C�����g���� (LoadLibrary("wuborbis.dll") )
	// WuvVorbisInit ���Ăяo���Ȃ��ƁAwu_ �����ɂ��֐��̌Ăяo���͂��ׂ�
	// ���s����̂Œ��ӁB
	if(WuVorbisInit(wuvorbis_dll))
	{
		fprintf(stderr, "Cannot load or incorrect wuvorbis.dll\n");
		return 3;
	}

	// CPU �`�F�b�N�� CPU �g�p�@�\�̐ݒ�
	// wu_DetectCPU �� CPU �@�\���擾���邽�߂Ɏg���B
	// �ʏ�Awu_DetectCPU �̖߂�l�����̂܂� wu_SetCPUType �ɓn����
	// �g���Bwu_SetCPUType ���g��Ȃ��ƁASSE �� 3DNow! �Ȃǂ� CPU �g���@�\��
	// �܂������g���Ȃ��̂Œ��ӂ���B
	// wu_SetCPUType ���݂�r�b�g�́Awu_DetectCPU ���Ԃ��l�̂���
	// #define WU_CPU_HAS_MMX 0x000020000
	// #define WU_CPU_HAS_3DN 0x000040000
	// #define WU_CPU_HAS_SSE 0x000080000
	// �̂R�����ł���B
	unsigned __int32 cputype = wu_DetectCPU();
	wu_SetCPUType(cputype);


	// �t�@�C�����J��
	FILE *file = fopen(argv[1], "rb");
	if(!file)
	{
		// �t�@�C�����J���Ȃ�
		WuVorbisUninit();
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return 3;
	}

	FILE *outfile = fopen(argv[2], "wb");
	if(!outfile)
	{
		// �t�@�C�����J���Ȃ�
		fclose(file);
		WuVorbisUninit();
		fprintf(stderr, "Cannot open %s\n", argv[2]);
		return 3;
	}

	// �R�[���o�b�N�֐��̃Z�b�g�A�b�v
	wu_ov_callbacks callbacks = {cb_read_func, cb_seek_func, cb_close_func, cb_tell_func};

	// wu_og_open_callbacks ���g�p���ĊJ��
	wu_OggVorbis_File ovfile;

	if(wu_ov_open_callbacks((void*)file, &ovfile, NULL, 0, callbacks) < 0)
	{
		// �t�@�C���𐳏�ɊJ���Ȃ�
		fclose(file);
		fclose(outfile);
		WuVorbisUninit();
		fprintf(stderr, "Cannot open %s (in ov_open_callbacks)\n", argv[1]);
		return 3;
	}

	// ����\��
	wu_vorbis_info *info = wu_ov_info(&ovfile, -1);

	if(!info)
	{
		wu_ov_clear(&ovfile);
		fclose(outfile);
		WuVorbisUninit();
		fprintf(stderr, "ov_info failed\n", argv[1]);
		return 3;
	}

	fprintf(stderr,
		"version   : %d\n"
		"channels  : %d\n"
		"rate      : %d\n"
		"bitrate_upper   : %d\n"
		"bitrate_nominal : %d\n"
		"bitrate_lower   : %d\n"
		"bitrate_window  : %d\n",
		info->version, info->channels, info->rate, info->bitrate_upper,
		info->bitrate_nominal, info->bitrate_lower, info->bitrate_window);

	// �f�R�[�h���Ă݂�
	// ���̃e�X�g�v���O�������o�͂���f�R�[�h���ʂ� raw pcm �Ȃ̂Œ���
	int section = -1;
	while(true)
	{
		char buf[1024];
		// wu_ov_read �� bigendianp �� 0�A
		// word �� 2(16bit signed integer) �� 4 (32bit float) �A
		// signed �� 1 �̕K�v������
		int res = wu_ov_read(&ovfile, buf, 1024, 0, 2, 1, &section);
		// wu_ov_read �͗v�����ꂽ�T�C�Y�𖞂������ɋA��ꍇ������̂Œ��ӁB
		// 0 ���߂�΃f�R�[�h�I���A���̐����߂��
		// �f�R�[�h����(�߂�l�͏������܂ꂽ�o�C�g��)�A
		// ���̐����߂����ꍇ�̓p�P�b�g�s���Ńf�R�[�h�ł��Ȃ����B
		//  (�l�b�g���[�N����̃X�g���[�~���O�Ƃ��łȂ��Ă����̏󋵂͔�������)
		// ���̐����߂����ꍇ�́A�ǂݎ���悤�ɂȂ�܂�
		// ���̂܂� wu_ov_read ���J��Ԃ��΂悢

		if(res == 0) break; // �f�R�[�h�I��
		if(res > 0)	fwrite(buf, 1, res, outfile);
	}

	// wu_ov_clear �� ovfile �\���̂��N���A
	wu_ov_clear(&ovfile);
	// (���̓t�@�C���� ov_clear �� cb_close_func ���Ăяo���Ď����I�ɕ���̂ł����ł͕��Ȃ�)

	// �o�̓t�@�C�������
	fclose(outfile);

	// WuVorbis ���I��
	// �ʏ� Windows �̓A�v���P�[�V�����I�����Ɏ����I�� DLL ���������̂�
	// ����͕K�v�Ȃ������B�B�B
	WuVorbisUninit();

	return 0;
}
//---------------------------------------------------------------------------
