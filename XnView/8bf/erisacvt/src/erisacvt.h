
/*****************************************************************************
                          ERISA �摜�R���o�[�^�[
 *****************************************************************************/


#include <gls.h>
#include <linkgls.h>

#include <psdfile.h>
#include <linkpsdf.h>

#include <stdio.h>

#include "resource.h"


//////////////////////////////////////////////////////////////////////////////
// XML �p�[�T�[
//////////////////////////////////////////////////////////////////////////////

class	XMLParser	: public	EDescription
{
public:
	int	m_nErrorCount ;

	// �\�z�֐�
	XMLParser( void ) : m_nErrorCount(0) { }
	// ���^�O�𐶐�
	virtual EDescription * CreateDescription( void ) ;
	// �G���[�o��
	virtual ESLError OutputError( const char * pszErrMsg ) ;
	// �x���o��
	virtual ESLError OutputWarning( const char * pszErrMsg ) ;
} ;


//////////////////////////////////////////////////////////////////////////////
// ���t�X�N���v�g�@�R���p�C��
//////////////////////////////////////////////////////////////////////////////

class	ECSXCompiler	: public	ECSCompiler
{
public:
	EString				m_strBaseDir ;
	EString				m_strExeDir ;
	EWideString			m_wstrLine ;

public:
	// �\�z�֐�
	ECSXCompiler( void ) ;
	// �x�[�X�f�B���N�g����ݒ肷��
	void SetBaseDirectory( const char * pszBaseDir ) ;
	// �P�s�R���p�C������
	virtual ESLError CompileScriptLine
		( ECSSourceStream & cssLine,
			int nLineNum, const char * pszFilePath = NULL ) ;
	// �w��̃p�X�̃t�@�C�����J��
	virtual ESLFileObject * OpenScriptFile( const char * pszFilePath ) ;
	// �G���[���o�͂���
	virtual ESLError OutputError
		( const char * pszErrMsg,
			const char * pszFilePath = NULL, int nLineNum = 0 ) ;
	// �x�����o�͂���
	virtual ESLError OutputWarning
		( const char * pszErrMsg,
			const char * pszFilePath = NULL, int nLineNum = 0 ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// �摜�o�b�t�@
//////////////////////////////////////////////////////////////////////////////

class	ECSImage	: public	ECSResource
{
public:
	// �\�z�֐�
	ECSImage( void ) ;
	// ���Ŋ֐�
	virtual ~ECSImage( void ) ;
	// �N���X���
	DECLARE_CLASS_INFO( ECSImage, ECSResource )

public:
	EGLMediaLoader	m_imgBuf ;			// �摜�f�[�^
	ECSInteger		m_intIndex ;		// �t���[���E���C���[�ԍ�
	ECSString		m_strName ;			// ���C���[��
	ECSString		m_strFileName ;		// �t�@�C����

public:
	// �I�u�W�F�N�g�̌^�����擾����
	virtual const wchar_t * GetTypeName( void ) const ;
	// �I�u�W�F�N�g�𕡐�
	virtual ECSObject * Duplicate( void ) ;
	// �I�u�W�F�N�g����
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;
	// �����o�ϐ��C���f�b�N�X�擾
	virtual ESLError GetVariableIndex( int & nIndex, ECSObject & obj ) ;
	// �����o�ϐ��擾
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// �����o�ϐ��ݒ�
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// �摜�R���e�L�X�g�o�b�t�@
//////////////////////////////////////////////////////////////////////////////

class	ECSImageContext	: public	ECSArray
{
public:
	// �\�z�֐�
	ECSImageContext( void ) ;
	// ���Ŋ֐�
	virtual ~ECSImageContext( void ) ;
	// �N���X���
	DECLARE_CLASS_INFO( ECSImageContext, ECSArray )

public:
	EGLMediaLoader	m_imgBuf ;			// �o�͗p�o�b�t�@
	ENumArray<UINT>	m_lstSequence ;		// �V�[�P���X���X�g
	ECSInteger		m_intDuration ;		// �A�j���[�V��������
	ECSStructure	m_sizeImage ;		// �摜�T�C�Y
	ECSStructure	m_ptHotSpot ;		// �z�b�g�X�|�b�g
	ECSString		m_strFileName ;		// �t�@�C����

	ECSStrTagArray	m_staFuncName ;

	enum	CutOperationFlag
	{
		cofKeepHotspot	= 0x01,
		cofRemoveAlpha	= 0x02,
		cofAutoAlpha	= 0x04,
	} ;

	typedef	ESLError (ECSImageContext::*PFUNC_CALL)
		( ECSContext & context, EObjArray<ECSObject> & lstArg ) ;
	static const wchar_t *	pwszFuncName[7] ;
	static const PFUNC_CALL	m_pfnCallFunc[6] ;

public:
	// �z�񒷎擾
	int GetLength( void ) const
		{
			return	m_varArray.GetSize() ;
		}
	// �摜�f�[�^�擾
	ECSImage * GetImageAt( int nIndex ) const
		{
			return	ESLTypeCast<ECSImage>( m_varArray.GetAt( nIndex ) ) ;
		}
	// �摜�R���e�L�X�g����
	void DeleteContext( void ) ;

public:
	// �摜�̓ǂݍ���
	ESLError LoadImageFile
		( const wchar_t * pwszFileName, int fBlendMode, int fMaskMode ) ;
	// �摜�̐؂�o��
	int CutImage
		( int nAlign, int nMargin,
			int nThreshold, int nFlags, EGL_IMAGE_RECT & irClip ) ;
	// �摜�̔z��
	int ArrangeImage( bool fWayVert, bool fPutRight, int nAlign ) ;
	// �A�j���[�V�����쐬
	int MakeAnimation( const wchar_t * pwszSeq, int nDuration ) ;
	// �}�[�W����
	void MergeContext
		( ECSImageContext & imgctx,
			int iFirst = 0, int nCount = -1,
			const wchar_t * pwszName = NULL ) ;

public:
	// �I�u�W�F�N�g�̌^�����擾����
	virtual const wchar_t * GetTypeName( void ) const ;
	// �I�u�W�F�N�g�𕡐�
	virtual ECSObject * Duplicate( void ) ;
	// �I�u�W�F�N�g����
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;
	// �����o�ϐ��C���f�b�N�X�擾
	virtual ESLError GetVariableIndex( int & nIndex, ECSObject & obj ) ;
	// �����o�ϐ��擾
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// �����o�ϐ��ݒ�
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;
	// �����o�֐��C���f�b�N�X�擾
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// �����o�֐��Ăяo��
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, EObjArray<ECSObject> & lstArg ) ;

public:
	// �����o�֐�
	ESLError Call_load
		( ECSContext & context, EObjArray<ECSObject> & lstArg ) ;
	ESLError Call_save
		( ECSContext & context, EObjArray<ECSObject> & lstArg ) ;
	ESLError Call_cut
		( ECSContext & context, EObjArray<ECSObject> & lstArg ) ;
	ESLError Call_arrange
		( ECSContext & context, EObjArray<ECSObject> & lstArg ) ;
	ESLError Call_animation
		( ECSContext & context, EObjArray<ECSObject> & lstArg ) ;
	ESLError Call_merge
		( ECSContext & context, EObjArray<ECSObject> & lstArg ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// �A�v���P�[�V�����N���X
//////////////////////////////////////////////////////////////////////////////

class	EConverterApp	: public	ECSContext
{
public:
	// �\�z�֐�
	EConverterApp( void ) ;
	// ���Ŋ֐�
	virtual ~EConverterApp( void ) ;
	// �N���X���
	DECLARE_CLASS_INFO( EConverterApp, ECSContext )

public:
	// ����
	enum	Options
	{
		optNologo	= 0x0001,
		optUsage	= 0x0002,
		optHelp		= 0x0004,
		optIndirect	= 0x0008,
		optInform	= 0x0010
	} ;
	DWORD			m_dwFlags ;
	EString			m_strErrMsg ;
	EString			m_strSrcFile ;
	EString			m_strDstFile ;
	EString			m_strReportFile ;
	EString			m_strOutputFile ;

	XMLParser		m_xmlCvt ;
	EDescription *	m_pxmlErisa ;

	int				m_nErrorCount ;
	XMLParser		m_xmlReport ;

	EStreamBuffer	m_bufOutput ;

	// �R���o�[�^�[�̊��ϐ�
	enum	FormatType
	{
		ftInvalid = -1,
		ftERI, ftERINA, ftERISA,
		ftBMP, ftTIFF, ftPNG, ftJPEG,
		ftMax
	} ;
	int				m_nFmtType ;			// �o�̓t�H�[�}�b�g
	int				m_nCmprMode ;			// ���k���[�h
	EWideString		m_wstrDstDir ;			// �o�̓f�B���N�g��
	EWideString		m_wstrSrcDir ;			// ���̓f�B���N�g��
	int				m_nPutAlign ;			// �摜��z�u����ۂ̃O���b�h
	int				m_nCutAlign ;			// �摜��؂�o���ۂ̃O���b�h
	int				m_nCutMargin ;			// �摜��؂蔲���ۂ̃}�[�W��
	int				m_nCutThreshold ;		// �؂蔲��臒l
	int				m_fKeepHotspot ;		// �z�b�g�X�|�b�g���ێ����邩�H
	int				m_fLayerBlend ;			// ���C���[���������邩�H
	int				m_fLayerMask ;			// �}�X�N�����̕��@

	static const wchar_t *	m_pwszFormatMime[ftMax] ;
	static const wchar_t *	m_pwszFormatExt[ftMax] ;

	// �t�@�C�����񋓃I�u�W�F�N�g
	class	EnumFileName
	{
	public:
		enum
		{
			typeNothing,
			typeWin32,
			typeSeqNum,
			typeScript
		}						m_type ;
		EConverterApp *			m_app ;
		HANDLE					m_hFind ;
		WIN32_FIND_DATA			m_wfd ;
		EString					m_strFileName ;
		EWideString				m_wstrBaseDir ;
		int						m_iFirst ;
		int						m_iEnd ;
		int						m_nStep ;
		int						m_iCurrent ;
		EObjArray<ECSObject>	m_lstArg ;
	public:
		// �\�z�֐�
		EnumFileName( EConverterApp * app ) ;
		EnumFileName
			( EConverterApp * app,
				const wchar_t * pwszBaseDir, const wchar_t * pwszFileName ) ;
		// ���Ŋ֐�
		~EnumFileName( void ) ;
		// �t�@�C�����ݒ�
		void SetEnumFiles
			( const wchar_t * pwszBaseDir, const wchar_t * pwszFileName ) ;
		// �t�@�C�����擾
		ESLError GetNextFileName
			( EWideString & wstrFileName, EString & strErrMsg ) ;
	} ;

	// �X�N���v�g��
	ECSExecutionImage	m_csxi ;			// ���s�C���[�W

public:
	// �g���^�I�u�W�F�N�g�𐶐�����
	virtual ECSObject * CreateExtendedObject
		( CSVariableType csvtType,
			const wchar_t * pwszType, DWORD * pdwFuncAddr = NULL ) ;

public:
	// �����̉��
	void ParseCmdLine( int argc, char * argv[] ) ;
	// ���o���E�G���[�E�����Ȃǂ̕\��
	int PrintMessage( void ) ;
	// �w�肳�ꂽ��\�[�X��W���o�͂֏o��
	void PrintResource( LPCTSTR lpName ) ;
	// ���s
	int Perform( void ) ;

public:
	// MIME ���� FormatType �񋓎q�ւ̕ϊ�
	static int FormatTypeFromMIME( const wchar_t * pwszMIME ) ;
	// �Ή�����t�H�[�}�b�g�̃f�t�H���g�̊g���q���擾
	static const wchar_t * GetDefaultFileExt( int fFmtType ) ;

public:
	// �摜�t�@�C������\������
	int InformFiles( void ) ;
	// �C���_�C���N�g�t�@�C����ǂݍ���
	int LoadIndirectFile( void ) ;
	// �N���������珈���葱�����쐬����
	int MakeProcess( void ) ;
	// ���������s����
	int BeginProcess( void ) ;

public:
	// ���ݒ���s��
	void SetEnvironment( EDescription * pdscEnv ) ;
	// �t�@�C���������s��
	void ProcessFiles( EDescription * pdscFile ) ;
	// �t�@�C������
	void ProcessFileTags
		( ECSImageContext & imgctx, EDescription * pdscFile ) ;
	// �摜��I��
	void ProcessSelectImage
		( ECSImageContext & imgctx, EDescription * pdscSelect ) ;
	// �t�@�C����ǂݍ���
	void ProcessImages
		( ECSImageContext & imgctx, EDescription * pdscImage ) ;
	// �A�ԃt�@�C���o��
	void WriteSequenceFiles
		( ECSImageContext & imgctx, const wchar_t * pwszDstFile ) ;
	// �o�̓t�@�C�����𐮌`����
	EWideString NormalizeDestinationFileName
		( const wchar_t * pwszDst, const wchar_t * pwszSrc ) ;

public:
	// �摜��ǂݍ���
	ESLError LoadImageFile
		( ECSImageContext & imgctx,
			const wchar_t * pwszSrc,
			const wchar_t * pwszHotSpot, int nIndex ) ;
	// �摜���t�@�C���ɏ����o��
	ESLError SaveImageContext
		( ECSImageContext & imgctx, const wchar_t * pwszDst, int nFmtType ) ;
	ESLError SaveImageFile
		( EGLMediaLoader & imgBuf, const wchar_t * pwszDst, int nFmtType ) ;

} ;

