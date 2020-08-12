
/*****************************************************************************
                    NOA32 �A�[�J�C�o �R���\�[����
 *****************************************************************************/


#include "noa32.h"


//
// ���C���֐�
//////////////////////////////////////////////////////////////////////////////
int main( int argc, char * argv[] )
{
	::eriInitializeLibrary( ) ;
	//
	// �\�����
	//
	ENoaApp	app ;
	const char *	pszErr ;
	pszErr = app.SetArgument( argc, argv ) ;
	//
	if ( !app.m_fNologo )
	{
		static const char *	pszLogo =
			"�T���A�[�J�C�o version 1.08\n"
			"Copyright (c) 2002-2005 Leshade Entis, Entis-soft. "
			"All rights reserved.\n\n" ;
		//
		printf( pszLogo ) ;
	}
	if ( pszErr != NULL )
	{
		printf( pszErr ) ;
		return	1 ;
	}
	if ( app.m_fHelp )
	{
		static const char *	pszHelp =
			"���� : noa32c [options] [<source-file>] <destination>\n"
			"\n"
			"options;\n"
			"\t/p               : �A�[�J�C�u�����܂�\n"
			"\t/x               : �A�[�J�C�u��W�J���܂�\n"
			"\t/r               : �����I�Ɉ��k�𖳌������Ȃ�\n"
			"\t/pass <password> : �p�X���[�h��ݒ肵�܂�\n"
			"\t/gp              : �A�[�J�C�u���̍ہA�����I�Ƀp�X���[�h�𐶐����܂�\n"
			"\t/l <list-file>   : ���X�g�t�@�C�����w�肵�܂�\n"
			"\t/d               : �T�u�f�B���N�g�����A�[�J�C�u�����܂�\n"
			"\t/raw             : ���f�[�^�̂܂܃A�[�J�C�u�����܂�\n"
			"\t/erisa           : ���k���ăA�[�J�C�u�����܂�\n"
			"\t/bshf            : �Í������ăA�[�J�C�u�����܂�\n"
			"\t/erisa_bshf      : ���k���Í������ăA�[�J�C�u�����܂�\n"
			"\t/time            : ���k�E�W�J�ɗv�������Ԃ��v�����܂��B\n"
			"\n"
			"source-file;\n"
			"�@�A�[�J�C�u�t�@�C�����A���̓A�[�J�C�u������t�@�C�������w�肵�܂��B\n"
			"�@�A�[�J�C�u���̍ۂɂ́A���[���h�J�[�h���g�p�ł��܂��B\n"
			"�@�A�[�J�C�u���̍ۂ� /l ���w�肵�Ă���ƁA���X�g�t�@�C�����g���ăA�[�J�C�u�����܂��B\n"
			"\n"
			"destination;\n"
			"�@�t�@�C���̓W�J��f�B���N�g���A���̓A�[�J�C�u�t�@�C�������w�肵�܂��B\n\n" ;
		//
		printf( pszHelp ) ;
		return	0 ;
	}
	//
	if ( app.m_fPackaging )
	{
		return	app.PackFiles( ) ;
	}
	else
	{
		return	app.DepackFiles( ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// �A�v���P�[�V�����N���X
//////////////////////////////////////////////////////////////////////////////

// �\�z�֐�
//////////////////////////////////////////////////////////////////////////////
ENoaApp::ENoaApp( void )
{
	m_fNologo = false ;
	m_fHelp = false ;
	m_fPackaging = true ;
	m_fAutoRaw = true ;
	m_fGenPassword = false ;
	m_fSubDirectory = false ;
	m_fGenListFile = false ;
	m_fTime = false ;
	m_fPackOption = ERISAArchive::etERISACode ;
	m_dwRndSeed = ::GetTickCount( ) ;
}

// ���Ŋ֐�
//////////////////////////////////////////////////////////////////////////////
ENoaApp::~ENoaApp( void )
{
}

// �\�����
//////////////////////////////////////////////////////////////////////////////
const char * ENoaApp::SetArgument( int argc, char * argv[] )
{
	int		i ;
	bool	fUsageErr = false ;
	for ( i = 1; i < argc; i ++ )
	{
		const char *	pszArg = argv[i] ;
		if ( pszArg[0] != '/' )
			break ;
		//
		EString	strArg = pszArg + 1 ;
		//
		if ( strArg == "p" )
		{
			m_fPackaging = true ;
		}
		else if ( strArg == "x" )
		{
			m_fPackaging = false ;
		}
		else if ( strArg == "r" )
		{
			m_fAutoRaw = false ;
		}
		else if ( strArg == "pass" )
		{
			if ( i + 1 < argc )
				m_strPassword = argv[++ i] ;
			else
				fUsageErr = true ;
		}
		else if ( strArg == "gp" )
		{
			m_fGenPassword = true ;
		}
		else if ( strArg == "l" )
		{
			m_fGenListFile = true ;
			if ( i + 1 < argc )
				m_strListFile = argv[++ i] ;
			else
				fUsageErr = true ;
		}
		else if ( strArg == "d" )
		{
			m_fSubDirectory = true ;
		}
		else if ( strArg == "raw" )
		{
			m_fPackOption = ERISAArchive::etRaw ;
		}
		else if ( strArg == "erisa" )
		{
			m_fPackOption = ERISAArchive::etERISACode ;
		}
		else if ( strArg == "bshf" )
		{
			m_fPackOption = ERISAArchive::etBSHFCrypt ;
		}
		else if ( strArg == "erisa_bshf" )
		{
			m_fPackOption = ERISAArchive::etERISACrypt ;
		}
		else if ( strArg == "time" )
		{
			m_fTime = true ;
		}
		else if ( strArg == "?" )
		{
			m_fHelp = true ;
		}
		else
		{
			fUsageErr = true ;
		}
	}
	if ( fUsageErr )
	{
		return	"�������s���ł��B/? �ŏ������m�F���Ă��������B\n" ;
	}
	//
	if ( !m_fHelp )
	{
		if ( i + 2 > argc )
		{
			if ( i >= argc )
			{
				return	"�o�̓t�@�C�������w�肳��Ă��܂���B\n" ;
			}
			if ( !m_fPackaging || !m_fGenListFile )
			{
				return	"���̓t�@�C�������w�肳��Ă��܂���B\n" ;
			}
			m_strDstFile = argv[i] ;
		}
		else
		{
			m_strSrcFile = argv[i] ;
			m_strDstFile = argv[i + 1] ;
		}
	}
	//
	return	NULL ;
}

// �A�[�J�C�u��
//////////////////////////////////////////////////////////////////////////////
int ENoaApp::PackFiles( void )
{
	//
	// �t�@�C�����X�g�𐶐�
	//
	ERISAArchiveList	arclist ;
	if ( !m_strSrcFile.IsEmpty() )
	{
		EString	strCurDir ;
		::GetCurrentDirectory( 0x400, strCurDir.GetBuffer(0x400) ) ;
		strCurDir.ReleaseBuffer( ) ;
		//
		if ( m_strSrcFile.Left(1) == "\\" )
		{
			m_strSrcFile = strCurDir.Left(2) + m_strSrcFile ;
		}
		else if ( m_strSrcFile.Middle(1,2) != ":\\" )
		{
			if ( strCurDir.Right(1) != "\\" )
			{
				strCurDir += "\\" ;
			}
			m_strSrcFile = strCurDir + m_strSrcFile ;
		}
		//
		EnumFileList( arclist, m_strSrcFile ) ;
	}
	else
	{
		ESLAssert( !m_strListFile.IsEmpty() ) ;
		ERawFile	file ;
		if ( file.Open( m_strListFile, file.modeRead | file.shareRead ) )
		{
			printf( "%s ���J���܂���ł����B\n", m_strListFile.CharPtr() ) ;
			return	1 ;
		}
		if ( arclist.LoadFileList( file ) )
		{
			printf( "%s ��ǂݍ��߂܂���ł����B\n", m_strListFile.CharPtr() ) ;
			return	1 ;
		}
	}
	//
	// �A�[�J�C�u�t�@�C�����J��
	//
	ERISAArchive::EDirectory	dirRoot ;
	ERISAArchiveList::EDirectory *	pdirList ;
	pdirList = arclist.GetCurrentFileEntries( ) ;
	CreateDirectoryInfo( dirRoot, pdirList ) ;
	ERawFile	dstfile ;
	if ( dstfile.Open( m_strDstFile, dstfile.modeCreate ) )
	{
		printf( "%s ���J���܂���ł����B\n", m_strDstFile.CharPtr() ) ;
		return	1 ;
	}
	ERISAArchive	arcfile ;
	if ( arcfile.Open( &dstfile, &dirRoot ) )
	{
		printf( "�A�[�J�C�u�t�@�C���̃w�b�_�̏����o���Ɏ��s���܂����B\n" ) ;
		return	1 ;
	}
	//
	// �����o��
	//
	int		nResult ;
	nResult = PackDirectory( arcfile, arclist ) ;
	//
	arcfile.Close( ) ;
	dstfile.Close( ) ;
	//
	// �t�@�C�����X�g�������o��
	//
	if ( !m_strSrcFile.IsEmpty() && !m_strListFile.IsEmpty() )
	{
		ERawFile	file ;
		if ( file.Open( m_strListFile, file.modeCreate ) )
		{
			printf( "%s ���J���܂���ł����B\n", m_strListFile.CharPtr() ) ;
			return	1 ;
		}
		if ( arclist.SaveFileList( file ) )
		{
			printf( "�t�@�C�����X�g�̏����o���Ɏ��s���܂����B\n" ) ;
			return	1 ;
		}
		file.Close( ) ;
	}
	//
	return	nResult ;
}

// �W�J
//////////////////////////////////////////////////////////////////////////////
int ENoaApp::DepackFiles( void )
{
	//
	// ���X�g�t�@�C����ǂݍ���
	//
	ERISAArchiveList *	pList = NULL ;
	ERISAArchiveList	arclist ;
	//
	if ( !m_strListFile.IsEmpty() )
	{
		ERawFile	file ;
		if ( file.Open( m_strListFile, file.modeRead | file.shareRead ) )
		{
			printf( "%s �t�@�C�����J���܂���ł����B\n",
								m_strListFile.CharPtr() ) ;
			return	1 ;
		}
		if ( arclist.LoadFileList( file ) )
		{
			printf( "���X�g�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B\n" ) ;
			return	1 ;
		}
		file.Close( ) ;
		pList = &arclist ;
	}
	//
	// �A�[�J�C�u�t�@�C�����J��
	//
	ERawFile	srcfile ;
	if ( srcfile.Open( m_strSrcFile, srcfile.modeRead | srcfile.shareRead ) )
	{
		printf( "%s �t�@�C�����J���܂���ł����B\n", m_strSrcFile.CharPtr() ) ;
		return	1 ;
	}
	ERISAArchive	arcfile ;
	if ( arcfile.Open( &srcfile ) )
	{
		printf( "�A�[�J�C�u�t�@�C���̃I�[�v���Ɏ��s���܂����B\n" ) ;
		return	1 ;
	}
	//
	// �o�͐�f�B���N�g�����擾
	//
	EString	strDstDir = m_strDstFile ;
	if ( strDstDir.Right(1) != "\\" )
	{
		strDstDir += "\\" ;
	}
	//
	int		nResult ;
	nResult = DepackDirectory( arcfile, pList, strDstDir ) ;
	//
	arcfile.Close( ) ;
	srcfile.Close( ) ;
	//
	return	nResult ;
}

// �t�@�C����񋓂��A�t�@�C�����X�g�ɒǉ�
//////////////////////////////////////////////////////////////////////////////
void ENoaApp::EnumFileList
	( ERISAArchiveList & arclist, const char * pszFiles )
{
	EString	strFiles = pszFiles ;
	EString	strDir = strFiles.GetFileDirectoryPart( ) ;
	//
	WIN32_FIND_DATA	wfd ;
	HANDLE	hFind = ::FindFirstFile( pszFiles, &wfd ) ;
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( m_fSubDirectory
					&& ::strcmp( wfd.cFileName, "." )
					&& ::strcmp( wfd.cFileName, ".." ) )
				{
					arclist.AddSubDirectory( wfd.cFileName ) ;
					arclist.DescendDirectory( wfd.cFileName ) ;
					EString	strFilePath = strDir + wfd.cFileName + "\\*.*" ;
					EnumFileList( arclist, strFilePath ) ;
					arclist.AscendDirectory( ) ;
				}
			}
			else
			{
				EString	strFilePath = strDir + wfd.cFileName ;
				EString	strPassword ;
				if ( m_fGenPassword )
				{
					strPassword = GeneratePassword( wfd.cFileName ) ;
				}
				else
				{
					strPassword = m_strPassword ;
				}
				arclist.AddFileEntry
					( strFilePath, m_fPackOption, strPassword ) ;
			}
		}
		while ( ::FindNextFile( hFind, &wfd ) ) ;
		//
		::FindClose( hFind ) ;
	}
}

// �p�X���[�h�𐶐�����
//////////////////////////////////////////////////////////////////////////////
EString ENoaApp::GeneratePassword( const char * pszFileName )
{
	static const char	cTable[97] =
		" !\"#$%&\'()=~|QWERTYUIOP`{ASDFGHJKL+*}ZXCVBNM<>?_"
		"1234567890-^\\qwertyuiop@[asdfghjkl;:]zxcvbnm,./\\" ;
	//
	int	i ;
	for ( i = 0; pszFileName[i] != '\0'; i ++ )
	{
		m_dwRndSeed ^= (((DWORD)pszFileName[i]) & 0xFF) << ((i & 0x03) * 8) ;
	}
	//
	EString	strPass ;
	int		nPassLen = MakeRandom(16) + 8 ;
	char *	pszPass = strPass.GetBuffer( nPassLen ) ;
	for ( i = 0; i < nPassLen; i ++ )
	{
		pszPass[i] = cTable[MakeRandom(96)] ;
	}
	strPass.ReleaseBuffer( nPassLen ) ;
	//
	return	strPass ;
}

// �����𐶐�����
//////////////////////////////////////////////////////////////////////////////
int ENoaApp::MakeRandom( int nMax )
{
	m_dwRndSeed = m_dwRndSeed * 5 + 0xA5BDE327 ;
	return	(m_dwRndSeed >> 16) % nMax ;
}

// �f�B���N�g���������o��
//////////////////////////////////////////////////////////////////////////////
int ENoaApp::PackDirectory
	( ERISAArchive & arcfile, ERISAArchiveList & arclist )
{
	ERISAArchiveList::EDirectory * pdir = arclist.GetCurrentFileEntries( ) ;
	ESLAssert( pdir != NULL ) ;
	//
	for ( unsigned int i = 0; i < pdir->GetSize(); i ++ )
	{
		//
		// �t�@�C���G���g�����擾����
		//
		ERISAArchiveList::EFileEntry *	pfile = pdir->GetAt(i) ;
		ESLAssert( pfile != NULL ) ;
		//
		if ( pfile->m_dwAttribute & ERISAArchive::attrDirectory )
		{
			//
			// �T�u�f�B���N�g��
			//
			ERISAArchiveList::EDirectory *	pdirSub = pfile->m_pSubDir ;
			ESLAssert( pdirSub != NULL ) ;
			ESLVerify( !arclist.DescendDirectory( pfile->m_strFileName ) ) ;
			ERISAArchive::EDirectory	dirFiles ;
			CreateDirectoryInfo( dirFiles, arclist.GetCurrentFileEntries() ) ;
			if ( arcfile.DescendDirectory( pfile->m_strFileName, &dirFiles ) )
			{
				printf( "%s �f�B���N�g���̍쐬�Ɏ��s���܂����B\n",
						arclist.GetCurrentDirectoryPath().CharPtr() ) ;
				return	1 ;
			}
			//
			int	nResult = PackDirectory( arcfile, arclist ) ;
			//
			ESLVerify( !arcfile.AscendDirectory() ) ;
			ESLVerify( !arclist.AscendDirectory() ) ;
			//
			if ( nResult != 0 )
			{
				return	nResult ;
			}
		}
		else
		{
			LARGE_INTEGER	liStart, liEnd, liFreq ;
			//
			// �t�@�C���G���g��
			//
			ERawFile	file ;
			if ( file.Open( pfile->m_strFileName,
								file.modeRead | file.shareRead ) )
			{
				printf( "%s ���J���܂���ł����B\n",
							pfile->m_strFileName.CharPtr() ) ;
				return	1 ;
			}
			UINT64	nFilePos = arcfile.GetLargePosition( ) ;
			if ( arcfile.DescendFile
				( pfile->m_strFileName.GetFileNamePart(),
					pfile->m_strPassword, ERISAArchive::otStream ) )
			{
				printf( "%s ���t�@�C���G���g���ɒǉ��ł��܂���ł����B\n",
									pfile->m_strFileName.GetFileNamePart() ) ;
				return	1 ;
			}
			//
			// �t�@�C�����e�������o��
			//
			EString	strFileName = pfile->m_strFileName.GetFileNamePart( ) ;
			UINT64	nTotalBytes = file.GetLargeLength( ) ;
			UINT64	nWrittenBytes = 0 ;
			//
			::QueryPerformanceCounter( &liStart ) ;
			while ( nWrittenBytes < nTotalBytes )
			{
				if ( !m_fTime )
				{
					printf( "%s : �����o�����ł��c %I64d/%I64d [bytes]\r",
							strFileName.CharPtr(), nWrittenBytes, nTotalBytes ) ;
				}
				//
				UINT64	nBufSize = nTotalBytes - nWrittenBytes ;
				if ( nBufSize > 0x4000 )
				{
					nBufSize = 0x4000 ;
				}
				DWORD	dwBufSize = (DWORD) nBufSize ;
				EStreamBuffer	buf ;
				void *	ptrBuf = buf.PutBuffer( dwBufSize ) ;
				DWORD	dwReadBytes = file.Read( ptrBuf, dwBufSize ) ;
				DWORD	dwCurrentBytes = arcfile.Write( ptrBuf, dwReadBytes ) ;
				//
				if ( dwCurrentBytes < dwBufSize )
				{
					printf( "\n�t�@�C�����삪���f����܂����B\n" ) ;
					return	1 ;
				}
				nWrittenBytes += dwCurrentBytes ;
			}
			::QueryPerformanceCounter( &liEnd ) ;
			arcfile.AscendFile( ) ;
			//
			UINT64	nCompressedSize =
				arcfile.GetLargePosition() - nFilePos - 0x10 ;
			if ( m_fAutoRaw && (nCompressedSize >= nTotalBytes)
				&& ((pfile->m_dwEncodeType == ERISAArchive::etERISACode)
					|| (pfile->m_dwEncodeType == ERISAArchive::etERISACrypt)) )
			{
				//
				// �����k����
				//
				arcfile.SeekLarge( nFilePos, ESLFileObject::FromBegin ) ;
				arcfile.SetEndOfFile( ) ;
				file.Seek( 0, ESLFileObject::FromBegin ) ;
				//
				ERISAArchive::FILE_INFO *	pinfo ;
				pinfo = arcfile.ReferFileEntries().GetFileInfo( strFileName ) ;
				ESLAssert( pinfo != NULL ) ;
				//
				if ( pinfo->dwEncodeType == ERISAArchive::etERISACode )
					pinfo->dwEncodeType = ERISAArchive::etRaw ;
				else if ( pinfo->dwEncodeType == ERISAArchive::etERISACrypt )
					pinfo->dwEncodeType = ERISAArchive::etBSHFCrypt ;
				//
				if ( arcfile.DescendFile
					( strFileName, pfile->m_strPassword,
										ERISAArchive::otStream ) )
				{
					printf( "%s ���t�@�C���G���g���ɒǉ��ł��܂���ł����B\n",
														strFileName.CharPtr() ) ;
					return	1 ;
				}
				//
				nWrittenBytes = 0 ;
				while ( nWrittenBytes < nTotalBytes )
				{
					UINT64	nBufSize = nTotalBytes - nWrittenBytes ;
					if ( nBufSize > 0x4000 )
					{
						nBufSize = 0x4000 ;
					}
					DWORD	dwBufSize = (DWORD) nBufSize ;
					EStreamBuffer	buf ;
					void *	ptrBuf = buf.PutBuffer( dwBufSize ) ;
					DWORD	dwReadBytes = file.Read( ptrBuf, dwBufSize ) ;
					DWORD	dwCurrentBytes = arcfile.Write( ptrBuf, dwReadBytes ) ;
					//
					if ( dwCurrentBytes < dwBufSize )
					{
						printf( "\n�t�@�C�����삪���f����܂����B\n" ) ;
						return	1 ;
					}
					nWrittenBytes += dwCurrentBytes ;
				}
				arcfile.AscendFile( ) ;
			}
			file.Close( ) ;
			//
			if ( !m_fTime )
			{
				printf( "%s : �����o���������܂��� %I64d/%I64d [bytes]\n",
						strFileName.CharPtr(), nWrittenBytes, nTotalBytes ) ;
			}
			else
			{
				::QueryPerformanceFrequency( &liFreq ) ;
				//
				printf( "%s : �����o���������܂��� : %.2f [msec]\n",
						strFileName.CharPtr(),
						(double) (liEnd.QuadPart - liStart.QuadPart)
												* 1000 / liFreq.QuadPart ) ;
			}
		}
	}
	return	0 ;
}

// �t�@�C�����X�g���珑���o���p�f�B���N�g�����𐶐�����
//////////////////////////////////////////////////////////////////////////////
void ENoaApp::CreateDirectoryInfo
	( ERISAArchive::EDirectory & dirFiles,
		ERISAArchiveList::EDirectory * pdir )
{
	ESLAssert( pdir != NULL ) ;
	//
	for ( unsigned int i = 0; i < pdir->GetSize(); i ++ )
	{
		ERISAArchiveList::EFileEntry *	pfile = pdir->GetAt(i) ;
		ESLAssert( pfile != NULL ) ;
		//
		SYSTEMTIME	stLastWrite ;
		ERISAArchive::FILE_TIME	ftime = { 0, 0, 0, 0, 0, 0, 0 } ;
		if ( !(pfile->m_dwAttribute & ERISAArchive::attrDirectory) )
		{
			ERawFile	file ;
			if ( file.Open( pfile->m_strFileName, 0 ) )
			{
				printf( "%s ���J���܂���ł����B\n", pfile->m_strFileName.CharPtr() ) ;
				continue ;
			}
			if ( !file.GetFileTime( NULL, NULL, &stLastWrite ) )
			{
				ftime.nSecond = (BYTE) stLastWrite.wSecond ;
				ftime.nMinute = (BYTE) stLastWrite.wMinute ;
				ftime.nHour = (BYTE) stLastWrite.wHour ;
				ftime.nWeek = (BYTE) stLastWrite.wDayOfWeek ;
				ftime.nDay = (BYTE) stLastWrite.wDay ;
				ftime.nMonth = (BYTE) stLastWrite.wMonth ;
				ftime.nYear = stLastWrite.wYear ;
			}
			file.Close( ) ;
		}
		//
		dirFiles.AddFileEntry
			( pfile->m_strFileName.GetFileNamePart(),
				pfile->m_dwAttribute, pfile->m_dwEncodeType, &ftime ) ;
	}
}

// �f�B���N�g���ɏ����o��
//////////////////////////////////////////////////////////////////////////////
int ENoaApp::DepackDirectory
	( ERISAArchive & arcfile,
		ERISAArchiveList * arclist, const EString & strBaseDir )
{
	//
	// �f�B���N�g���G���g�����擾
	//
	ERISAArchive::EDirectory	dirFiles ;
	arcfile.GetFileEntries( dirFiles ) ;
	//
	ERISAArchiveList::EDirectory *	pdir = NULL ;
	if ( arclist != NULL )
	{
		pdir = arclist->GetCurrentFileEntries( ) ;
		ESLAssert( pdir != NULL ) ;
	}
	//
	for ( unsigned int i = 0; i < dirFiles.GetSize(); i ++ )
	{
		//
		// �t�@�C���G���g�����擾
		//
		ERISAArchive::FILE_INFO *	pinfo = dirFiles.GetAt( i ) ;
		ESLAssert( pinfo != NULL ) ;
		//
		if ( pinfo->dwAttribute & ERISAArchive::attrDirectory )
		{
			//
			// �T�u�f�B���N�g��
			//
			if ( arcfile.DescendDirectory( pinfo->ptrFileName ) )
			{
				printf( "%s �T�u�f�B���N�g�����J���܂���ł����B\n",
												pinfo->ptrFileName ) ;
				return	1 ;
			}
			bool	fListSubDir = false ;
			if ( arclist != NULL )
			{
				fListSubDir =
					!arclist->DescendDirectory( pinfo->ptrFileName ) ;
			}
			//
			::CreateDirectory( strBaseDir + pinfo->ptrFileName, NULL ) ;
			EString	strSubDir = strBaseDir + pinfo->ptrFileName + "\\" ;
			//
			int		nResult ;
			nResult = DepackDirectory
				( arcfile, (fListSubDir ? arclist : NULL), strSubDir ) ;
			//
			if ( fListSubDir )
			{
				arclist->AscendDirectory( ) ;
			}
			arcfile.AscendDirectory( ) ;
		}
		else
		{
			LARGE_INTEGER	liStart, liEnd, liFreq ;
			//
			// �t�@�C���W�J
			//
			const char *	pszPassword = NULL ;
			if ( pdir != NULL )
			{
				ERISAArchiveList::EFileEntry *	pfile ;
				pfile = SearchFile( pdir, pinfo->ptrFileName ) ;
				if ( pfile != NULL )
				{
					pszPassword = pfile->m_strPassword ;
				}
			}
			else
			{
				pszPassword = m_strPassword ;
			}
			//
			ERawFile	dstfile ;
			EString		strDstPath = strBaseDir + pinfo->ptrFileName ;
			if ( dstfile.Open( strDstPath, dstfile.modeCreate ) )
			{
				printf( "%s �t�@�C�����J���܂���ł����B\n", strDstPath.CharPtr() ) ;
				return	1 ;
			}
			//
			::QueryPerformanceCounter( &liStart ) ;
			if ( arcfile.DescendFile
				( pinfo->ptrFileName, pszPassword, ERISAArchive::otStream ) )
			{
				printf( "\n%s �t�@�C�����J���܂���ł����B\n", pinfo->ptrFileName ) ;
				dstfile.Close( ) ;
				::DeleteFile( strDstPath ) ;
				return	1 ;
			}
			//
			UINT64	nTotalBytes = arcfile.GetLargeLength( ) ;
			UINT64	nWrittenBytes = 0 ;
			//
			while ( nWrittenBytes < nTotalBytes )
			{
				if ( !m_fTime )
				{
					printf( "%s : �����o���Ă��܂��c %d/%d [bytes]\r",
							pinfo->ptrFileName, nWrittenBytes, nTotalBytes ) ;
				}
				//
				UINT64	nBufBytes = nTotalBytes - nWrittenBytes ;
				if ( nBufBytes > 0x4000 )
				{
					nBufBytes = 0x4000 ;
				}
				DWORD	dwBufBytes = (DWORD) nBufBytes ;
				EStreamBuffer	buf ;
				void *	ptrBuf = buf.PutBuffer( dwBufBytes ) ;
				//
				DWORD	dwReadBytes = arcfile.Read( ptrBuf, dwBufBytes ) ;
				DWORD	dwCurrentBytes = dstfile.Write( ptrBuf, dwReadBytes ) ;
				//
				if ( dwCurrentBytes < dwReadBytes )
				{
					printf( "\n�t�@�C���̏����o�������f����܂����B\n" ) ;
					return	1 ;
				}
				if ( dwCurrentBytes < dwBufBytes )
				{
					break ;
				}
				nWrittenBytes += dwCurrentBytes ;
			}
			if ( nWrittenBytes == nTotalBytes )
			{
				if ( !m_fTime )
				{
					printf( "%s : �����o���������܂��� %I64d/%I64d [bytes]\n",
							pinfo->ptrFileName, nWrittenBytes, nTotalBytes ) ;
				}
				else
				{
					::QueryPerformanceCounter( &liEnd ) ;
					::QueryPerformanceFrequency( &liFreq ) ;
					//
					printf( "%s : �����o���������܂��� : %.2f [msec]\n",
							pinfo->ptrFileName,
							(double) (liEnd.QuadPart - liStart.QuadPart)
													* 1000 / liFreq.QuadPart ) ;
				}
			}
			//
			// �^�C���X�^���v�ݒ�
			//
			SYSTEMTIME	stLastWrite ;
			stLastWrite.wSecond = pinfo->ftFileTime.nSecond ;
			stLastWrite.wMinute = pinfo->ftFileTime.nMinute ;
			stLastWrite.wHour = pinfo->ftFileTime.nHour ;
			stLastWrite.wDayOfWeek = pinfo->ftFileTime.nWeek ;
			stLastWrite.wDay = pinfo->ftFileTime.nDay ;
			stLastWrite.wMonth = pinfo->ftFileTime.nMonth ;
			stLastWrite.wYear = pinfo->ftFileTime.nYear ;
			dstfile.SetFileTime( NULL, NULL, &stLastWrite ) ;
			dstfile.Close( ) ;
			//
			if ( arcfile.AscendFile() || (nWrittenBytes < nTotalBytes) )
			{
				if ( (pinfo->dwEncodeType == ERISAArchive::etERISACode)
					|| (pinfo->dwEncodeType == ERISAArchive::etERISACrypt) )
				{
					printf( "\n%s �t�@�C���͐���ł͂���܂���B\n", pinfo->ptrFileName ) ;
					printf( "�p�X���[�h���ԈႦ�Ă���\��������܂��B\n" ) ;
				}
				else
				{
					printf( "\n%s �t�@�C���͐���ł͂���܂���B\n", pinfo->ptrFileName ) ;
				}
				::DeleteFile( strDstPath ) ;
				return	1 ;
			}
		}
	}
	return	0 ;
}

// �t�@�C���G���g��������
//////////////////////////////////////////////////////////////////////////////
ERISAArchiveList::EFileEntry * ENoaApp::SearchFile
	( ERISAArchiveList::EDirectory * pdir, const char * pszFileName )
{
	for ( unsigned int i = 0; i < pdir->GetSize(); i ++ )
	{
		ERISAArchiveList::EFileEntry *	pfile = pdir->GetAt( i ) ;
		ESLAssert( pfile != NULL ) ;
		//
		EString	strFileName = pfile->m_strFileName.GetFileNamePart( ) ;
		if ( !strFileName.CompareNoCase( pszFileName ) )
		{
			return	pfile ;
		}
	}
	return	NULL ;
}

