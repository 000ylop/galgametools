

#include <gls.h>
#include <linkgls.h>
#include <stdio.h>



//////////////////////////////////////////////////////////////////////////////
// �A�[�J�C�o�N���X
//////////////////////////////////////////////////////////////////////////////

class	ENoaArchive	: public	ERISAArchive
{
public:
	// �\�z�֐�
	ENoaArchive( bool fTime = false ) ;
	// ���Ŋ֐�
	virtual ~ENoaArchive( void ) ;
	// �N���X���
	DECLARE_CLASS_INFO( ENoaArchive, ERISAArchive )

protected:
	bool	m_fTime ;
	EString	m_strFileName ;

public:
	// �t�@�C�����J��
	int DescendFile
		( const char * pszFileName, const char * pszPassword = NULL ) ;

protected:
	// �W�J�E�����̐i�s�󋵂�ʒm����
	virtual ESLError OnProcessFileData( DWORD dwCurrent, DWORD dwTotal ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// �A�v���P�[�V�����N���X
//////////////////////////////////////////////////////////////////////////////

class	ENoaApp
{
public:
	bool	m_fNologo ;			// ���o����\�����邩
	bool	m_fHelp ;			// ������\������
	bool	m_fPackaging ;		// �A�[�J�C�u������
	bool	m_fAutoRaw ;		// ���k���������Ǝ����I�ɖ����k�ɂ���
	bool	m_fGenPassword ;	// �p�X���[�h�𐶐�����
	bool	m_fSubDirectory ;	// �T�u�f�B���N�g�����A�[�J�C�u�����邩�H
	bool	m_fGenListFile ;	// ���X�g�t�@�C���𐶐�����
	bool	m_fTime ;			// ���Ԃ��v������
	int		m_fPackOption ;		// �A�[�J�C�u���̃I�v�V����
	EString	m_strPassword ;		// �p�X���[�h
	EString	m_strListFile ;		// ���X�g�t�@�C����
	EString	m_strSrcFile ;		// ���̓t�@�C����
	EString	m_strDstFile ;		// �o�̓t�@�C����
	DWORD	m_dwRndSeed ;		// �����̎�

public:
	// �\�z�֐�
	ENoaApp( void ) ;
	// ���Ŋ֐�
	~ENoaApp( void ) ;
	// �\�����
	const char * SetArgument( int argc, char * argv[] ) ;
	// �A�[�J�C�u��
	int PackFiles( void ) ;
	// �W�J
	int DepackFiles( void ) ;

protected:
	// �t�@�C����񋓂��A�t�@�C�����X�g�ɒǉ�
	void EnumFileList
		( ERISAArchiveList & arclist, const char * pszFiles ) ;
	// �p�X���[�h�𐶐�����
	EString GeneratePassword( const char * pszFileName ) ;
	// �����𐶐�����
	int MakeRandom( int nMax ) ;

	// �f�B���N�g���������o��
	int PackDirectory
		( ERISAArchive & arcfile, ERISAArchiveList & arclist ) ;
	// �t�@�C�����X�g���珑���o���p�f�B���N�g�����𐶐�����
	void CreateDirectoryInfo
		( ERISAArchive::EDirectory & dirFiles,
			ERISAArchiveList::EDirectory * pdir ) ;

	// �f�B���N�g���ɏ����o��
	int DepackDirectory
		( ERISAArchive & arcfile,
			ERISAArchiveList * arclist, const EString & strBaseDir ) ;
	// �t�@�C���G���g��������
	ERISAArchiveList::EFileEntry * SearchFile
		( ERISAArchiveList::EDirectory * pdir, const char * pszFileName ) ;

} ;

