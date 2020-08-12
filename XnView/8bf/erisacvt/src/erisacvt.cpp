
/*****************************************************************************
                          ERISA �摜�R���o�[�^�[
 *****************************************************************************/


#include "erisacvt.h"


//////////////////////////////////////////////////////////////////////////////
// �G���g���|�C���g
//////////////////////////////////////////////////////////////////////////////

int main( int argc, char * argv[] )
{
	int	result = 0 ;
	::glsInitializeLibrary( ) ;
	EGLMediaLoader::Initialize( ) ;
	ECotophaScript::Initialize( ) ;
	{
		EConverterApp	app ;
		app.ParseCmdLine( argc, argv ) ;
		result = app.PrintMessage( ) ;
		if ( result == 0 )
		{
			result = app.Perform( ) ;
		}
	}
	ECotophaScript::Release( ) ;
	EGLMediaLoader::Close( ) ;
	::glsCloseLibrary( ) ;
	return	result ;
}


//////////////////////////////////////////////////////////////////////////////
// XML �p�[�T�[
//////////////////////////////////////////////////////////////////////////////

// ���^�O�𐶐�
//////////////////////////////////////////////////////////////////////////////
EDescription * XMLParser::CreateDescription( void )
{
	return	new XMLParser ;
}

// �G���[�o��
//////////////////////////////////////////////////////////////////////////////
ESLError XMLParser::OutputError( const char * pszErrMsg )
{
	m_nErrorCount ++ ;
	printf( "XML �\���G���[�F%s\n", pszErrMsg ) ;
	return	EDescription::OutputError( pszErrMsg ) ;
}

// �x���o��
//////////////////////////////////////////////////////////////////////////////
ESLError XMLParser::OutputWarning( const char * pszErrMsg )
{
	printf( "XML �\���G���[�F%s\n", pszErrMsg ) ;
	return	EDescription::OutputWarning( pszErrMsg ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ���t�X�N���v�g�@�R���p�C��
//////////////////////////////////////////////////////////////////////////////

// �\�z�֐�
//////////////////////////////////////////////////////////////////////////////
ECSXCompiler::ECSXCompiler( void )
{
	HMODULE	hModule = ::GetModuleHandle( NULL ) ;
	EString	strModulePath ;
	::GetModuleFileName( hModule, strModulePath.GetBuffer(0x400), 0x400 ) ;
	strModulePath.ReleaseBuffer( ) ;
	m_strExeDir = strModulePath.GetFileDirectoryPart( ) ;
}

// �x�[�X�f�B���N�g����ݒ肷��
//////////////////////////////////////////////////////////////////////////////
void ECSXCompiler::SetBaseDirectory( const char * pszBaseDir )
{
	m_strBaseDir = pszBaseDir ;
}

// �P�s�R���p�C������
//////////////////////////////////////////////////////////////////////////////
ESLError ECSXCompiler::CompileScriptLine
	( ECSSourceStream & cssLine, int nLineNum, const char * pszFilePath )
{
	m_wstrLine = cssLine ;
	return	ECSCompiler::CompileScriptLine( cssLine, nLineNum, pszFilePath ) ;
}

// �w��̃p�X�̃t�@�C�����J��
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ECSXCompiler::OpenScriptFile( const char * pszFilePath )
{
	EString	strFilePath = pszFilePath ;
	ERawFile *	pfile = new ERawFile ;
	long int	nOpenFlags =
		ESLFileObject::modeRead | ESLFileObject::shareRead ;
	if ( pfile->Open( strFilePath, nOpenFlags ) )
	{
		if ( (strFilePath[0] == '\\') || (strFilePath.Find( ':' ) >= 0) )
		{
			delete	pfile ;
			return	NULL ;
		}
		if ( pfile->Open( m_strBaseDir + strFilePath, nOpenFlags ) )
		{
			if ( pfile->Open( m_strExeDir + strFilePath, nOpenFlags ) )
			{
				delete	pfile ;
				return	NULL ;
			}
		}
	}
	return	pfile ;
}

// �G���[���o�͂���
//////////////////////////////////////////////////////////////////////////////
ESLError ECSXCompiler::OutputError
	( const char * pszErrMsg, const char * pszFilePath, int nLineNum )
{
	if ( (pszFilePath != NULL) && (nLineNum > 0) )
	{
		printf( "%s (%d) : �G���[ : %s\n", pszFilePath, nLineNum, pszErrMsg ) ;
	}
	else
	{
		printf( "�ȉ��̍s : �G���[ : %s\n", pszErrMsg ) ;
		printf( EString(m_wstrLine) + "\n" ) ;
	}
	return	ECSCompiler::OutputError( pszErrMsg, pszFilePath, nLineNum ) ;
}

// �x�����o�͂���
//////////////////////////////////////////////////////////////////////////////
ESLError ECSXCompiler::OutputWarning
	( const char * pszErrMsg, const char * pszFilePath, int nLineNum )
{
	if ( (pszFilePath != NULL) && (nLineNum > 0) )
	{
		printf( "%s (%d) : �x�� : %s\n", pszFilePath, nLineNum, pszErrMsg ) ;
	}
	else
	{
		printf( "�ȉ��̍s : �x�� : %s\n", pszErrMsg ) ;
		printf( EString(m_wstrLine) + "\n" ) ;
	}
	return	ECSCompiler::OutputWarning( pszErrMsg, pszFilePath, nLineNum ) ;
}


//////////////////////////////////////////////////////////////////////////////
// �A�v���P�[�V�����N���X
//////////////////////////////////////////////////////////////////////////////

// �N���X���
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EConverterApp, ECSContext )

const wchar_t *	EConverterApp::m_pwszFormatMime[EConverterApp::ftMax] =
{
	L"image/x-eri", L"image/x-erina", L"image/x-erisa",
	L"image/bmp", L"image/tiff", L"image/png", L"image/jpeg"
} ;
const wchar_t *	EConverterApp::m_pwszFormatExt[EConverterApp::ftMax] =
{
	L"eri", L"eri", L"eri", L"bmp", L"tif", L"png", L"jpg"
} ;

// �\�z�֐�
//////////////////////////////////////////////////////////////////////////////
EConverterApp::EConverterApp( void )
{
	m_dwFlags = 0 ;
	//
	m_nFmtType = ftERI ;
	m_nCmprMode = 0 ;
	m_nPutAlign = 1 ;
	m_nCutAlign = 0 ;
	m_nCutMargin = 0 ;
	m_nCutThreshold = 0 ;
	m_fKeepHotspot = 1 ;
	m_fLayerBlend = 0 ;
	m_fLayerMask = 0 ;
}

// ���Ŋ֐�
//////////////////////////////////////////////////////////////////////////////
EConverterApp::~EConverterApp( void )
{
}

// EConverterApp::EnumFileName �\�z
//////////////////////////////////////////////////////////////////////////////
EConverterApp::EnumFileName::EnumFileName( EConverterApp * app )
	: m_app( app ), m_type( typeNothing ), m_hFind( INVALID_HANDLE_VALUE )
{
}

EConverterApp::EnumFileName::EnumFileName
	( EConverterApp * app,
		const wchar_t * pwszBaseDir, const wchar_t * pwszFileName )
	: m_app( app ), m_hFind( INVALID_HANDLE_VALUE )
{
	SetEnumFiles( pwszBaseDir, pwszFileName ) ;
}

// ���Ŋ֐�
//////////////////////////////////////////////////////////////////////////////
EConverterApp::EnumFileName::~EnumFileName( void )
{
	if ( m_hFind != INVALID_HANDLE_VALUE )
	{
		::FindClose( m_hFind ) ;
	}
}

// �t�@�C�����ݒ�
//////////////////////////////////////////////////////////////////////////////
void EConverterApp::EnumFileName::SetEnumFiles
	( const wchar_t * pwszBaseDir, const wchar_t * pwszFileName )
{
	EStreamWideString		swsFileName = pwszFileName ;
	EStreamWideString		swsUsage =
					L"(%s)\\: (<0-9>*) [\\- (<0-9>*)] [\\, (%n)] \\" ;
	EObjArray<EWideString>	lstParam ;
	EString					strErrMsg ;
	if ( swsFileName[0] == L'$' )
	{
		m_type = typeScript ;
		m_strFileName = swsFileName.Middle(1) ;
		m_iCurrent = 0 ;
	}
	else if ( !swsFileName.IsMatchUsage( swsUsage, strErrMsg, &lstParam ) )
	{
		m_type = typeSeqNum ;
		m_strFileName = lstParam[0] ;
		m_iFirst = lstParam[1].AsInteger( ) ;
		if ( lstParam[2].IsEmpty() )
		{
			m_iEnd = -1 ;
		}
		else
		{
			m_iEnd = lstParam[2].AsInteger( ) ;
		}
		if ( lstParam[3].IsEmpty() )
		{
			m_nStep = 1 ;
			if ( (m_iEnd >= 0) && (m_iEnd < m_iFirst) )
			{
				m_nStep = -1 ;
			}
		}
		else
		{
			m_nStep = lstParam[3].AsInteger( ) ;
		}
		m_iCurrent = m_iFirst ;
	}
	else
	{
		m_type = typeWin32 ;
		m_iCurrent = 0 ;
		m_strFileName = swsFileName ;
	}
	//
	m_wstrBaseDir = pwszBaseDir ;
	if ( (m_wstrBaseDir.GetLength() > 1)
		&& (m_wstrBaseDir.Right(1) != L"\\") )
	{
		m_wstrBaseDir += L'\\' ;
	}
	if ( m_type != typeScript )
	{
		if ( (m_strFileName.GetAt(0) != '\\')
			&& (m_strFileName.Find( ':' ) < 0) )
		{
			EString	strFileName = m_wstrBaseDir ;
			strFileName += m_strFileName ;
			m_strFileName = strFileName ;
		}
	}
}

// �t�@�C�����擾
//////////////////////////////////////////////////////////////////////////////
ESLError EConverterApp::EnumFileName::GetNextFileName
	( EWideString & wstrFileName, EString & strErrMsg )
{
	strErrMsg.FreeString( ) ;
	//
	if ( m_type == typeWin32 )
	{
		//
		// �t�@�C�����E���C���h�J�[�h
		//
		for ( ; ; )
		{
			if ( (m_iCurrent ++) == 0 )
			{
				m_hFind = ::FindFirstFile( m_strFileName, &m_wfd ) ;
				if ( m_hFind != INVALID_HANDLE_VALUE )
				{
					if ( !(m_wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
					{
						wstrFileName = m_strFileName.GetFileDirectoryPart() ;
						wstrFileName += EWideString( m_wfd.cFileName ) ;
						return	eslErrSuccess ;
					}
				}
				else
				{
					strErrMsg = m_strFileName + " �t�@�C����������܂���B" ;
					break ;
				}
			}
			else if ( m_hFind != INVALID_HANDLE_VALUE )
			{
				if ( ::FindNextFile( m_hFind, &m_wfd ) )
				{
					if ( !(m_wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
					{
						wstrFileName = m_strFileName.GetFileDirectoryPart() ;
						wstrFileName += EWideString( m_wfd.cFileName ) ;
						return	eslErrSuccess ;
					}
				}
				else
				{
					break ;
				}
			}
		}
	}
	else if ( m_type == typeSeqNum )
	{
		//
		// �A�ԃt�@�C��
		//
		if ( (m_iCurrent * m_nStep <= m_iEnd * m_nStep) || (m_iEnd == -1) )
		{
			int			nNum = m_iCurrent ;
			wstrFileName = m_strFileName ;
			//
			for ( int i = wstrFileName.GetLength() - 1; i >= 0; i -- )
			{
				if ( wstrFileName[i] == L'%' )
				{
					wstrFileName.SetAt( i, (wchar_t) (L'0' + (nNum % 10)) ) ;
					nNum /= 10 ;
				}
			}
			//
			m_iCurrent += m_nStep ;
			return	eslErrSuccess ;
		}
	}
	else if ( m_type == typeScript )
	{
		//
		// �X�N���v�g
		//
		DWORD *	pdwFuncAddr =
			m_app->m_csxi.GetFunctionAddress( EWideString(m_strFileName) ) ;
		if ( pdwFuncAddr != NULL )
		{
			ESLError	err = m_app->CallFunction( *pdwFuncAddr, m_lstArg ) ;
			if ( err )
			{
				strErrMsg = GetESLErrorMsg( err ) ;
			}
			else
			{
				ECSString *	pstrResult =
					ESLTypeCast<ECSString>( m_app->GetReturnValue() ) ;
				if ( pstrResult != NULL )
				{
					if ( pstrResult->m_varStr.IsEmpty() )
					{
						return	eslErrGeneral ;
					}
					wstrFileName = L"" ;
					if ( (pstrResult->m_varStr.GetAt(0) != L'\\')
						&& (pstrResult->m_varStr.Find( L':' ) < 0) )
					{
						wstrFileName = m_wstrBaseDir ;
					}
					wstrFileName += pstrResult->m_varStr ;
					return	eslErrSuccess ;
				}
				strErrMsg = m_strFileName
					+ "�֐��̕Ԃ�l�� String �ł͂���܂���B" ;
			}
		}
		else
		{
			strErrMsg = m_strFileName + " �֐���������܂���B" ;
		}
	}
	//
	return	eslErrGeneral ;
}

// �����̉��
//////////////////////////////////////////////////////////////////////////////
void EConverterApp::ParseCmdLine( int argc, char * argv[] )
{
	int	i ;
	for ( i = 1; i < argc; i ++ )
	{
		if ( argv[i][0] != '/' )
		{
			break ;
		}
		EString	strArg = argv[i] + 1 ;
		if ( strArg == "l" )
		{
			if ( i + 1 >= argc )
			{
				m_strErrMsg = "�ϊ������t�@�C�����w�肳��Ă��܂���B\n" ;
				return ;
			}
			m_strSrcFile = argv[++ i] ;
			m_dwFlags |= optIndirect ;
		}
		else if ( strArg == "r" )
		{
			if ( i + 1 >= argc )
			{
				m_strErrMsg = "���|�[�g�t�@�C�����w�肳��Ă��܂���B\n" ;
				return ;
			}
			m_strReportFile = argv[++ i] ;
		}
		else if ( strArg == "o" )
		{
			if ( i + 1 >= argc )
			{
				m_strErrMsg = "�A�E�g�v�b�g�t�@�C�����w�肳��Ă��܂���B\n" ;
				return ;
			}
			m_strOutputFile = argv[++ i] ;
		}
		else if ( strArg == "i" )
		{
			m_dwFlags |= optInform ;
		}
		else if ( strArg == "nologo" )
		{
			m_dwFlags |= optNologo ;
		}
		else if ( strArg == "?" )
		{
			m_dwFlags |= optUsage ;
		}
		else if ( strArg == "help" )
		{
			m_dwFlags |= optHelp ;
		}
		else if ( strArg == "clip" )
		{
			m_nCutAlign = 1 ;
		}
		else if ( strArg == "eri" )
		{
			m_nFmtType = ftERI ;
		}
		else if ( strArg == "erina" )
		{
			m_nFmtType = ftERINA ;
		}
		else if ( strArg == "erisa" )
		{
			m_nFmtType = ftERISA ;
		}
		else if ( strArg.Left(2) == "cp" )
		{
			bool	fError ;
			m_nCmprMode = strArg.Middle(2).AsInteger( false, &fError ) ;
			if ( (m_nCmprMode > 3) || fError )
			{
				m_nCmprMode = 0 ;
				m_strErrMsg = argv[i] ;
				m_strErrMsg += " �͕s���Ȉ��k���[�h�̎w��ł��B\n" ;
				return ;
			}
		}
		else if ( strArg.Left(5) == "mime:" )
		{
			m_nFmtType =
				FormatTypeFromMIME( EWideString( strArg.Middle(5) ) ) ;
			if ( m_nFmtType == ftInvalid )
			{
				m_strErrMsg = strArg.Middle(5) +
					" �͑Ή����Ă��Ȃ��o�̓t�H�[�}�b�g�ł��B\n" ;
			}
		}
		else
		{
			m_strErrMsg = argv[i] ;
			m_strErrMsg += " �͒�`����Ă��Ȃ��I�v�V�����ł��B\n" ;
			m_strErrMsg += "/? �I�v�V�����ŏ������m�F���Ă��������B\n" ;
			return ;
		}
	}
	if ( i >= argc )
	{
		if ( m_strSrcFile.IsEmpty() && !(m_dwFlags & (optUsage | optHelp)) )
		{
			m_strErrMsg = "���̓t�@�C�����w�肳��Ă��܂���B\n" ;
			m_strErrMsg += "/? �I�v�V�����ŏ������m�F���Ă��������B\n" ;
		}
		return ;
	}
	if ( (m_dwFlags & optIndirect) || !m_strSrcFile.IsEmpty() )
	{
		m_strErrMsg = "�����ɖ���������܂��B\n" ;
		return ;
	}
	m_strSrcFile = argv[i] ;
	m_fLayerBlend = 1 ;
	//
	if ( i + 1 < argc )
	{
		m_strDstFile = argv[i + 1] ;
	}
	//
	return ;
}

// ���o���E�G���[�E�����Ȃǂ̕\��
//////////////////////////////////////////////////////////////////////////////
int EConverterApp::PrintMessage( void )
{
	static const char	szTitle[] =
		"ERISA image converter version 1.05\n"
		"Copyright (C) 2004-2005 Leshade Entis. All rights reserved.\n\n" ;
	//
	if ( !(m_dwFlags & optNologo) )
	{
		printf( szTitle ) ;
	}
	if ( !m_strErrMsg.IsEmpty() )
	{
		printf( m_strErrMsg ) ;
		return	1 ;
	}
	if ( m_dwFlags & optUsage )
	{
		PrintResource( MAKEINTRESOURCE(IDR_USAGE) ) ;
		return	0 ;
	}
	if ( m_dwFlags & optHelp )
	{
		PrintResource( MAKEINTRESOURCE(IDR_HELP) ) ;
		return	0 ;
	}
	return	0 ;
}

// �w�肳�ꂽ��\�[�X��W���o�͂֏o��
//////////////////////////////////////////////////////////////////////////////
void EConverterApp::PrintResource( LPCTSTR lpName )
{
	HMODULE	hModule = ::GetModuleHandle( NULL ) ;
	HRSRC	hRsrc = ::FindResource( hModule, lpName, "RT_RCDATA" ) ;
	HGLOBAL	hData = ::LoadResource( hModule, hRsrc ) ;
	void *	pData = ::LockResource( hData ) ;
	//
	if ( pData != NULL )
	{
		DWORD	dwWrittenBytes ;
		DWORD	dwSize = ::SizeofResource( hModule, hRsrc ) ;
		::WriteFile
			( ::GetStdHandle( STD_OUTPUT_HANDLE ),
				pData, dwSize, &dwWrittenBytes, NULL ) ;
	}
}

// ���s
//////////////////////////////////////////////////////////////////////////////
int EConverterApp::Perform( void )
{
	//
	// ����
	//
	int	nResult = 0 ;
	if ( m_dwFlags & (optUsage | optHelp) )
	{
		return	0 ;
	}
	if ( m_dwFlags & optInform )
	{
		nResult = InformFiles( ) ;
	}
	else
	{
		if ( m_dwFlags & optIndirect )
		{
			nResult = LoadIndirectFile( ) ;
		}
		else
		{
			nResult = MakeProcess( ) ;
		}
		if ( nResult == 0 )
		{
			nResult = BeginProcess( ) ;
		}
	}
	//
	// ���|�[�g�o��
	//
	if ( !m_strReportFile.IsEmpty() )
	{
		ERawFile	file ;
		if ( file.Open( m_strReportFile, file.modeCreate ) )
		{
			printf( "���|�[�g�t�@�C�����J���܂���ł����B\n" ) ;
			return	1 ;
		}
		m_xmlReport.WriteDescription( file ) ;
		file.Close( ) ;
	}
	//
	// �o�͌���
	//
	if ( !m_strOutputFile.IsEmpty() )
	{
		ERawFile	file ;
		if ( file.Open( m_strOutputFile, file.modeCreate ) )
		{
			printf( "�A�E�g�v�b�g�t�@�C�����J���܂���ł����B\n" ) ;
			return	1 ;
		}
		m_bufOutput.WriteToFile( file ) ;
		file.Close( ) ;
	}
	//
	return	nResult ;
}

// MIME ���� FormatType �񋓎q�ւ̕ϊ�
//////////////////////////////////////////////////////////////////////////////
int EConverterApp::FormatTypeFromMIME( const wchar_t * pwszMIME )
{
	EWideString	wstrMIME = pwszMIME ;
	for ( int i = 0; i < ftMax; i ++ )
	{
		if ( wstrMIME.CompareNoCase( m_pwszFormatMime[i] ) == 0 )
		{
			return	i ;
		}
	}
	return	ftInvalid ;
}

// �Ή�����t�H�[�}�b�g�̃f�t�H���g�̊g���q���擾
//////////////////////////////////////////////////////////////////////////////
const wchar_t * EConverterApp::GetDefaultFileExt( int fFmtType )
{
	if ( (fFmtType >= 0) && (fFmtType < ftMax) )
	{
		return	m_pwszFormatExt[fFmtType] ;
	}
	return	NULL ;
}

// �摜�t�@�C������\������
//////////////////////////////////////////////////////////////////////////////
int EConverterApp::InformFiles( void )
{
	//
	// �t�@�C����񋓂���
	//
	EnumFileName	efnFiles( this, L"", EWideString(m_strSrcFile) ) ;
	ESLError		err	;
	EWideString		wstrFileName ;
	EString			strErrMsg ;
	//
	m_xmlReport.SetTag( L"inform" ) ;
	//
	for ( ; ; )
	{
		//
		// �t�@�C�������擾
		//
		err = efnFiles.GetNextFileName( wstrFileName, strErrMsg ) ;
		if ( err )
		{
			if ( !strErrMsg.IsEmpty() )
			{
				printf( strErrMsg ) ;
			}
			break ;
		}
		//
		// �t�@�C�����J��
		//
		ERawFile	file ;
		ERIFile		erif ;
		EString		strFileName = wstrFileName ;
		printf( "%s : ", strFileName.CharPtr() ) ;
		if ( file.Open( strFileName, file.modeRead | file.shareRead ) )
		{
			printf( "�t�@�C�����J���܂���ł����B\n" ) ;
			return	2 ;
		}
		if ( erif.Open( &file ) )
		{
			printf( "����� ERI �摜�t�@�C���ł͂���܂���B\n" ) ;
			continue ;
		}
		//
		// �摜���擾
		//
		ERI_INFO_HEADER &	eih = erif.m_InfoHeader ;
		EString		strTransformation, strArchitexture, strFormatType ;
		EGLSize		sizeImage ;
		EGLPoint	ptHotSpot( 0, 0 ) ;
		int			nBitsPerPixel, nResolution = 0 ;
		//
		switch ( eih.fdwTransformation )
		{
		case	CVTYPE_LOSSLESS_ERI:
			strTransformation = "�t���k" ;
			break ;
		case	CVTYPE_DCT_ERI:
			strTransformation = "��t DCT �ϊ�" ;
			break ;
		case	CVTYPE_LOT_ERI:
			strTransformation = "��t LOT �ϊ�" ;
			break ;
		default:
			strTransformation = "�s���ȉ摜�ϊ�" ;
			break ;
		}
		switch ( eih.dwArchitecture )
		{
		case	ERI_RUNLENGTH_GAMMA:
			strArchitexture = "ERI ����" ;
			break ;
		case	ERI_RUNLENGTH_HUFFMAN:
			strArchitexture = "ERINA ����" ;
			break ;
		case	ERISA_NEMESIS_CODE:
			strArchitexture = "ERISA ����" ;
			break ;
		case	ERI_ARITHMETIC_CODE:
			strArchitexture = "�p�~���ꂽ�Z�p����" ;
			break ;
		default:
			strArchitexture = "�s���ȕ���" ;
			break ;
		}
		switch ( eih.fdwFormatType )
		{
		case	ERI_RGB_IMAGE:
			strFormatType = "RGB �`��" ;
			break ;
		case	ERI_GRAY_IMAGE:
			strFormatType = "�O���C�X�P�[��" ;
			break ;
		case	ERI_RGBA_IMAGE:
			strFormatType = "RGBA �`��" ;
			break ;
		default:
			strFormatType = "�s���Ȍ`��" ;
			break ;
		}
		sizeImage.w = eih.nImageWidth ;
		sizeImage.h = eih.nImageHeight ;
		if ( sizeImage.h < 0 )
		{
			sizeImage.h = - sizeImage.h ;
		}
		nBitsPerPixel = eih.dwBitsPerPixel ;
		//
		if ( erif.m_fdwReadMask & ERIFile::rmDescription )
		{
			ERIFile::ETagInfo	taginf ;
			taginf.CreateTagInfo( erif.m_wstrDescription ) ;
			ptHotSpot = taginf.GetHotSpot( ) ;
			nResolution = taginf.GetResolution( ) ;
		}
		//
		// �摜���̕\��
		//
		printf( "\n\t���k�����F%s�^%s",
			strTransformation.CharPtr(), strArchitexture.CharPtr() ) ;
		printf( "\n\t�t�H�[�}�b�g�F%s", strFormatType.CharPtr() ) ;
		printf( "\n\t�摜�T�C�Y�F%d �~ %d", sizeImage.w, sizeImage.h ) ;
		if ( nResolution > 0 )
		{
			printf( "(%d dpi)", nResolution ) ;
		}
		printf( "\n\t�z�b�g�X�|�b�g�F%d, %d\n\n", ptHotSpot.x, ptHotSpot.y ) ;
		//
		// ���|�[�g�o��
		//
		EDescription *	pdscTag = new EDescription ;
		pdscTag->SetTag( L"file" ) ;
		m_xmlReport.AddContentTag( pdscTag ) ;
		//
		pdscTag->SetContentString
			( L"filename", EWideString(strFileName) ) ;
		pdscTag->SetContentString
			( L"transformation", EWideString(strTransformation) ) ;
		pdscTag->SetContentString
			( L"architecture", EWideString(strArchitexture) ) ;
		pdscTag->SetContentString
			( L"format", EWideString(strFormatType) ) ;
		//
		pdscTag->SetAttrInteger( L"width", sizeImage.w ) ;
		pdscTag->SetAttrInteger( L"height", sizeImage.h ) ;
		if ( nResolution > 0 )
		{
			pdscTag->SetAttrInteger( L"resolution", nResolution ) ;
		}
		pdscTag->SetIntegerAt( L"hotspot", L"x", ptHotSpot.x ) ;
		pdscTag->SetIntegerAt( L"hotspot", L"y", ptHotSpot.y ) ;
		//
		if ( erif.m_fdwReadMask & ERIFile::rmDescription )
		{
			pdscTag->SetContentString
				( L"description", erif.m_wstrDescription ) ;
		}
		if ( erif.m_fdwReadMask & ERIFile::rmCopyright )
		{
			pdscTag->SetContentString
				( L"copyright", erif.m_wstrCopyright ) ;
		}
	}
	//
	return	0 ;
}

// �C���_�C���N�g�t�@�C����ǂݍ���
//////////////////////////////////////////////////////////////////////////////
int EConverterApp::LoadIndirectFile( void )
{
	//
	// �t�@�C����ǂݍ���
	//
	ERawFile		file ;
	EStreamBuffer	buf ;
	if ( file.Open( m_strSrcFile, file.modeRead | file.shareRead ) )
	{
		printf
			( "%s �t�@�C�����J���܂���ł����B\n", m_strSrcFile.CharPtr() ) ;
		return	1 ;
	}
	buf.ReadFromFile( file ) ;
	file.Close( ) ;
	//
	if ( m_xmlCvt.ReadDescription( buf ) )
	{
		printf( "�ϊ������t�@�C���̉�͒��ɃG���[���������܂����B\n" ) ;
		return	0 ;
	}
	//
	// ���t�X�N���v�g���\�z
	//
	m_pxmlErisa = m_xmlCvt.GetContentTagAs( 0, L"erisa" ) ;
	if ( m_pxmlErisa == NULL )
	{
		printf( "<erisa> �^�O��������܂���B\n" ) ;
	}
	ECSSourceStream	cssSource ;
	int				iNextTag = 0 ;
	for ( ; ; )
	{
		EDescription *	pdscScript =
			m_pxmlErisa->GetContentTagAs( iNextTag, L"cotopha", &iNextTag ) ;
		if ( pdscScript == NULL )
		{
			break ;
		}
		EDescription *	pdscComment = pdscScript->GetContentTagAt( 0 ) ;
		if ( (pdscComment == NULL) || pdscComment->IsValid() )
		{
			continue ;
		}
		EStreamWideString	swsScript = pdscComment->Contents( ) ;
		if ( !swsScript.MoveToNextLine( ) )
		{
			int	iFirstLine = swsScript.GetIndex( ) ;
			int	iEndLine = iFirstLine ;
			while ( !swsScript.MoveToNextLine() )
			{
				iEndLine = swsScript.GetIndex( ) ;
			}
			cssSource +=
				swsScript.Middle( iFirstLine, iEndLine - iFirstLine ) ;
		}
	}
	if ( !cssSource.IsEmpty() )
	{
		//
		// �R���p�C�����s
		//
		ECSXCompiler	compiler ;
		ESLError		err ;
		//
		compiler.Initialize( &m_csxi ) ;
		compiler.SetBaseDirectory
			( m_strSrcFile.GetFileDirectoryPart() ) ;
		//
		compiler.CompileScriptLine
			( ECSSourceStream( L"DeclareType Image, ImageContext" ), 0, NULL ) ;
		compiler.CompileScriptLine
			( ECSSourceStream( L"Include \"cotopha.ch\"" ), 0, NULL ) ;
		compiler.CompileScript( cssSource, NULL ) ;
		err = compiler.FinishCompile( ) ;
		if ( err )
		{
			printf( "�X�N���v�g : �G���[ : %s\n", GetESLErrorMsg(err) ) ;
			return	1 ;
		}
		if ( compiler.GetErrorCount() > 0 )
		{
			return	1 ;
		}
		//
		m_csxi.SolveLinkInfo( ) ;
		if ( m_csxi.GetUnsolvedLinkCount() > 0 )
		{
			unsigned int	i, nCount ;
			nCount = m_csxi.GetUnsolvedLinkCount() ;
			for ( i = 0; i < nCount; i ++ )
			{
				ECSWideString	wstrName = m_csxi.GetUnsolveLinkName( i ) ;
				printf( "%s �͖������̎Q�Ƃł��B\n",
							EString(wstrName).CharPtr() ) ;
			}
			return	1 ;
		}
		//
		// �X�N���v�g��������
		//
		InitializeContext( &m_csxi ) ;
	}
	//
	return	0 ;
}

// �N���������珈���葱�����쐬����
//////////////////////////////////////////////////////////////////////////////
int EConverterApp::MakeProcess( void )
{
	EDescription *	pdscFile ;
	m_pxmlErisa = m_xmlCvt.CreateContentTagAs( 0, L"erisa" ) ;
	pdscFile = m_pxmlErisa->CreateContentTagAs( 0, L"file" ) ;
	pdscFile->SetAttrString( L"src", EWideString(m_strSrcFile) ) ;
	pdscFile->SetAttrString( L"dst", EWideString(m_strDstFile) ) ;
	//
	if ( m_nCutAlign != 0 )
	{
		EDescription *	pdscCut ;
		pdscCut = pdscFile->CreateContentTagAs( 0, L"cut" ) ;
	}
	return	0 ;
}

// ���������s����
//////////////////////////////////////////////////////////////////////////////
int EConverterApp::BeginProcess( void )
{
	m_nErrorCount = 0 ;
	m_xmlReport.SetTag( L"process" ) ;
	//
	for ( int i = 0; i < m_pxmlErisa->GetContentTagCount(); i ++ )
	{
		EDescription *	pdscTag = m_pxmlErisa->GetContentTagAt( i ) ;
		if ( pdscTag == NULL )
		{
			continue ;
		}
		if ( pdscTag->Tag() == L"env" )
		{
			SetEnvironment( pdscTag ) ;
		}
		else if ( pdscTag->Tag() == L"file" )
		{
			ProcessFiles( pdscTag ) ;
		}
	}
	return	m_nErrorCount ;
}

// ���ݒ���s��
//////////////////////////////////////////////////////////////////////////////
void EConverterApp::SetEnvironment( EDescription * pdscEnv )
{
	if ( pdscEnv->GetAttributeIndexAs( L"mime" ) >= 0 )
	{
		EWideString	wstrMime = pdscEnv->GetAttrString( L"mime", NULL ) ;
		int	nFmtType = FormatTypeFromMIME( wstrMime ) ;
		if ( nFmtType == ftInvalid )
		{
			printf( "<env> : mime = \"%s\" ��"
				"�Ή����Ă��Ȃ��o�͌`���ł�\n", EString(wstrMime).CharPtr() ) ;
			m_nErrorCount ++ ;
		}
	}
	if ( pdscEnv->GetAttributeIndexAs( L"dstdir" ) >= 0 )
	{
		m_wstrDstDir = pdscEnv->GetAttrString( L"dstdir", NULL ) ;
		if ( (m_wstrDstDir.GetLength() > 1)
			&& (m_wstrDstDir.Right(1) != L"\\") )
		{
			m_wstrDstDir += L'\\' ;
		}
		if ( (m_wstrDstDir.GetAt(0) != L'\\')
				&& (m_wstrDstDir.Find( L':' ) < 0) )
		{
			EWideString	wstrDir = m_strSrcFile.GetFileDirectoryPart( ) ;
			wstrDir += m_wstrDstDir ;
			m_wstrDstDir = wstrDir ;
		}
	}
	if ( pdscEnv->GetAttributeIndexAs( L"srcdir" ) >= 0 )
	{
		m_wstrSrcDir = pdscEnv->GetAttrString( L"srcdir", NULL ) ;
		if ( (m_wstrSrcDir.GetLength() > 1)
			&& (m_wstrSrcDir.Right(1) != L"\\") )
		{
			m_wstrSrcDir += L'\\' ;
		}
		if ( (m_wstrSrcDir.GetAt(0) != L'\\')
				&& (m_wstrSrcDir.Find( L':' ) < 0) )
		{
			EWideString	wstrDir = m_strSrcFile.GetFileDirectoryPart( ) ;
			wstrDir += m_wstrSrcDir ;
			m_wstrSrcDir = wstrDir ;
		}
	}
	m_nPutAlign = pdscEnv->GetAttrInteger( L"put_align", m_nPutAlign ) ;
	m_nCutAlign = pdscEnv->GetAttrInteger( L"cut_align", m_nCutAlign ) ;
	m_nCutMargin = pdscEnv->GetAttrInteger( L"cut_margin", m_nCutMargin ) ;
	m_nCutThreshold =
		pdscEnv->GetAttrInteger( L"cut_threshold", m_nCutThreshold ) ;
	m_fKeepHotspot =
		pdscEnv->GetAttrInteger( L"keep_hotspot", m_fKeepHotspot ) ;
	m_fLayerBlend =
		pdscEnv->GetAttrInteger( L"layer_blend", m_fLayerBlend ) ;
	m_fLayerMask =
		pdscEnv->GetAttrInteger( L"layer_mask", m_fLayerMask ) ;
}

// �t�@�C���������s��
//////////////////////////////////////////////////////////////////////////////
void EConverterApp::ProcessFiles( EDescription * pdscFile )
{
	if ( pdscFile->GetAttributeIndexAs( L"src" ) >= 0 )
	{
		//
		// ���o�̓t�@�C�����񋓂̏���
		//
		EnumFileName	efnSrc( this ), efnDst( this ) ;
		EWideString		wstrDstDir ;
		EWideString		wstrDstFile =
				pdscFile->GetAttrString( L"dst", NULL ) ;
		efnSrc.SetEnumFiles
			( m_wstrSrcDir, pdscFile->GetAttrString( L"src", NULL ) ) ;
		if ( (wstrDstFile.Right(1) != L"\\") && !wstrDstFile.IsEmpty() )
		{
			efnDst.SetEnumFiles( m_wstrDstDir, wstrDstFile ) ;
			if ( efnDst.m_type == EnumFileName::typeWin32 )
			{
				if ( (wstrDstFile.GetAt(0) != L'\\')
					&& (wstrDstFile.Find( L':' ) < 0) )
				{
					wstrDstFile = m_wstrDstDir + wstrDstFile ;
				}
			}
		}
		else
		{
			if ( (wstrDstFile.GetAt(0) != L'\\')
				&& (wstrDstFile.Find( L':' ) < 0) )
			{
				wstrDstDir = m_wstrDstDir + wstrDstFile ;
			}
			else
			{
				wstrDstDir = wstrDstFile ;
			}
		}
		long int	nIndex = 0 ;
		EString		strErrMsg ;
		EWideString	wstrSrcFile ;
		for ( ; ; )
		{
			//
			// ���̓t�@�C�����擾
			//
			efnSrc.m_lstArg.SetAt( 0, new ECSInteger( nIndex ) ) ;
			if ( efnSrc.GetNextFileName( wstrSrcFile, strErrMsg ) )
			{
				if ( !strErrMsg.IsEmpty() )
				{
					printf( strErrMsg + "\n" ) ;
					m_nErrorCount ++ ;
				}
				break ;
			}
			//
			// �o�̓t�@�C��������
			//
			if ( efnDst.m_type == EnumFileName::typeWin32 )
			{
			}
			else if ( efnDst.m_type )
			{
				efnDst.m_lstArg.SetAt
					( 0, new ECSInteger( nIndex ) ) ;
				efnDst.m_lstArg.SetAt
					( 0, new ECSString( wstrSrcFile.GetFileNamePart() ) ) ;
				if ( efnDst.GetNextFileName( wstrDstFile, strErrMsg ) )
				{
					if ( !strErrMsg.IsEmpty() )
					{
						printf( strErrMsg + "\n" ) ;
						m_nErrorCount ++ ;
					}
					else
					{
						printf
							( "%s �ɑΉ�����o�̓t�@�C������"
								"����ł��܂���ł����B\n",
								EString(wstrSrcFile).CharPtr() ) ;
						m_nErrorCount ++ ;
					}
					break ;
				}
				wstrDstFile =
					NormalizeDestinationFileName( wstrDstFile, wstrSrcFile ) ;
			}
			else if ( !wstrDstFile.IsEmpty() )
			{
				wstrDstFile = wstrDstDir
					+ wstrSrcFile.GetFileTitlePart()
					+ L"." + GetDefaultFileExt( m_nFmtType ) ;
			}
			if ( wstrSrcFile.CompareNoCase( wstrDstFile ) == 0 )
			{
				printf
					( "%s �͓��o�̓t�@�C�������d�Ȃ��Ă��܂��B\n",
						EString(wstrSrcFile).CharPtr() ) ;
				m_nErrorCount ++ ;
				continue ;
			}
			//
			// ���̓t�@�C����ǂݍ���
			//
			ECSImageContext	imgctx ;
			EString			strFileName = wstrSrcFile ;
			EWideString		wstrHotSpot =
					pdscFile->GetAttrString( L"hotspot", NULL ) ;
			//
			printf( "%s : \n", strFileName.CharPtr() ) ;
			if ( LoadImageFile
				( imgctx, wstrSrcFile, wstrHotSpot, nIndex ) )
			{
				printf( "�ǂݍ��݂Ɏ��s���܂����B\n\n" ) ;
				m_nErrorCount ++ ;
				return ;
			}
			//
			// �����J�n
			//
			ProcessFileTags( imgctx, pdscFile ) ;
			//
			// �ۑ�
			//
			if ( !wstrDstFile.IsEmpty() )
			{
				printf( " --> %s\n", EString(wstrDstFile).CharPtr() ) ;
				if ( SaveImageContext( imgctx, wstrDstFile, m_nFmtType ) )
				{
					printf( "    �����o���Ɏ��s���܂����B\n" ) ;
					m_nErrorCount ++ ;
					return ;
				}
			}
			else
			{
				if ( imgctx.m_varArray.GetSize() != 0 )
				{
					printf( "�i�摜�R���e�L�X�g�Ɏc���Ă���"
								"�摜�f�[�^�͔j������܂��j\n" ) ;
				}
			}
			printf( "\n" ) ;
			//
			nIndex ++ ;
		}
	}
	else
	{
		//
		// �P��t�@�C���̏���
		//
		ECSImageContext	imgctx ;
		EWideString		wstrDst = pdscFile->GetAttrString( L"dst", NULL ) ;
		printf( "file : \n" ) ;
		ProcessFileTags( imgctx, pdscFile ) ;
		//
		// �ۑ�
		//
		if ( !wstrDst.IsEmpty() )
		{
			if ( (wstrDst.GetAt(0) != L'\\')
				&& (wstrDst.Find( L':' ) < 0) )
			{
				wstrDst = m_wstrDstDir + wstrDst ;
			}
			printf( " --> %s\n", EString(wstrDst).CharPtr() ) ;
			if ( SaveImageContext( imgctx, wstrDst, m_nFmtType ) )
			{
				m_nErrorCount ++ ;
				return ;
			}
		}
		else
		{
			if ( imgctx.m_varArray.GetSize() != 0 )
			{
				printf( "�i�摜�R���e�L�X�g�Ɏc���Ă���"
							"�摜�f�[�^�͔j������܂��j\n" ) ;
			}
		}
		printf( "\n" ) ;
	}
}

// �t�@�C������
//////////////////////////////////////////////////////////////////////////////
void EConverterApp::ProcessFileTags
	( ECSImageContext & imgctx, EDescription * pdscFile )
{
	//
	// �X�N���v�g�Ăяo��
	//
	EWideString	wstrScript = pdscFile->GetAttrString( L"script", NULL ) ;
	if ( !wstrScript.IsEmpty() )
	{
		DWORD *	pdwFuncAddr = m_csxi.GetFunctionAddress( wstrScript ) ;
		if ( pdwFuncAddr != NULL )
		{
			EObjArray<ECSObject>	lstArg ;
			lstArg.SetAt( 0, new ECSReference( &imgctx ) ) ;
			ESLError	err = CallFunction( *pdwFuncAddr, lstArg ) ;
			if ( err )
			{
				printf( "\n�G���[�F%s\n", GetESLErrorMsg(err) ) ;
			}
		}
		else
		{
			printf( "%s �֐���������܂���B\n",
						EString(wstrScript).CharPtr() ) ;
		}
	}
	//
	// �e�^�O�̏���
	//
	for ( int iTag = 0; iTag < pdscFile->GetContentTagCount(); iTag ++ )
	{
		EDescription *	pdscTag = pdscFile->GetContentTagAt( iTag ) ;
		if ( pdscTag == NULL )
		{
			continue ;
		}
		if ( pdscTag->Tag() == L"cut" )
		{
			//
			// �摜�̐؂�o���E�T�C�Y����
			//
			EGL_IMAGE_RECT	irCut ;
			int				nAlign, nMargin, nThreshold, fKeepHotspot ;
			int				nOpFlags = 0 ;
			EWideString		wstrAlphaOp ;
			nAlign = pdscTag->GetAttrInteger( L"align", m_nCutAlign ) ;
			nMargin = pdscTag->GetAttrInteger( L"margin", m_nCutMargin ) ;
			nThreshold =
				pdscTag->GetAttrInteger( L"threshold", m_nCutThreshold ) ;
			fKeepHotspot =
				pdscTag->GetAttrInteger( L"keep_hotspot", m_fKeepHotspot ) ;
			if ( fKeepHotspot )
			{
				nOpFlags |= ECSImageContext::cofKeepHotspot ;
			}
			wstrAlphaOp = pdscTag->GetAttrString( L"op_alpha", L"auto" ) ;
			if ( wstrAlphaOp == L"remove" )
			{
				nOpFlags |= ECSImageContext::cofRemoveAlpha ;
			}
			else if ( wstrAlphaOp != L"keep" )
			{
				nOpFlags |= ECSImageContext::cofAutoAlpha ;
			}
			if ( !imgctx.CutImage
				( nAlign, nMargin, nThreshold, nOpFlags, irCut ) )
			{
				printf( "cut : (%d, %d) - (%d x %d)\n",
						irCut.x, irCut.y, irCut.w, irCut.h ) ;
				//
				EDescription *	pdscComment = new EDescription ;
				pdscComment->SetCommentTag
					( imgctx.m_strFileName.m_varStr + L" �̐؂�o��" ) ;
				m_xmlReport.AddContentTag( pdscComment ) ;
				//
				EDescription *	pdscCut = new EDescription ;
				pdscCut->SetTag( L"cut" ) ;
				pdscCut->SetAttrInteger( L"x", irCut.x ) ;
				pdscCut->SetAttrInteger( L"y", irCut.y ) ;
				pdscCut->SetAttrInteger( L"width", irCut.w ) ;
				pdscCut->SetAttrInteger( L"height", irCut.h ) ;
				pdscCut->SetAttrString
					( L"file", imgctx.m_strFileName.m_varStr ) ;
				m_xmlReport.AddContentTag( pdscCut ) ;
			}
			else
			{
				m_nErrorCount ++ ;
			}
		}
		else if ( pdscTag->Tag() == L"arrange" )
		{
			//
			// �摜�̐���z��
			//
			bool	fWayVert, fPutRight ;
			fWayVert = (pdscTag->GetAttrString( L"way", L"horz" ) == L"vert") ;
			fPutRight = fWayVert ;
			if ( pdscTag->GetAttributeIndexAs( L"put" ) >= 0 )
			{
				fPutRight =
					(pdscTag->GetAttrString( L"put", L"under" ) == L"right") ;
			}
			int		nAlign = pdscTag->GetAttrInteger( L"align", m_nPutAlign ) ;
			//
			m_nErrorCount +=
				imgctx.ArrangeImage( fWayVert, fPutRight, nAlign ) ;
		}
		else if ( pdscTag->Tag() == L"animation" )
		{
			//
			// �A�j���[�V�����̐���
			//
			EWideString	wstrSeq = pdscTag->GetAttrString( L"seq", NULL ) ;
			int			nDuration =
				pdscTag->GetAttrInteger
					( L"duration", imgctx.m_intDuration.m_varInt ) ;
			//
			m_nErrorCount +=
				imgctx.MakeAnimation( wstrSeq, nDuration ) ;
		}
		else if ( pdscTag->Tag() == L"write" )
		{
			//
			// �A�ԃt�@�C���ɏo��
			//
			WriteSequenceFiles
				( imgctx, pdscTag->GetAttrString( L"dst", NULL ) ) ;
		}
		else if ( pdscTag->Tag() == L"image" )
		{
			//
			// �摜�t�@�C���ǂݍ���
			//
			ProcessImages( imgctx, pdscTag ) ;
		}
		else if ( pdscTag->Tag() == L"remove" )
		{
			//
			// �摜�R���e�L�X�g�̍폜
			//
			imgctx.DeleteContext( ) ;
		}
		else if ( pdscTag->Tag() == L"select" )
		{
			//
			// �摜�R���e�L�X�g�𐶐�
			//
			ProcessSelectImage( imgctx, pdscTag ) ;
		}
	}
}

// �摜��I��
//////////////////////////////////////////////////////////////////////////////
void EConverterApp::ProcessSelectImage
	( ECSImageContext & imgctx, EDescription * pdscSelect )
{
	//
	// �p�����[�^�擾
	//
	EWideString	wstrDst = pdscSelect->GetAttrString( L"dst", NULL ) ;
	EStreamWideString	swsLayer =
				pdscSelect->GetAttrString( L"layer", NULL ) ;
	EStreamWideString	swsFrame =
				pdscSelect->GetAttrString( L"frame", NULL ) ;
	//
	EObjArray<EWideString>	lstParam ;
	EStreamWideString	swsUsage = L"(<0-9>*) [\\- (<0-9>*)] [\\, (%n)] \\" ;
	int			iFirst, iEnd, nStep = 0 ;
	EString		strErrMsg ;
	if ( !swsFrame.IsMatchUsage( swsUsage, strErrMsg, &lstParam ) )
	{
		iFirst = lstParam[0].AsInteger( ) ;
		if ( !lstParam[1].IsEmpty() )
		{
			iEnd = lstParam[1].AsInteger( ) ;
		}
		else
		{
			iEnd = iFirst ;
		}
		if ( !lstParam[2].IsEmpty() )
		{
			nStep = lstParam[3].AsInteger( ) ;
		}
		else if ( iFirst > iEnd )
		{
			nStep = -1 ;
		}
		else
		{
			nStep = 1 ;
		}
	}
	//
	// �摜���𕡐�
	//
	ECSImageContext	ictxSel ;
	ictxSel.m_sizeImage.SetMemberAsInt
			( L"w", imgctx.m_sizeImage.GetMemberAsInt( L"w", 0 ) ) ;
	ictxSel.m_sizeImage.SetMemberAsInt
			( L"h", imgctx.m_sizeImage.GetMemberAsInt( L"h", 0 ) ) ;
	ictxSel.m_ptHotSpot.SetMemberAsInt
			( L"x", imgctx.m_ptHotSpot.GetMemberAsInt( L"x", 0 ) ) ;
	ictxSel.m_ptHotSpot.SetMemberAsInt
			( L"y", imgctx.m_ptHotSpot.GetMemberAsInt( L"y", 0 ) ) ;
	ictxSel.m_strFileName.m_varStr = imgctx.m_strFileName.m_varStr ;
	//
	// �摜��I��
	//
	if ( !swsLayer.IsEmpty() )
	{
		if ( nStep == 0 )
		{
			iFirst = 0 ;
			iEnd = imgctx.GetLength() - 1 ;
			nStep = 1 ;
		}
	}
	if ( nStep != 0 )
	{
		for ( int i = iFirst; i * nStep <= iEnd * nStep; i += nStep )
		{
			ECSImage *	pImage = imgctx.GetImageAt( i ) ;
			if ( pImage != NULL )
			{
				bool	fSelect = true ;
				if ( !swsLayer.IsEmpty() )
				{
					EStreamWideString	swsName ;
					EString				strErrMsg ;
					swsLayer.MoveIndex( 0 ) ;
					swsName = pImage->m_strName.m_varStr ;
					if ( swsName.IsMatchUsage( swsLayer, strErrMsg ) )
					{
						fSelect = false ;
					}
				}
				if ( fSelect )
				{
					ECSImage *	pDupImage = (ECSImage*) pImage->Duplicate( ) ;
					if ( wstrDst.IsEmpty() )
					{
						imgctx.m_varArray.RemoveAt( i ) ;
						iEnd -= nStep ;
						i -= nStep ;
					}
					ictxSel.m_varArray.Add( pDupImage ) ;
				}
			}
		}
	}
	//
	// �������s
	//
	printf( "select : \n" ) ;
	ProcessFileTags( ictxSel, pdscSelect ) ;
	//
	// ���ʏo��
	//
	if ( !wstrDst.IsEmpty() )
	{
		wstrDst = NormalizeDestinationFileName
					( wstrDst, ictxSel.m_strFileName.m_varStr ) ;
		printf( " --> %s\n", EString(wstrDst).CharPtr() ) ;
		//
		if ( (wstrDst.GetAt(0) != L'\\') && (wstrDst.Find( L':' ) < 0) )
		{
			wstrDst = m_wstrDstDir + wstrDst ;
		}
		if ( SaveImageContext( ictxSel, wstrDst, m_nFmtType ) )
		{
			m_nErrorCount ++ ;
			return ;
		}
	}
	else
	{
		imgctx.MergeContext( ictxSel ) ;
	}
}

// �t�@�C����ǂݍ���
//////////////////////////////////////////////////////////////////////////////
void EConverterApp::ProcessImages
	( ECSImageContext & imgctx, EDescription * pdscImage )
{
	//
	// ���̓t�@�C�����擾
	//
	EObjArray<EWideString>	lstSources ;
	EnumFileName			efnSrc( this ) ;
	EWideString	wstrSrc = pdscImage->GetAttrString( L"src", NULL ) ;
	if ( wstrSrc.IsEmpty() )
	{
		EStreamWideString	swsSources =
			pdscImage->GetContentString( NULL, NULL ) ;
		int		i = 0 ;
		while ( !swsSources.DisregardSpace() )
		{
			EWideString	wstrFile =
				swsSources.GetEnclosedString( L';', FALSE ) ;
			wstrFile.TrimRight( ) ;
			if ( (wstrFile.GetAt(0) != L'\\')
				&& (wstrFile.Find( L':' ) < 0) )
			{
				lstSources[i ++] = m_wstrSrcDir + wstrFile ;
			}
			else
			{
				lstSources[i ++] = wstrFile ;
			}
		}
	}
	else
	{
		efnSrc.SetEnumFiles( m_wstrSrcDir, wstrSrc ) ;
	}
	//
	// ���`���l���p�t�@�C�����擾
	//
	EWideString		wstrAlpha = pdscImage->GetAttrString( L"alpha", NULL ) ;
	EnumFileName	efnAlpha( this ) ;
	if ( !wstrAlpha.IsEmpty() )
	{
		efnAlpha.SetEnumFiles( m_wstrSrcDir, wstrAlpha ) ;
	}
	//
	// �z�b�g�X�|�b�g�擾
	//
	EWideString	wstrHotSpot = pdscImage->GetAttrString( L"hotspot", NULL ) ;
	//
	// �����摜�t�@�C����ǂݍ���
	//
	printf( "<image> :\n" ) ;
	//
	int		nIndex = 0 ;
	EString	strErrMsg ;
	for ( ; ; )
	{
		//
		// ���̓t�@�C�����擾
		//
		if ( efnSrc.m_type )
		{
			efnSrc.m_lstArg.SetAt( 0, new ECSInteger( nIndex ) ) ;
			if ( efnSrc.GetNextFileName( wstrSrc, strErrMsg ) )
			{
				if ( !strErrMsg.IsEmpty() )
				{
					printf( "  %s\n", strErrMsg.CharPtr() ) ;
					m_nErrorCount ++ ;
				}
				break ;
			}
		}
		else
		{
			if ( nIndex >= (int) lstSources.GetSize() )
			{
				break ;
			}
			wstrSrc = lstSources[nIndex] ;
		}
		//
		// �摜�t�@�C���̓ǂݍ���
		//
		ECSImageContext	icImage ;
		printf( "  %s : ", EString(wstrSrc).CharPtr() ) ;
		if ( LoadImageFile( icImage, wstrSrc, wstrHotSpot, nIndex ) )
		{
			printf( "\n  �ǂݍ��݂Ɏ��s���܂����B\n" ) ;
			continue ;
		}
		printf( "OK\n" ) ;
		//
		// ���`���l���摜�̓ǂݍ���
		//
		if ( efnAlpha.m_type )
		{
			efnAlpha.m_lstArg.SetAt( 0, new ECSInteger( nIndex ) ) ;
			efnAlpha.m_lstArg.SetAt( 1, new ECSString( wstrSrc ) ) ;
			if ( efnAlpha.GetNextFileName( wstrAlpha, strErrMsg ) )
			{
				if ( !strErrMsg.IsEmpty() )
				{
					printf( "  alpha : %s\n", strErrMsg.CharPtr() ) ;
					m_nErrorCount ++ ;
				}
				break ;
			}
			printf( "  alpha : %s : ", EString(wstrAlpha).CharPtr() ) ;
			//
			ECSImageContext	icAlpha ;
			if ( LoadImageFile( icAlpha, wstrAlpha, NULL, nIndex ) )
			{
				printf( "\n  �ǂݍ��݂Ɏ��s���܂���\n" ) ;
			}
			else if ( icImage.m_imgBuf.GetSize()
							!= icAlpha.m_imgBuf.GetSize() )
			{
				printf( "\n  �摜�T�C�Y����v���Ȃ��̂�"
							"���`���l���������ł��܂���B\n" ) ;
			}
			else
			{
				printf( "OK\n" ) ;
				//
				while ( icImage.m_imgBuf.GetTotalFrameCount() > 1 )
				{
					icImage.m_imgBuf.RemoveFrameAt( 1 ) ;
				}
				while ( icAlpha.m_imgBuf.GetTotalFrameCount() > 1 )
				{
					icAlpha.m_imgBuf.RemoveFrameAt( 1 ) ;
				}
				icImage.m_imgBuf.GetSequenceTable().RemoveAll( ) ;
				icImage.m_imgBuf.GetSequenceTable().SetAt( 0, 0 ) ;
				//
				icAlpha.m_imgBuf.ConvertFormatTo( EIF_GRAY_BITMAP, 8 ) ;
				icImage.m_imgBuf.ConvertFormatTo( EIF_RGBA_BITMAP, 32 ) ;
				icImage.m_imgBuf.BlendAlphaChannel
					( NULL, icAlpha.m_imgBuf, EGL_BAC_MULTIPLY ) ;
			}
		}
		//
		// �摜�o�b�t�@�ɒǉ�
		//
		imgctx.MergeContext( icImage ) ;
		//
		nIndex ++ ;
	}
}

// �A�ԃt�@�C���o��
//////////////////////////////////////////////////////////////////////////////
void EConverterApp::WriteSequenceFiles
	( ECSImageContext & imgctx, const wchar_t * pwszDstFile )
{
	EnumFileName	efnDst( this, m_wstrDstDir, pwszDstFile ) ;
	if ( efnDst.m_type == EnumFileName::typeWin32 )
	{
		printf( "\'%s\' �A�ԃt�@�C���̏������s���ł��B\n",
								EString(pwszDstFile).CharPtr() ) ;
		m_nErrorCount ++ ;
		return ;
	}
	int	i ;
	for ( i = 0; i < imgctx.GetLength(); i ++ )
	{
		ECSImage *	pImage = imgctx.GetImageAt( i ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		EWideString	wstrDst ;
		EString		strErrMsg ;
		efnDst.m_lstArg.SetAt( 0, new ECSReference( &imgctx ) ) ;
		efnDst.m_lstArg.SetAt( 1, new ECSInteger( i ) ) ;
		if ( efnDst.GetNextFileName( wstrDst, strErrMsg ) )
		{
			if ( !strErrMsg.IsEmpty() )
			{
				printf( "%s\n", strErrMsg.CharPtr() ) ;
			}
			break ;
		}
		//
		wstrDst = NormalizeDestinationFileName
					( wstrDst, imgctx.m_strFileName.m_varStr ) ;
		printf( " --> %s\n", EString(wstrDst).CharPtr() ) ;
		//
		if ( SaveImageFile( pImage->m_imgBuf, wstrDst, m_nFmtType ) )
		{
			printf( "    �����o���Ɏ��s���܂����B\n" ) ;
			m_nErrorCount ++ ;
		}
	}
	imgctx.m_varArray.RemoveBetween( 0, i ) ;
}

// �o�̓t�@�C�����𐮌`����
//////////////////////////////////////////////////////////////////////////////
EWideString EConverterApp::NormalizeDestinationFileName
	( const wchar_t * pwszDst, const wchar_t * pwszSrc )
{
	EWideString	wstrDst, wstrBuf = pwszDst ;
	EWideString	wstrSrc = EWideString(pwszSrc).GetFileTitlePart() ;
	//
	// * �L���̒u������
	//
	int	iLast = 0, iNext ;
	for ( ; ; )
	{
		iNext = wstrBuf.Find( L'*', iLast ) ;
		if ( iNext < 0 )
		{
			break ;
		}
		wstrDst += wstrBuf.Middle( iLast, iNext - iLast ) ;
		wstrDst += wstrSrc ;
		iLast = iNext + 1 ;
	}
	wstrDst += wstrBuf.Middle( iLast ) ;
	//
	// ? �L���̒u������
	//
	int	i = 0, j = 0 ;
	while ( i < (int) wstrDst.GetLength() )
	{
		if ( wstrDst[i] == L'?' )
		{
			if ( j < (int) wstrSrc.GetLength() )
			{
				wstrDst.SetAt( i, wstrSrc.GetAt( j ++ ) ) ;
			}
			else
			{
				wstrDst = wstrDst.Left( i ) + wstrDst.Middle( i + 1 ) ;
			}
		}
		i ++ ;
	}
	return	wstrDst ;
}

// �摜��ǂݍ���
//////////////////////////////////////////////////////////////////////////////
ESLError EConverterApp::LoadImageFile
	( ECSImageContext & imgctx,
		const wchar_t * pwszSrc,
		const wchar_t * pwszHotSpot, int nIndex )
{
	//
	// �摜�ǂݍ���
	//
	if ( imgctx.LoadImageFile( pwszSrc, m_fLayerBlend, m_fLayerMask ) )
	{
		return	eslErrSuccess ;
	}
	//
	// �z�b�g�X�|�b�g�ݒ�
	//
	EStreamWideString	swsHotSpot = pwszHotSpot ;
	if ( !swsHotSpot.IsEmpty() )
	{
		EObjArray<EWideString>	lstParam ;
		EStreamWideString		swsUsage = L"(%n),(%n)\\" ;
		EString					strErrMsg ;
		if ( swsHotSpot.CurrentCharacter() == L'$' )
		{
			DWORD *	pdwFuncAddr =
				m_csxi.GetFunctionAddress( swsHotSpot.Middle(1) ) ;
			if ( pdwFuncAddr != NULL )
			{
				EObjArray<ECSObject>	lstArg ;
				lstArg.SetAt( 0, new ECSReference( &imgctx ) ) ;
				lstArg.SetAt( 1, new ECSInteger( nIndex ) ) ;
				ESLError	err = CallFunction( *pdwFuncAddr, lstArg ) ;
				if ( err )
				{
					printf( "\n�G���[�F%s\n", GetESLErrorMsg(err) ) ;
				}
			}
			else
			{
				printf( "hotspot : %s �֐���������܂���B\n",
						EString(swsHotSpot.Middle(1)).CharPtr() ) ;
			}
		}
		else if ( !swsHotSpot.IsMatchUsage( swsUsage, strErrMsg, &lstParam ) )
		{
			imgctx.m_ptHotSpot.
				SetMemberAsInt( L"x", lstParam[0].AsInteger() ) ;
			imgctx.m_ptHotSpot.
				SetMemberAsInt( L"y", lstParam[1].AsInteger() ) ;
		}
		else
		{
			printf( "\'%s\' �͕s���ȃz�b�g�X�|�b�g�̏����ł��B\n",
									EString(pwszHotSpot).CharPtr() ) ;
		}
	}
	//
	return	eslErrSuccess ;
}

// �摜���t�@�C���ɏ����o��
//////////////////////////////////////////////////////////////////////////////
ESLError EConverterApp::SaveImageContext
	( ECSImageContext & imgctx, const wchar_t * pwszDst, int nFmtType )
{
	EGL_IMAGE_RECT	irClip ;
	if ( imgctx.m_imgBuf.GetInfo() == NULL )
	{
		m_nErrorCount += imgctx.CutImage
			( m_nCutAlign, m_nCutMargin,
				m_nCutThreshold, m_fKeepHotspot, irClip ) ;
		m_nErrorCount +=
			imgctx.MakeAnimation( NULL, imgctx.m_intDuration.m_varInt ) ;
	}
	return	SaveImageFile( imgctx.m_imgBuf, pwszDst, m_nFmtType ) ;
}

ESLError EConverterApp::SaveImageFile
	( EGLMediaLoader & imgBuf, const wchar_t * pwszDst, int nFmtType )
{
	EWideString	wstrDst = pwszDst ;
	EWideString	wstrExt = wstrDst.GetFileExtensionPart( ) ;
	if ( wstrExt.IsEmpty() )
	{
		if ( wstrDst.Right(1) != L"." )
		{
			wstrDst += L'.' ;
		}
		wstrDst += GetDefaultFileExt( m_nFmtType ) ;
		pwszDst = wstrDst ;
	}
	if ( (nFmtType >= ftERI) && (nFmtType <= ftERISA) )
	{
		//
		// ERI �t�H�[�}�b�g
		//
		static const EGLImage::CompressTypeFlag	ctfTypes[] =
		{
			EGLImage::ctfCompatibleFormat,
			EGLImage::ctfExtendedFormat,
			EGLImage::ctfSuperiorArchitecure
		} ;
		static const DWORD	dwFlags[] =
		{
			ERISAEncoder::efBestCmpr,
			ERISAEncoder::efHighCmpr,
			ERISAEncoder::efNormalCmpr,
			ERISAEncoder::efLowCmpr
		} ;
		ERawFile	file ;
		if ( file.Open( EString(pwszDst), file.modeCreate ) )
		{
			return	eslErrGeneral ;
		}
		if ( imgBuf.WriteImageFile
			( file, ctfTypes[nFmtType - ftERI], dwFlags[m_nCmprMode] ) )
		{
			return	eslErrGeneral ;
		}
		file.Close( ) ;
		return	eslErrSuccess ;
	}
	else if ( nFmtType == ftBMP )
	{
		//
		// Windows Bitmap �t�H�[�}�b�g
		//
		ERawFile	file ;
		if ( file.Open( EString(pwszDst), file.modeCreate ) )
		{
			return	eslErrGeneral ;
		}
		if ( imgBuf.WriteBitmapFile( file ) )
		{
			return	eslErrGeneral ;
		}
		file.Close( ) ;
		return	eslErrSuccess ;
	}
	else
	{
		//
		// ���̑��̃t�H�[�}�b�g
		//
		const wchar_t *	pwszMimeType = NULL ;
		switch ( nFmtType )
		{
		case	ftTIFF:
			pwszMimeType = L"image/tiff" ;
			break ;
		case	ftPNG:
			pwszMimeType = L"image/png" ;
			break ;
		case	ftJPEG:
			pwszMimeType = L"image/jpeg" ;
			break ;
		default:
			return	eslErrGeneral ;
		}
		if ( !EGLMediaLoader::IsInstalledGDIplus() )
		{
			printf( "  %s �`���ŕۑ�����ɂ� GDI+ ���K�v�ł��B\n",
									EString(pwszMimeType).CharPtr() ) ;
			return	eslErrGeneral ;
		}
		return	imgBuf.SaveWithGDIplus( EString(pwszDst), pwszMimeType ) ;
	}
}

