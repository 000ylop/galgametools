

#include <gls.h>
#include <linkgls.h>
#include <stdio.h>



//////////////////////////////////////////////////////////////////////////////
// アーカイバクラス
//////////////////////////////////////////////////////////////////////////////

class	ENoaArchive	: public	ERISAArchive
{
public:
	// 構築関数
	ENoaArchive( bool fTime = false ) ;
	// 消滅関数
	virtual ~ENoaArchive( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ENoaArchive, ERISAArchive )

protected:
	bool	m_fTime ;
	EString	m_strFileName ;

public:
	// ファイルを開く
	int DescendFile
		( const char * pszFileName, const char * pszPassword = NULL ) ;

protected:
	// 展開・復号の進行状況を通知する
	virtual ESLError OnProcessFileData( DWORD dwCurrent, DWORD dwTotal ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// アプリケーションクラス
//////////////////////////////////////////////////////////////////////////////

class	ENoaApp
{
public:
	bool	m_fNologo ;			// 見出しを表示するか
	bool	m_fHelp ;			// 書式を表示する
	bool	m_fPackaging ;		// アーカイブ化する
	bool	m_fAutoRaw ;		// 圧縮率が悪いと自動的に無圧縮にする
	bool	m_fGenPassword ;	// パスワードを生成する
	bool	m_fSubDirectory ;	// サブディレクトリもアーカイブ化するか？
	bool	m_fGenListFile ;	// リストファイルを生成する
	bool	m_fTime ;			// 時間を計測する
	int		m_fPackOption ;		// アーカイブ化のオプション
	EString	m_strPassword ;		// パスワード
	EString	m_strListFile ;		// リストファイル名
	EString	m_strSrcFile ;		// 入力ファイル名
	EString	m_strDstFile ;		// 出力ファイル名
	DWORD	m_dwRndSeed ;		// 乱数の種

public:
	// 構築関数
	ENoaApp( void ) ;
	// 消滅関数
	~ENoaApp( void ) ;
	// 構文解析
	const char * SetArgument( int argc, char * argv[] ) ;
	// アーカイブ化
	int PackFiles( void ) ;
	// 展開
	int DepackFiles( void ) ;

protected:
	// ファイルを列挙し、ファイルリストに追加
	void EnumFileList
		( ERISAArchiveList & arclist, const char * pszFiles ) ;
	// パスワードを生成する
	EString GeneratePassword( const char * pszFileName ) ;
	// 乱数を生成する
	int MakeRandom( int nMax ) ;

	// ディレクトリを書き出す
	int PackDirectory
		( ERISAArchive & arcfile, ERISAArchiveList & arclist ) ;
	// ファイルリストから書き出し用ディレクトリ情報を生成する
	void CreateDirectoryInfo
		( ERISAArchive::EDirectory & dirFiles,
			ERISAArchiveList::EDirectory * pdir ) ;

	// ディレクトリに書き出す
	int DepackDirectory
		( ERISAArchive & arcfile,
			ERISAArchiveList * arclist, const EString & strBaseDir ) ;
	// ファイルエントリを検索
	ERISAArchiveList::EFileEntry * SearchFile
		( ERISAArchiveList::EDirectory * pdir, const char * pszFileName ) ;

} ;

