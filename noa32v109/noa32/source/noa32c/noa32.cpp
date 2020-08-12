
/*****************************************************************************
                    NOA32 アーカイバ コンソール版
 *****************************************************************************/


#include "noa32.h"


//
// メイン関数
//////////////////////////////////////////////////////////////////////////////
int main( int argc, char * argv[] )
{
	::eriInitializeLibrary( ) ;
	//
	// 構文解析
	//
	ENoaApp	app ;
	const char *	pszErr ;
	pszErr = app.SetArgument( argc, argv ) ;
	//
	if ( !app.m_fNologo )
	{
		static const char *	pszLogo =
			"乃亞アーカイバ version 1.08\n"
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
			"書式 : noa32c [options] [<source-file>] <destination>\n"
			"\n"
			"options;\n"
			"\t/p               : アーカイブ化します\n"
			"\t/x               : アーカイブを展開します\n"
			"\t/r               : 自動的に圧縮を無効化しない\n"
			"\t/pass <password> : パスワードを設定します\n"
			"\t/gp              : アーカイブ化の際、自動的にパスワードを生成します\n"
			"\t/l <list-file>   : リストファイルを指定します\n"
			"\t/d               : サブディレクトリもアーカイブ化します\n"
			"\t/raw             : 生データのままアーカイブ化します\n"
			"\t/erisa           : 圧縮してアーカイブ化します\n"
			"\t/bshf            : 暗号化してアーカイブ化します\n"
			"\t/erisa_bshf      : 圧縮＆暗号化してアーカイブ化します\n"
			"\t/time            : 圧縮・展開に要した時間を計測します。\n"
			"\n"
			"source-file;\n"
			"　アーカイブファイル名、又はアーカイブ化するファイル名を指定します。\n"
			"　アーカイブ化の際には、ワールドカードを使用できます。\n"
			"　アーカイブ化の際に /l を指定していると、リストファイルを使ってアーカイブ化します。\n"
			"\n"
			"destination;\n"
			"　ファイルの展開先ディレクトリ、又はアーカイブファイル名を指定します。\n\n" ;
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
// アプリケーションクラス
//////////////////////////////////////////////////////////////////////////////

// 構築関数
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

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ENoaApp::~ENoaApp( void )
{
}

// 構文解析
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
		return	"書式が不正です。/? で書式を確認してください。\n" ;
	}
	//
	if ( !m_fHelp )
	{
		if ( i + 2 > argc )
		{
			if ( i >= argc )
			{
				return	"出力ファイル名が指定されていません。\n" ;
			}
			if ( !m_fPackaging || !m_fGenListFile )
			{
				return	"入力ファイル名が指定されていません。\n" ;
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

// アーカイブ化
//////////////////////////////////////////////////////////////////////////////
int ENoaApp::PackFiles( void )
{
	//
	// ファイルリストを生成
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
			printf( "%s を開けませんでした。\n", m_strListFile.CharPtr() ) ;
			return	1 ;
		}
		if ( arclist.LoadFileList( file ) )
		{
			printf( "%s を読み込めませんでした。\n", m_strListFile.CharPtr() ) ;
			return	1 ;
		}
	}
	//
	// アーカイブファイルを開く
	//
	ERISAArchive::EDirectory	dirRoot ;
	ERISAArchiveList::EDirectory *	pdirList ;
	pdirList = arclist.GetCurrentFileEntries( ) ;
	CreateDirectoryInfo( dirRoot, pdirList ) ;
	ERawFile	dstfile ;
	if ( dstfile.Open( m_strDstFile, dstfile.modeCreate ) )
	{
		printf( "%s を開けませんでした。\n", m_strDstFile.CharPtr() ) ;
		return	1 ;
	}
	ERISAArchive	arcfile ;
	if ( arcfile.Open( &dstfile, &dirRoot ) )
	{
		printf( "アーカイブファイルのヘッダの書き出しに失敗しました。\n" ) ;
		return	1 ;
	}
	//
	// 書き出す
	//
	int		nResult ;
	nResult = PackDirectory( arcfile, arclist ) ;
	//
	arcfile.Close( ) ;
	dstfile.Close( ) ;
	//
	// ファイルリストを書き出す
	//
	if ( !m_strSrcFile.IsEmpty() && !m_strListFile.IsEmpty() )
	{
		ERawFile	file ;
		if ( file.Open( m_strListFile, file.modeCreate ) )
		{
			printf( "%s を開けませんでした。\n", m_strListFile.CharPtr() ) ;
			return	1 ;
		}
		if ( arclist.SaveFileList( file ) )
		{
			printf( "ファイルリストの書き出しに失敗しました。\n" ) ;
			return	1 ;
		}
		file.Close( ) ;
	}
	//
	return	nResult ;
}

// 展開
//////////////////////////////////////////////////////////////////////////////
int ENoaApp::DepackFiles( void )
{
	//
	// リストファイルを読み込む
	//
	ERISAArchiveList *	pList = NULL ;
	ERISAArchiveList	arclist ;
	//
	if ( !m_strListFile.IsEmpty() )
	{
		ERawFile	file ;
		if ( file.Open( m_strListFile, file.modeRead | file.shareRead ) )
		{
			printf( "%s ファイルを開けませんでした。\n",
								m_strListFile.CharPtr() ) ;
			return	1 ;
		}
		if ( arclist.LoadFileList( file ) )
		{
			printf( "リストファイルの読み込みに失敗しました。\n" ) ;
			return	1 ;
		}
		file.Close( ) ;
		pList = &arclist ;
	}
	//
	// アーカイブファイルを開く
	//
	ERawFile	srcfile ;
	if ( srcfile.Open( m_strSrcFile, srcfile.modeRead | srcfile.shareRead ) )
	{
		printf( "%s ファイルを開けませんでした。\n", m_strSrcFile.CharPtr() ) ;
		return	1 ;
	}
	ERISAArchive	arcfile ;
	if ( arcfile.Open( &srcfile ) )
	{
		printf( "アーカイブファイルのオープンに失敗しました。\n" ) ;
		return	1 ;
	}
	//
	// 出力先ディレクトリを取得
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

// ファイルを列挙し、ファイルリストに追加
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

// パスワードを生成する
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

// 乱数を生成する
//////////////////////////////////////////////////////////////////////////////
int ENoaApp::MakeRandom( int nMax )
{
	m_dwRndSeed = m_dwRndSeed * 5 + 0xA5BDE327 ;
	return	(m_dwRndSeed >> 16) % nMax ;
}

// ディレクトリを書き出す
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
		// ファイルエントリを取得する
		//
		ERISAArchiveList::EFileEntry *	pfile = pdir->GetAt(i) ;
		ESLAssert( pfile != NULL ) ;
		//
		if ( pfile->m_dwAttribute & ERISAArchive::attrDirectory )
		{
			//
			// サブディレクトリ
			//
			ERISAArchiveList::EDirectory *	pdirSub = pfile->m_pSubDir ;
			ESLAssert( pdirSub != NULL ) ;
			ESLVerify( !arclist.DescendDirectory( pfile->m_strFileName ) ) ;
			ERISAArchive::EDirectory	dirFiles ;
			CreateDirectoryInfo( dirFiles, arclist.GetCurrentFileEntries() ) ;
			if ( arcfile.DescendDirectory( pfile->m_strFileName, &dirFiles ) )
			{
				printf( "%s ディレクトリの作成に失敗しました。\n",
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
			// ファイルエントリ
			//
			ERawFile	file ;
			if ( file.Open( pfile->m_strFileName,
								file.modeRead | file.shareRead ) )
			{
				printf( "%s を開けませんでした。\n",
							pfile->m_strFileName.CharPtr() ) ;
				return	1 ;
			}
			UINT64	nFilePos = arcfile.GetLargePosition( ) ;
			if ( arcfile.DescendFile
				( pfile->m_strFileName.GetFileNamePart(),
					pfile->m_strPassword, ERISAArchive::otStream ) )
			{
				printf( "%s をファイルエントリに追加できませんでした。\n",
									pfile->m_strFileName.GetFileNamePart() ) ;
				return	1 ;
			}
			//
			// ファイル内容を書き出す
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
					printf( "%s : 書き出し中です… %I64d/%I64d [bytes]\r",
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
					printf( "\nファイル操作が中断されました。\n" ) ;
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
				// 無圧縮処理
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
					printf( "%s をファイルエントリに追加できませんでした。\n",
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
						printf( "\nファイル操作が中断されました。\n" ) ;
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
				printf( "%s : 書き出し完了しました %I64d/%I64d [bytes]\n",
						strFileName.CharPtr(), nWrittenBytes, nTotalBytes ) ;
			}
			else
			{
				::QueryPerformanceFrequency( &liFreq ) ;
				//
				printf( "%s : 書き出し完了しました : %.2f [msec]\n",
						strFileName.CharPtr(),
						(double) (liEnd.QuadPart - liStart.QuadPart)
												* 1000 / liFreq.QuadPart ) ;
			}
		}
	}
	return	0 ;
}

// ファイルリストから書き出し用ディレクトリ情報を生成する
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
				printf( "%s を開けませんでした。\n", pfile->m_strFileName.CharPtr() ) ;
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

// ディレクトリに書き出す
//////////////////////////////////////////////////////////////////////////////
int ENoaApp::DepackDirectory
	( ERISAArchive & arcfile,
		ERISAArchiveList * arclist, const EString & strBaseDir )
{
	//
	// ディレクトリエントリを取得
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
		// ファイルエントリを取得
		//
		ERISAArchive::FILE_INFO *	pinfo = dirFiles.GetAt( i ) ;
		ESLAssert( pinfo != NULL ) ;
		//
		if ( pinfo->dwAttribute & ERISAArchive::attrDirectory )
		{
			//
			// サブディレクトリ
			//
			if ( arcfile.DescendDirectory( pinfo->ptrFileName ) )
			{
				printf( "%s サブディレクトリを開けませんでした。\n",
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
			// ファイル展開
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
				printf( "%s ファイルを開けませんでした。\n", strDstPath.CharPtr() ) ;
				return	1 ;
			}
			//
			::QueryPerformanceCounter( &liStart ) ;
			if ( arcfile.DescendFile
				( pinfo->ptrFileName, pszPassword, ERISAArchive::otStream ) )
			{
				printf( "\n%s ファイルを開けませんでした。\n", pinfo->ptrFileName ) ;
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
					printf( "%s : 書き出しています… %d/%d [bytes]\r",
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
					printf( "\nファイルの書き出しが中断されました。\n" ) ;
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
					printf( "%s : 書き出し完了しました %I64d/%I64d [bytes]\n",
							pinfo->ptrFileName, nWrittenBytes, nTotalBytes ) ;
				}
				else
				{
					::QueryPerformanceCounter( &liEnd ) ;
					::QueryPerformanceFrequency( &liFreq ) ;
					//
					printf( "%s : 書き出し完了しました : %.2f [msec]\n",
							pinfo->ptrFileName,
							(double) (liEnd.QuadPart - liStart.QuadPart)
													* 1000 / liFreq.QuadPart ) ;
				}
			}
			//
			// タイムスタンプ設定
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
					printf( "\n%s ファイルは正常ではありません。\n", pinfo->ptrFileName ) ;
					printf( "パスワードが間違えている可能性があります。\n" ) ;
				}
				else
				{
					printf( "\n%s ファイルは正常ではありません。\n", pinfo->ptrFileName ) ;
				}
				::DeleteFile( strDstPath ) ;
				return	1 ;
			}
		}
	}
	return	0 ;
}

// ファイルエントリを検索
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

