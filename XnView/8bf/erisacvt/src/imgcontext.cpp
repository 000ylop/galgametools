
/*****************************************************************************
                          ERISA �摜�R���o�[�^�[
 *****************************************************************************/


#include "erisacvt.h"


//////////////////////////////////////////////////////////////////////////////
// �摜�o�b�t�@
//////////////////////////////////////////////////////////////////////////////

// �N���X���
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSImage, ECSResource )

// �\�z�֐�
//////////////////////////////////////////////////////////////////////////////
ECSImage::ECSImage( void )
{
	m_fOwnRsrc = rofNothing ;
	m_pRsrc = &m_imgBuf ;
}

// ���Ŋ֐�
//////////////////////////////////////////////////////////////////////////////
ECSImage::~ECSImage( void )
{
}

// �I�u�W�F�N�g�̌^�����擾����
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSImage::GetTypeName( void ) const
{
	return	L"Image" ;
}

// �I�u�W�F�N�g�𕡐�
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSImage::Duplicate( void )
{
	ECSImage *	pImage = new ECSImage ;
	PEGL_IMAGE_INFO	pInfo = m_imgBuf ;
	if ( pInfo != NULL )
	{
		::eglAddImageBufferRef( pInfo ) ;
		pImage->m_imgBuf.AddFrame( pInfo ) ;
		pImage->m_imgBuf.GetSequenceTable().SetAt( 0, 0 ) ;
		pImage->m_imgBuf.SetCurrentSequence( 0 ) ;
	}
	pImage->m_imgBuf.SetHotSpot( m_imgBuf.GetHotSpot() ) ;
	pImage->m_intIndex.m_varInt = m_intIndex.m_varInt ;
	pImage->m_strName.m_varStr = m_strName.m_varStr ;
	pImage->m_strFileName.m_varStr = m_strFileName.m_varStr ;
	return	pImage ;
}

// �I�u�W�F�N�g����
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImage::Move( ECSContext & context, ECSObject * obj )
{
	ECSImage *	pSrcImage =
		ESLTypeCast<ECSImage>( obj->GetObjectEntity() ) ;
	if ( pSrcImage == NULL )
	{
		return	ESLErrorMsg
			( "Image �̑������ Image �I�u�W�F�N�g�ł͂���܂���B" ) ;
	}
	PEGL_IMAGE_INFO	pInfo = pSrcImage->m_imgBuf ;
	if ( pInfo != NULL )
	{
		::eglAddImageBufferRef( pInfo ) ;
		m_imgBuf.DeleteImage() ;
		m_imgBuf.AddFrame( pInfo ) ;
		m_imgBuf.GetSequenceTable().SetAt( 0, 0 ) ;
		m_imgBuf.SetCurrentSequence( 0 ) ;
	}
	m_imgBuf.SetHotSpot( pSrcImage->m_imgBuf.GetHotSpot() ) ;
	m_intIndex.m_varInt = pSrcImage->m_intIndex.m_varInt ;
	m_strName.m_varStr = pSrcImage->m_strName.m_varStr ;
	m_strFileName.m_varStr = pSrcImage->m_strFileName.m_varStr ;
	delete	obj ;
	return	eslErrSuccess ;
}

// �����o�ϐ��C���f�b�N�X�擾
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImage::GetVariableIndex( int & nIndex, ECSObject & obj )
{
	if ( obj.m_vtType != csvtString )
	{
		return	ESLErrorMsg
			( "Image �^�̃����o�͕�����ŎQ�Ƃ��Ȃ���΂Ȃ�܂���B" ) ;
	}
	ECSWideString &	wstrName = ((ECSString&)obj).m_varStr ;
	if ( wstrName == L"index" )
	{
		nIndex = 0 ;
	}
	else if ( wstrName == L"name" )
	{
		nIndex = 1 ;
	}
	else if ( wstrName == L"filename" )
	{
		nIndex = 2 ;
	}
	else
	{
		return	ESLErrorMsg( "Image �^�̒�`����Ă��Ȃ������o�Q�Ƃł��B" ) ;
	}
	return	eslErrSuccess ;
}

// �����o�ϐ��擾
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSImage::GetVariableAt( int nIndex )
{
	switch ( nIndex )
	{
	case	0:
		return	&m_intIndex ;
	case	1:
		return	&m_strName ;
	case	2:
		return	&m_strFileName ;
	}
	return	NULL ;
}

// �����o�ϐ��ݒ�
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSImage::SetVariableAt( int nIndex, ECSObject * obj )
{
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// �摜�R���e�L�X�g�o�b�t�@
//////////////////////////////////////////////////////////////////////////////

// �N���X���
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSImageContext, ECSArray )

const wchar_t *	ECSImageContext::pwszFuncName[7] =
{
	L"load", L"save", L"cut", L"arrange", L"animation", L"merge", NULL
} ;
const ECSImageContext::PFUNC_CALL
	ECSImageContext::m_pfnCallFunc[6] =
{
	&ECSImageContext::Call_load,
	&ECSImageContext::Call_save,
	&ECSImageContext::Call_cut,
	&ECSImageContext::Call_arrange,
	&ECSImageContext::Call_animation,
	&ECSImageContext::Call_merge
} ;

// �\�z�֐�
//////////////////////////////////////////////////////////////////////////////
ECSImageContext::ECSImageContext( void )
{
	m_sizeImage.m_wstrTag = L"Size" ;
	m_sizeImage.SetMemberAsInt( L"w", 0 ) ;
	m_sizeImage.SetMemberAsInt( L"h", 0 ) ;
	m_ptHotSpot.m_wstrTag = L"Point" ;
	m_ptHotSpot.SetMemberAsInt( L"x", 0 ) ;
	m_ptHotSpot.SetMemberAsInt( L"y", 0 ) ;
	//
	for ( int i = 0; pwszFuncName[i]; i ++ )
	{
		m_staFuncName.Add( new ECSWideString( pwszFuncName[i] ) ) ;
	}
}

// ���Ŋ֐�
//////////////////////////////////////////////////////////////////////////////
ECSImageContext::~ECSImageContext( void )
{
}

// �摜�R���e�L�X�g����
//////////////////////////////////////////////////////////////////////////////
void ECSImageContext::DeleteContext( void )
{
	m_varArray.RemoveAll( ) ;
	m_lstSequence.RemoveAll( ) ;
	m_intDuration.m_varInt = 0 ;
	m_strFileName.m_varStr.FreeString( ) ;
	m_sizeImage.SetMemberAsInt( L"w", 0 ) ;
	m_sizeImage.SetMemberAsInt( L"h", 0 ) ;
	m_ptHotSpot.SetMemberAsInt( L"x", 0 ) ;
	m_ptHotSpot.SetMemberAsInt( L"y", 0 ) ;
}

// �摜�̓ǂݍ���
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::LoadImageFile
	( const wchar_t * pwszFileName, int fBlendMode, int fMaskMode )
{
	EGLMediaLoader	imgBuf ;
	EString			strSrcFile = pwszFileName ;
	EWideString		wstrFileName = strSrcFile.GetFileNamePart() ;
	//
	if ( !imgBuf.LoadMediaFile( strSrcFile ) )
	{
		//
		// ERI, MEI, BMP, etc... ���ݒ�
		//
		m_lstSequence.RemoveAll( ) ;
		m_lstSequence.Merge( 0, imgBuf.GetSequenceTable() ) ;
		m_intDuration.m_varInt = imgBuf.GetTotalTime( ) ;
		m_sizeImage.SetMemberAsInt( L"w", imgBuf.GetWidth() ) ;
		m_sizeImage.SetMemberAsInt( L"h", imgBuf.GetHeight() ) ;
		m_ptHotSpot.SetMemberAsInt( L"x", imgBuf.GetHotSpot().x ) ;
		m_ptHotSpot.SetMemberAsInt( L"y", imgBuf.GetHotSpot().y ) ;
		//
		for ( int i = 0; i < (int) imgBuf.GetTotalFrameCount(); i ++ )
		{
			ECSImage *	pImage = new ECSImage ;
			pImage->m_intIndex.m_varInt = i ;
			pImage->m_strFileName.m_varStr = wstrFileName ;
			m_varArray.Add( pImage ) ;
			//
			PEGL_IMAGE_INFO	pFrame = imgBuf.GetFrameAt( i ) ;
			if ( pFrame == NULL )
			{
				continue ;
			}
			::eglAddImageBufferRef( pFrame ) ;
			pImage->m_imgBuf.AddFrame( pFrame ) ;
			pImage->m_imgBuf.GetSequenceTable().SetAt( 0, 0 ) ;
			pImage->m_imgBuf.SetCurrentSequence( 0 ) ;
		}
	}
	else
	{
		//
		// PSD �t�@�C���̓ǂݍ���
		//
		ERawFile	file ;
		if ( file.Open( strSrcFile, file.modeRead | file.shareRead ) )
		{
			return	eslErrGeneral ;
		}
		PSD::File	psdf ;
		if ( psdf.Open( file ) )
		{
			return	eslErrGeneral ;
		}
		//
		// ��{���ݒ�
		//
		PSD::FILE_HEADER	filehdr ;
		psdf.GetFileHeader( filehdr ) ;
		m_sizeImage.SetMemberAsInt( L"w", filehdr.dwWidth ) ;
		m_sizeImage.SetMemberAsInt( L"h", filehdr.dwHeight ) ;
		//
		// �p���b�g�e�[�u���擾
		//
		EGL_PALETTE		rgbPalette[0x100] ;
		EStreamBuffer	bufColor ;
		::eslFillMemory( rgbPalette, 0, sizeof(rgbPalette) ) ;
		//
		if ( filehdr.wMode == PSD::modeIndexed )
		{
			psdf.GetColorModeData( bufColor ) ;
			EPtrBuffer	ptrbuf = bufColor.GetBuffer( ) ;
			//
			if ( ptrbuf.GetLength() < 0x100 * 3 )
			{
				const BYTE *	pbytColor =
					(const BYTE *) ptrbuf.GetBuffer( ) ;
				for ( int i = 0; i < 3; i ++ )
				{
					rgbPalette[i].rgb.Red = pbytColor[i] ;
					rgbPalette[i].rgb.Green = pbytColor[i + 0x100] ;
					rgbPalette[i].rgb.Blue = pbytColor[i + 0x200] ;
				}
			}
		}
		//
		// ���C���[�������擾
		//
		int	nLayerCount = psdf.GetLayerCount() ;
		EGLImage	imgBase, imgBaseChannels, imgBaseAlpha ;
		//
		if ( nLayerCount > 0 )
		{
			if ( fMaskMode & 0x02 )
			{
				if ( psdf.LoadBaseImage( imgBaseChannels ) )
				{
					return	eslErrGeneral ;
				}
				if ( imgBaseChannels.GetFormatType() & EIF_WITH_ALPHA )
				{
					EGLSize	size = imgBaseChannels.GetSize( ) ;
					imgBaseAlpha.CreateImage
						( EIF_GRAY_BITMAP, size.w, size.h, 8 ) ;
					imgBaseChannels.
						UnpackAlphaChannel( NULL, imgBaseAlpha, 0 ) ;
				}
				else
				{
					fMaskMode &= ~0x02 ;
				}
			}
			HEGL_DRAW_IMAGE	hDraw = NULL ;
			EGL_DRAW_PARAM	dp ;
			if ( fBlendMode )
			{
				::eslFillMemory( &dp, 0, sizeof(dp) ) ;
				hDraw = ::eglCreateDrawImage( ) ;
			}
			for ( int i = 0; i < nLayerCount; i ++ )
			{
				//
				// ���C���[�摜�擾
				//
				EGLImage				imgLayer ;
				PSD::File::LayerInfo	liLayer ;
				if ( psdf.GetLayerInfo( liLayer, i ) )
				{
					continue ;
				}
				if ( psdf.LoadLayerImage( imgLayer, i ) )
				{
					continue ;
				}
				PEGL_IMAGE_INFO	pLayer = imgLayer ;
				if ( (pLayer != NULL)
					&& (pLayer->fdwFormatType & EIF_WITH_ALPHA) )
				{
					if ( !(fMaskMode & 0x01) )
					{
						imgLayer.BlendAlphaChannel
							( NULL, NULL, EGL_BAC_MULTIPLY ) ;
					}
					else
					{
						EGL_IMAGE_INFO	eiiLayer = *pLayer ;
						eiiLayer.fdwFormatType &= ~EIF_WITH_ALPHA ;
						imgLayer.ConvertFrom( &eiiLayer ) ;
					}
					if ( fMaskMode & 0x02 )
					{
						EGL_IMAGE_INFO	eiiSrcRGB, eiiAlpha ;
						EGLImageRect	irAlpha = liLayer.irRect ;
						if ( !::eglGetClippedImageInfo
								( &eiiAlpha, imgBaseAlpha, &irAlpha ) )
						{
							eiiSrcRGB = *pLayer ;
							imgLayer.BlendAlphaChannel
								( &eiiSrcRGB, &eiiAlpha, EGL_BAC_MULTIPLY ) ;
						}
					}
				}
				if ( fBlendMode )
				{
					//
					// ���C���[�摜�������`��
					//
					if ( (pLayer == NULL)
						|| (liLayer.dwFlags & PSD::flagInvisible) )
					{
						continue ;
					}
					if ( imgBase.GetInfo() == NULL )
					{
						imgBase.CreateImage
							( pLayer->fdwFormatType,
								filehdr.dwWidth, filehdr.dwHeight,
										pLayer->dwBitsPerPixel ) ;
						imgBase.ReverseVertically( ) ;
						hDraw->Initialize( imgBase, NULL, NULL ) ;
					}
					//
					switch ( PSD::dwswap( liLayer.dwBlendMode ) )
					{
					case	PSD::blendMultiply:
						dp.dwFlags = EGL_DRAW_F_MUL ;
						break ;
					case	PSD::blendDivision:
						dp.dwFlags = EGL_DRAW_F_DIV ;
						break ;
					default:
						dp.dwFlags = EGL_DRAW_BLEND_ALPHA ;
						break ;
					}
					dp.ptBasePos.x = liLayer.irRect.x ;
					dp.ptBasePos.y = liLayer.irRect.y ;
					dp.pSrcImage = pLayer ;
					dp.nTransparency = liLayer.nTransparency ;
					//
					if ( !hDraw->PrepareDraw( &dp ) )
					{
						hDraw->DrawImage( ) ;
					}
				}
				else
				{
					//
					// ���C���[�摜��ǉ�
					//
					ECSImage *	pImage = new ECSImage ;
					pImage->m_intIndex.m_varInt = i ;
					pImage->m_strName.m_varStr = liLayer.strLayerName ;
					pImage->m_strFileName.m_varStr = wstrFileName ;
					m_varArray.Add( pImage ) ;
					//
					if ( pLayer != NULL )
					{
						if ( pLayer->pPaletteEntries != NULL )
						{
							DWORD	dwPaletteBytes =
								max( pLayer->dwPaletteCount, 0x100 )
													* sizeof(EGL_PALETTE) ;
							::eslMoveMemory
								( pLayer->pPaletteEntries,
									rgbPalette, dwPaletteBytes ) ;
						}
						::eglAddImageBufferRef( pLayer ) ;
						pImage->m_imgBuf.AddFrame( pLayer ) ;
						pImage->m_imgBuf.GetSequenceTable().SetAt( 0, 0 ) ;
						pImage->m_imgBuf.SetCurrentSequence( 0 ) ;
					}
					pImage->m_imgBuf.SetHotSpot
						( EGLPoint( - liLayer.irRect.x, - liLayer.irRect.y ) ) ;
				}
			}
			if ( hDraw != NULL )
			{
				hDraw->Release( ) ;
			}
		}
		else
		{
			//
			// �w�i�摜�݂̂� PSD �t�@�C��
			//
			if ( psdf.LoadBaseImage( imgBase ) )
			{
				return	eslErrGeneral ;
			}
		}
		if ( imgBase.GetInfo() != NULL )
		{
			ECSImage *	pImage = new ECSImage ;
			pImage->m_intIndex.m_varInt = 0 ;
			pImage->m_strFileName.m_varStr = wstrFileName ;
			m_varArray.Add( pImage ) ;
			//
			PEGL_IMAGE_INFO	pBaseInf = imgBase ;
			if ( pBaseInf != NULL )
			{
				if ( pBaseInf->pPaletteEntries != NULL )
				{
					DWORD	dwPaletteBytes =
						max( pBaseInf->dwPaletteCount, 0x100 )
											* sizeof(EGL_PALETTE) ;
					::eslMoveMemory
						( pBaseInf->pPaletteEntries,
							rgbPalette, dwPaletteBytes ) ;
				}
				::eglAddImageBufferRef( pBaseInf ) ;
				pImage->m_imgBuf.AddFrame( pBaseInf ) ;
				pImage->m_imgBuf.GetSequenceTable().SetAt( 0, 0 ) ;
				pImage->m_imgBuf.SetCurrentSequence( 0 ) ;
			}
		}
	}
	m_strFileName.m_varStr = wstrFileName ;
	return	eslErrSuccess ;
}

// �摜�̐؂�o��
//////////////////////////////////////////////////////////////////////////////
int ECSImageContext::CutImage
	( int nAlign, int nMargin,
		int nThreshold, int nFlags, EGL_IMAGE_RECT & irClip )
{
	//
	// �؂�o���̈�𔻒肷��
	//
	int		i ;
	EGLRect	rctClip ;
	EGLSize	sizeImage ;
	sizeImage.w = m_sizeImage.GetMemberAsInt( L"w", 0x7FFFFFFF ) ;
	sizeImage.h = m_sizeImage.GetMemberAsInt( L"h", 0x7FFFFFFF ) ;
	if ( nAlign > 0 )
	{
		rctClip.left = sizeImage.w - 1 ;
		rctClip.top = sizeImage.h - 1 ;
		rctClip.right = 0 ;
		rctClip.bottom = 0 ;
	}
	else
	{
		rctClip.left = 0 ;
		rctClip.top = 0 ;
		rctClip.right = sizeImage.w - 1 ;
		rctClip.bottom = sizeImage.h - 1 ;
	}
	//
	for ( i = 0; i < GetLength(); i ++ )
	{
		ECSImage *	pImage = GetImageAt( i ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		PEGL_IMAGE_INFO	pInfo = pImage->m_imgBuf ;
		if ( pInfo == NULL )
		{
			continue ;
		}
		EGLRect		rctLocal = rctClip ;
		EGLPoint	ptHotSpot = pImage->m_imgBuf.GetHotSpot( ) ;
		rctLocal.left += ptHotSpot.x ;
		rctLocal.top += ptHotSpot.y ;
		rctLocal.right += ptHotSpot.x ;
		rctLocal.bottom += ptHotSpot.y ;
		if ( (rctLocal.left <= 0) && (rctLocal.top <= 0)
			&& (rctLocal.right >= (int) pInfo->dwImageWidth)
			&& (rctLocal.bottom >= (int) pInfo->dwImageHeight) )
		{
			continue ;
		}
		if ( !(pInfo->fdwFormatType & EIF_WITH_ALPHA)
			|| (pInfo->dwBitsPerPixel != 32) || (nAlign == 0) )
		{
			if ( rctLocal.left > 0 )
			{
				rctLocal.left = 0 ;
			}
			if ( rctLocal.top > 0 )
			{
				rctLocal.top = 0 ;
			}
			if ( rctLocal.right < (int) pInfo->dwImageWidth )
			{
				rctLocal.right = (int) pInfo->dwImageWidth - 1 ;
			}
			if ( rctLocal.bottom < (int) pInfo->dwImageHeight )
			{
				rctLocal.bottom = (int) pInfo->dwImageHeight - 1 ;
			}
		}
		else
		{
			const BYTE *	pbytLine = (const BYTE *) pInfo->ptrImageArray ;
			for ( int y = 0; y < (int) pInfo->dwImageHeight; y ++ )
			{
				int		fUpperMask = - (int) (y < rctLocal.top) ;
				int		fUnderMask = - (int) (y > rctLocal.bottom) ;
				bool	fLineFlag = false ;
				if ( nThreshold == 0 )
				{
					for ( int x = 0; x < (int) pInfo->dwImageWidth; x ++ )
					{
						if ( ((DWORD*)pbytLine)[x] )
						{
							fLineFlag = true ;
							int	fLeftMask = - (int) (x < rctLocal.left) ;
							int	fRightMask = - (int) (x > rctLocal.right) ;
							rctLocal.left =
								(~fLeftMask & rctLocal.left) | (fLeftMask & x) ;
							rctLocal.right =
								(~fRightMask & rctLocal.right) | (fRightMask & x) ;
						}
					}
				}
				else
				{
					const BYTE *	pbytNext = pbytLine ;
					for ( int x = 0; x < (int) pInfo->dwImageWidth; x ++ )
					{
						int	v = (int) pbytNext[0] + (int) pbytNext[1]
								+ (int) pbytNext[2] + (int) pbytNext[3] ;
						pbytNext += 4 ;
						if ( v > nThreshold )
						{
							fLineFlag = true ;
							int	fLeftMask = - (int) (x < rctLocal.left) ;
							int	fRightMask = - (int) (x > rctLocal.right) ;
							rctLocal.left =
								(~fLeftMask & rctLocal.left) | (fLeftMask & x) ;
							rctLocal.right =
								(~fRightMask & rctLocal.right) | (fRightMask & x) ;
						}
					}
				}
				if ( fLineFlag )
				{
					rctLocal.top =
						(~fUpperMask & rctLocal.top) | (fUpperMask & y) ;
					rctLocal.bottom =
						(~fUnderMask & rctLocal.bottom) | (fUnderMask & y) ;
				}
				pbytLine += pInfo->dwBytesPerLine ;
			}
		}
		rctClip.left = rctLocal.left - ptHotSpot.x ;
		rctClip.top = rctLocal.top - ptHotSpot.y ;
		rctClip.right = rctLocal.right - ptHotSpot.x ;
		rctClip.bottom = rctLocal.bottom - ptHotSpot.y ;
	}
	//
	// �؂�o�����W�𐮂���
	//
	rctClip.left -= nMargin ;
	rctClip.top -= nMargin ;
	rctClip.right += nMargin ;
	rctClip.bottom += nMargin ;
	if ( rctClip.left < 0 )
	{
		rctClip.left = 0 ;
	}
	if ( rctClip.top < 0 )
	{
		rctClip.top = 0 ;
	}
/*	if ( rctClip.right >= sizeImage.w )
	{
		rctClip.right = sizeImage.w - 1 ;
	}
	if ( rctClip.bottom >= sizeImage.h )
	{
		rctClip.bottom = sizeImage.h - 1 ;
	}*/
	if ( (rctClip.left > rctClip.right) || (rctClip.top > rctClip.bottom) )
	{
		return	1 ;
	}
	if ( nAlign > 0 )
	{
		rctClip.right += nAlign ;
		rctClip.bottom += nAlign ;
		rctClip.left -= rctClip.left % nAlign ;
		rctClip.top -= rctClip.top % nAlign ;
		rctClip.right -= rctClip.right % nAlign + 1 ;
		rctClip.bottom -= rctClip.bottom % nAlign + 1 ;
	}
	irClip = EGLImageRect( rctClip ) ;
	if ( (irClip.w <= 0) || (irClip.h <= 0) )
	{
		return	1 ;
	}
	//
	// �摜��؂�o��
	//
	HEGL_DRAW_IMAGE	hDraw = ::eglCreateDrawImage( ) ;
	EGL_DRAW_PARAM	dp ;
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	//
	for ( i = 0; i < GetLength(); i ++ )
	{
		ECSImage *	pImage = GetImageAt( i ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		PEGL_IMAGE_INFO	pInfo = pImage->m_imgBuf ;
		if ( pInfo == NULL )
		{
			continue ;
		}
		EGLPoint	ptHotSpot = pImage->m_imgBuf.GetHotSpot( ) ;
		PEGL_IMAGE_INFO	pNewImage =
			::eglCreateImageBuffer
				( pInfo->fdwFormatType,
					irClip.w, irClip.h, pInfo->dwBitsPerPixel ) ;
		::eglReverseVertically( pNewImage ) ;
		//
		if ( pNewImage->pPaletteEntries && pInfo->pPaletteEntries )
		{
			DWORD	dwPaletteLength =
				max( pNewImage->dwPaletteCount,
						pInfo->dwPaletteCount ) * sizeof(EGL_PALETTE) ;
			::eslMoveMemory
				( pNewImage->pPaletteEntries,
					pInfo->pPaletteEntries, dwPaletteLength ) ;
		}
		//
		hDraw->Initialize( pNewImage, NULL, NULL ) ;
		dp.ptBasePos.x = - ptHotSpot.x - irClip.x ;
		dp.ptBasePos.y = - ptHotSpot.y - irClip.y ;
		dp.pSrcImage = pInfo ;
		if ( !hDraw->PrepareDraw( &dp ) )
		{
			hDraw->DrawImage( ) ;
		}
		//
		pImage->m_imgBuf.DeleteImage( ) ;
		pImage->m_imgBuf.AddFrame( pNewImage ) ;
		pImage->m_imgBuf.GetSequenceTable().SetAt( 0, 0 ) ;
		pImage->m_imgBuf.SetCurrentSequence( 0 ) ;
		pImage->m_imgBuf.SetHotSpot( EGLPoint( 0, 0 ) ) ;
		//
		if ( nFlags & cofRemoveAlpha )
		{
			pNewImage->fdwFormatType &= ~EIF_WITH_ALPHA ;
		}
		else if ( (nFlags & cofAutoAlpha)
				&& (pNewImage->fdwFormatType & EIF_WITH_ALPHA)
				&& (pNewImage->dwBitsPerPixel == 32) )
		{
			bool	fRemoveAlpha = true ;
			const BYTE *	pbytLine =
				(const BYTE *) pNewImage->ptrImageArray ;
			for ( int y = 0; y < (int) pNewImage->dwImageHeight; y ++ )
			{
				const DWORD *	pdwLine = (const DWORD *) pbytLine ;
				pbytLine += pNewImage->dwBytesPerLine ;
				for ( int x = 0; x < (int) pNewImage->dwImageWidth; x ++ )
				{
					BYTE	bytAlpha = (BYTE) (pdwLine[x] >> 24) ;
					if ( bytAlpha < 0xFE )
					{
						fRemoveAlpha = false ;
						break ;
					}
				}
				if ( !fRemoveAlpha )
				{
					break ;
				}
			}
			if ( fRemoveAlpha )
			{
				pNewImage->fdwFormatType &= ~EIF_WITH_ALPHA ;
			}
		}
	}
	//
	hDraw->Release( ) ;
	//
	// �摜�T�C�Y�E�z�b�g�X�|�b�g���X�V
	//
	m_sizeImage.SetMemberAsInt( L"w", irClip.w ) ;
	m_sizeImage.SetMemberAsInt( L"h", irClip.h ) ;
	if ( nFlags & cofKeepHotspot )
	{
		m_ptHotSpot.SetMemberAsInt
			( L"x", m_ptHotSpot.GetMemberAsInt( L"x", 0 ) - irClip.x ) ;
		m_ptHotSpot.SetMemberAsInt
			( L"y", m_ptHotSpot.GetMemberAsInt( L"y", 0 ) - irClip.y ) ;
	}
	else
	{
		m_ptHotSpot.SetMemberAsInt( L"x", 0 ) ;
		m_ptHotSpot.SetMemberAsInt( L"y", 0 ) ;
	}
	//
	return	0 ;
}

// �摜�̔z��
//////////////////////////////////////////////////////////////////////////////
int ECSImageContext::ArrangeImage( bool fWayVert, bool fPutRight, int nAlign )
{
	//
	// �V�����摜�o�b�t�@�̃T�C�Y���v�Z
	//
	int			i ;
	EGLSize		sizeImage = m_imgBuf.GetSize( ) ;
	EGLPoint	ptNext( 0, 0 ) ;
	if ( fPutRight )
	{
		ptNext.x = sizeImage.w ;
	}
	else
	{
		ptNext.y = sizeImage.h ;
	}
	if ( nAlign <= 0 )
	{
		nAlign = 1 ;
	}
	EGLPoint	ptBeginPos = ptNext ;
	bool		fAlpha = false ;
	//
	for ( i = 0; i < GetLength(); i ++ )
	{
		ECSImage *	pImage = GetImageAt( i ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		EGLSize	size = pImage->m_imgBuf.GetSize( ) ;
		size.w += ptNext.x ;
		size.h += ptNext.y ;
		if ( size.w > sizeImage.w )
		{
			sizeImage.w = size.w ;
		}
		if ( size.h > sizeImage.h )
		{
			sizeImage.h = size.h ;
		}
		if ( fWayVert )
		{
			ptNext.y = (size.h + nAlign - 1) / nAlign * nAlign ;
		}
		else
		{
			ptNext.x = (size.w + nAlign - 1) / nAlign * nAlign ;
		}
		if ( pImage->m_imgBuf.GetFormatType()
				& (EIF_WITH_ALPHA | EIF_WITH_CLIPPING) )
		{
			fAlpha = true ;
		}
	}
	//
	// �摜�o�b�t�@�̐���
	//
	if ( m_imgBuf.GetFormatType() & (EIF_WITH_ALPHA | EIF_WITH_CLIPPING) )
	{
		fAlpha = true ;
	}
	//
	PEGL_IMAGE_INFO	pNewBuf =
		::eglCreateImageBuffer
			( fAlpha ? EIF_RGBA_BITMAP : EIF_RGB_BITMAP,
							sizeImage.w, sizeImage.h, 32 ) ;
	::eglReverseVertically( pNewBuf ) ;
	//
	// ���̉摜�𕡐�
	//
	HEGL_DRAW_IMAGE	hDraw = ::eglCreateDrawImage( ) ;
	EGL_DRAW_PARAM	dp ;
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	hDraw->Initialize( pNewBuf, NULL, NULL ) ;
	//
	dp.pSrcImage = m_imgBuf ;
	if ( dp.pSrcImage != NULL )
	{
		if ( !hDraw->PrepareDraw( &dp ) )
		{
			hDraw->DrawImage( ) ;
		}
	}
	//
	// �摜��z��
	//
	dp.ptBasePos = ptBeginPos ;
	for ( i = 0; i < GetLength(); i ++ )
	{
		ECSImage *	pImage = GetImageAt( i ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		dp.pSrcImage = pImage->m_imgBuf ;
		if ( dp.pSrcImage == NULL )
		{
			continue ;
		}
		if ( !hDraw->PrepareDraw( &dp ) )
		{
			hDraw->DrawImage( ) ;
		}
		if ( fWayVert )
		{
			dp.ptBasePos.y += dp.pSrcImage->dwImageHeight ;
			dp.ptBasePos.y =
				(dp.ptBasePos.y + nAlign - 1) / nAlign * nAlign ;
		}
		else
		{
			dp.ptBasePos.x += dp.pSrcImage->dwImageWidth ;
			dp.ptBasePos.x =
				(dp.ptBasePos.x + nAlign - 1) / nAlign * nAlign ;
		}
	}
	//
	// �摜�R���e�L�X�g������
	//
	DeleteContext( ) ;
	//
	m_imgBuf.DeleteImage( ) ;
	m_imgBuf.AddFrame( pNewBuf ) ;
	m_imgBuf.GetSequenceTable().SetAt( 0, 0 ) ;
	m_imgBuf.SetCurrentSequence( 0 ) ;
	//
	hDraw->Release( ) ;
	//
	// �摜�z��ɕ�����ݒ�
	//
	ECSImage *	pImage = new ECSImage ;
	::eglAddImageBufferRef( pNewBuf ) ;
	pImage->m_imgBuf.AddFrame( pNewBuf ) ;
	pImage->m_imgBuf.GetSequenceTable().SetAt( 0, 0 ) ;
	pImage->m_imgBuf.SetCurrentSequence( 0 ) ;
	m_varArray.Add( pImage ) ;
	//
	return	0 ;
}

// �A�j���[�V�����쐬
//////////////////////////////////////////////////////////////////////////////
int ECSImageContext::MakeAnimation( const wchar_t * pwszSeq, int nDuration )
{
	//
	// �t���[���o�b�t�@��ݒ�
	//
	int			i ;
	EGLSize		sizeImage ;
	EGLPoint	ptHotSpot ;
	//
	m_imgBuf.DeleteImage( ) ;
	//
	sizeImage.w = m_sizeImage.GetMemberAsInt( L"w", 0 ) ;
	sizeImage.h = m_sizeImage.GetMemberAsInt( L"h", 0 ) ;
	ptHotSpot.x = m_ptHotSpot.GetMemberAsInt( L"x", 0 ) ;
	ptHotSpot.y = m_ptHotSpot.GetMemberAsInt( L"y", 0 ) ;
	//
	m_imgBuf.SetHotSpot( ptHotSpot ) ;
	m_imgBuf.SetTotalTime( nDuration ) ;
	//
	for ( i = 0; i < GetLength(); i ++ )
	{
		ECSImage *	pImage = GetImageAt( i ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		PEGL_IMAGE_INFO	pInfo = pImage->m_imgBuf ;
		if ( pInfo == NULL )
		{
			continue ;
		}
		EGLSize	size = pImage->m_imgBuf.GetSize( ) ;
		if ( size != sizeImage )
		{
			printf( " %d �t���[���̉摜�T�C�Y����v���܂���B\n", i ) ;
			return	1 ;
		}
		::eglAddImageBufferRef( pInfo ) ;
		m_imgBuf.AddFrame( pInfo ) ;
	}
	//
	// �V�[�P���X�e�[�u����ݒ�
	//
	EStreamWideString	swsSeq = pwszSeq ;
	m_imgBuf.GetSequenceTable().RemoveAll( ) ;
	int		nFrameCount = m_imgBuf.GetTotalFrameCount( ) ;
	//
	if ( !swsSeq.DisregardSpace() )
	{
		while ( !swsSeq.DisregardSpace() )
		{
			int	nRadix = swsSeq.GetNumberRadix( ) ;
			if ( nRadix < 0 )
			{
				printf( " �t���[���ԍ��ɂ͐��l���w�肵�Ă��������B\n" ) ;
				return	1 ;
			}
			int	iFrame = swsSeq.GetInteger( nRadix ) ;
			if ( (iFrame < 0) || (iFrame >= nFrameCount) )
			{
				printf( " �t���[���ԍ����͈͂𒴂��Ă��܂��B\n" ) ;
				return	1 ;
			}
			wchar_t	wch = swsSeq.HasToComeChar( L"-*," ) ;
			if ( wch == L'-' )
			{
				int	nRadix = swsSeq.GetNumberRadix( ) ;
				if ( nRadix < 0 )
				{
					printf( " �t���[���ԍ��ɂ͐��l���w�肵�Ă��������B\n" ) ;
					return	1 ;
				}
				int	iEnd = swsSeq.GetInteger( nRadix ) ;
				if ( (iEnd < 0) || (iEnd >= nFrameCount) )
				{
					printf( " �t���[���ԍ����͈͂𒴂��Ă��܂��B\n" ) ;
					return	1 ;
				}
				int	nStep = (iEnd >= iFrame) ? 1 : -1 ;
				for ( ; ; )
				{
					m_imgBuf.GetSequenceTable().Add( iFrame ) ;
					if ( iFrame == iEnd )
					{
						break ;
					}
					iFrame += nStep ;
				}
				wch = swsSeq.HasToComeChar( L"," ) ;
			}
			else if ( wch == L'*' )
			{
				int	nRadix = swsSeq.GetNumberRadix( ) ;
				if ( nRadix < 0 )
				{
					printf( " �t���[���̔����񐔂ɂ͐��l���w�肵�Ă��������B\n" ) ;
					return	1 ;
				}
				int	nRepCount = swsSeq.GetInteger( nRadix ) ;
				for ( i = 0; i < nRepCount; i ++ )
				{
					m_imgBuf.GetSequenceTable().Add( iFrame ) ;
				}
				wch = swsSeq.HasToComeChar( L"," ) ;
			}
			if ( wch == L',' )
			{
				continue ;
			}
			if ( !swsSeq.DisregardSpace() )
			{
				printf( " �V�[�P���X�e�[�u���̏������s���ł��B\n" ) ;
				return	1 ;
			}
		}
	}
	else
	{
		m_imgBuf.GetSequenceTable().Merge( 0, m_lstSequence ) ;
	}
	if ( m_imgBuf.GetSequenceTable().GetSize() == 0 )
	{
		ENumArray<UINT> &	lstSeq = m_imgBuf.GetSequenceTable() ;
		for ( i = 0; i < GetLength(); i ++ )
		{
			lstSeq.Add( i ) ;
		}
	}
	m_imgBuf.SetCurrentSequence( 0 ) ;
	//
	return	0 ;
}

// �}�[�W����
//////////////////////////////////////////////////////////////////////////////
void ECSImageContext::MergeContext
	( ECSImageContext & imgctx,
		int iFirst, int nCount, const wchar_t * pwszName )
{
	//
	// �K���t���[���̗�
	//
	if ( iFirst < 0 )
	{
		iFirst = 0 ;
	}
	if ( nCount < 0 )
	{
		nCount = imgctx.GetLength( ) ;
	}
	if ( iFirst + nCount > imgctx.GetLength() )
	{
		nCount = imgctx.GetLength() - iFirst ;
	}
	if ( nCount <= 0 )
	{
		return ;
	}
	EStreamWideString	swsUsage = pwszName ;
	ENumArray<int>		lstFrame ;
	int		i ;
	for ( i = 0; i < imgctx.GetLength(); i ++ )
	{
		ECSImage *	pImage = imgctx.GetImageAt( i ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		if ( (i < iFirst) || ((i - iFirst) >= nCount) )
		{
			continue ;
		}
		if ( !swsUsage.IsEmpty() )
		{
			EStreamWideString	swsName = pImage->m_strName.m_varStr ;
			EString				strErrMsg ;
			swsUsage.MoveIndex( 0 ) ;
			if ( swsName.IsMatchUsage( swsUsage, strErrMsg ) )
			{
				continue ;
			}
		}
		lstFrame.Add( i ) ;
	}
	if ( lstFrame.GetSize() == 0 )
	{
		return ;
	}
	//
	// �x�[�X�T�C�Y�ύX
	//
	EGLSize	sizeImage, sizeAdd ;
	sizeImage.w = m_sizeImage.GetMemberAsInt( L"w", 0 ) ;
	sizeImage.h = m_sizeImage.GetMemberAsInt( L"h", 0 ) ;
	sizeAdd.w = imgctx.m_sizeImage.GetMemberAsInt( L"w", 0 ) ;
	sizeAdd.h = imgctx.m_sizeImage.GetMemberAsInt( L"h", 0 ) ;
	if ( sizeImage.w < sizeAdd.w )
	{
		m_sizeImage.SetMemberAsInt( L"w", sizeAdd.w ) ;
	}
	if ( sizeImage.h < sizeAdd.h )
	{
		m_sizeImage.SetMemberAsInt( L"h", sizeAdd.h ) ;
	}
	//
	// �A�j���[�V�������Ԓǉ�
	//
	m_intDuration.m_varInt += imgctx.m_intDuration.m_varInt ;
	//
	// �V�[�P���X�e�[�u������
	//
	for ( i = 0; i < (int) imgctx.m_lstSequence.GetSize(); i ++ )
	{
		m_lstSequence.Add( imgctx.m_lstSequence[i] + GetLength() ) ;
	}
	//
	// �摜�o�b�t�@�ǉ�
	//
	for ( i = 0; i < (int) lstFrame.GetSize(); i ++ )
	{
		ECSImage *	pImage = imgctx.GetImageAt( lstFrame[i] ) ;
		ESLAssert( pImage != NULL ) ;
		PEGL_IMAGE_INFO	pInfo = pImage->m_imgBuf ;
		if ( pInfo == NULL )
		{
			continue ;
		}
		m_varArray.Add( pImage->Duplicate() ) ;
	}
}

// �I�u�W�F�N�g�̌^�����擾����
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSImageContext::GetTypeName( void ) const
{
	return	L"ImageContext" ;
}

// �I�u�W�F�N�g�𕡐�
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSImageContext::Duplicate( void )
{
	ECSImageContext *	pImgContext = new ECSImageContext ;
	pImgContext->CopyFrom( *this ) ;
	pImgContext->m_lstSequence.RemoveAll( ) ;
	pImgContext->m_lstSequence.Merge( 0, m_lstSequence ) ;
	pImgContext->m_sizeImage.CopyFrom( m_sizeImage ) ;
	pImgContext->m_ptHotSpot.CopyFrom( m_ptHotSpot ) ;
	return	pImgContext ;
}

// �I�u�W�F�N�g����
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::Move( ECSContext & context, ECSObject * obj )
{
	ECSImageContext *	pImgContext =
		ESLTypeCast<ECSImageContext>( obj->GetObjectEntity() ) ;
	if ( pImgContext == NULL )
	{
		return	ESLErrorMsg
			( "ImageContext �̑������ "
				"ImageContext �I�u�W�F�N�g�ł͂���܂���B" ) ;
	}
	CopyFrom( *pImgContext ) ;
	m_lstSequence.RemoveAll( ) ;
	m_lstSequence.Merge( 0, pImgContext->m_lstSequence ) ;
	m_sizeImage.CopyFrom( pImgContext->m_sizeImage ) ;
	m_ptHotSpot.CopyFrom( pImgContext->m_ptHotSpot ) ;
	delete	obj ;
	return	eslErrSuccess ;
}

// �����o�ϐ��C���f�b�N�X�擾
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::GetVariableIndex( int & nIndex, ECSObject & obj )
{
	if ( obj.m_vtType == csvtString )
	{
		ECSWideString &	wstrName = ((ECSString&)obj).m_varStr ;
		if ( wstrName == L"duration" )
		{
			nIndex = -1 ;
			return	eslErrSuccess ;
		}
		else if ( wstrName == L"size" )
		{
			nIndex = -2 ;
			return	eslErrSuccess ;
		}
		else if ( wstrName == L"hotspot" )
		{
			nIndex = -3 ;
			return	eslErrSuccess ;
		}
		else if ( wstrName == L"filename" )
		{
			nIndex = -4 ;
			return	eslErrSuccess ;
		}
	}
	return	ECSArray::GetVariableIndex( nIndex, obj ) ;
}

// �����o�ϐ��擾
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSImageContext::GetVariableAt( int nIndex )
{
	if ( nIndex < 0 )
	{
		switch ( nIndex )
		{
		case	-1:
			return	&m_intDuration ;
		case	-2:
			return	&m_sizeImage ;
		case	-3:
			return	&m_ptHotSpot ;
		case	-4:
			return	&m_strFileName ;
		}
		return	NULL ;
	}
	return	ECSArray::GetVariableAt( nIndex ) ;
}

// �����o�ϐ��ݒ�
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSImageContext::SetVariableAt( int nIndex, ECSObject * obj )
{
	if ( nIndex < 0 )
	{
		return	NULL ;
	}
	return	ECSArray::SetVariableAt( nIndex, obj ) ;
}

// �����o�֐��C���f�b�N�X�擾
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName.FindIndex( pwszName ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	ESLError	err = ECSArray::GetFunction( context, nIndex, pwszName ) ;
	if ( err )
	{
		return	err ;
	}
	nIndex += m_staFuncName.GetSize( ) ;
	return	eslErrSuccess ;
}

// �����o�֐��Ăяo��
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::CallFunction
	( ECSContext & context,
		int nIndex, EObjArray<ECSObject> & lstArg )
{
	if ( (nIndex >= 0) && (nIndex < (int) m_staFuncName.GetSize()) )
	{
		return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
	}
	nIndex -= m_staFuncName.GetSize( ) ;
	return	ECSArray::CallFunction( context, nIndex, lstArg ) ;
}

// �����o�֐� : load
//	( string filename, integer blend_mode := 0, integer layer_mask := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::Call_load
	( ECSContext & context, EObjArray<ECSObject> & lstArg )
{
	EConverterApp *	app = ESLTypeCast<EConverterApp>( &context ) ;
	ESLAssert( app != NULL ) ;
	//
	ESLError	err ;
	err = app->VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrFileName ;
	int				fBlendMode, fLayerMask ;
	err = app->GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsInt( fBlendMode, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsInt( fLayerMask, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	if ( (wstrFileName.GetAt(0) != L'\\')
			&& (wstrFileName.Find( L':' ) < 0) )
	{
		wstrFileName = ECSWideString(app->m_wstrSrcDir) + wstrFileName ;
	}
	return	app->PushObject
		( new ECSInteger
			( LoadImageFile( wstrFileName, fBlendMode, fLayerMask ) ) ) ;
}

// �����o�֐� : save( string filename[, string mime_type] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::Call_save
	( ECSContext & context, EObjArray<ECSObject> & lstArg )
{
	EConverterApp *	app = ESLTypeCast<EConverterApp>( &context ) ;
	ESLAssert( app != NULL ) ;
	//
	ESLError	err ;
	err = app->VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrFileName, wstrMimeType ;
	err = app->GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsStr( wstrMimeType, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	if ( (wstrFileName.GetAt(0) != L'\\')
			&& (wstrFileName.Find( L':' ) < 0) )
	{
		wstrFileName = ECSWideString(app->m_wstrDstDir) + wstrFileName ;
	}
	if ( wstrMimeType.IsEmpty() )
	{
		wstrMimeType = app->m_pwszFormatMime[app->m_nFmtType] ;
	}
	int				nFmtType = app->FormatTypeFromMIME( wstrMimeType ) ;
	ECSWideString	wstrExt = wstrFileName.GetFileExtensionPart() ;
	if ( wstrExt.IsEmpty() )
	{
		if ( wstrFileName.Right(1) != L"." )
		{
			wstrFileName += L'.' ;
		}
		wstrFileName += app->GetDefaultFileExt( nFmtType ) ;
	}
//	if ( m_imgBuf.GetInfo() == NULL )
	{
//		err = eslErrGeneral ;
	}
//	else
	{
		err = app->SaveImageContext( *this, wstrFileName, nFmtType ) ;
	}
	return	app->PushObject( new ECSInteger( err ) ) ;
}

// �����o�֐� :
// cut( integer align := 0, integer margin := 0,
//			integer threshold := 0,
//			integer op_flags := 5 [, referece cut_offset] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::Call_cut
	( ECSContext & context, EObjArray<ECSObject> & lstArg )
{
	EConverterApp *	app = ESLTypeCast<EConverterApp>( &context ) ;
	ESLAssert( app != NULL ) ;
	//
	ESLError	err ;
	err = app->VerifyArgumentCount( lstArg, 1, 6 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nAlign, nMargin, nThreshold, nFlags ;
	err = app->GetArgumentAsInt( nAlign, lstArg, 1, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsInt( nMargin, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsInt( nThreshold, lstArg, 3, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsInt( nFlags, lstArg, 4, 5 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSStructure *	pPoint =
		ESLTypeCast<ECSStructure>( lstArg.GetAt(5)->GetObjectEntity() ) ;
	if ( pPoint != NULL )
	{
		if ( pPoint->m_wstrTag != L"Point" )
		{
			pPoint = NULL ;
		}
	}
	//
	EGL_IMAGE_RECT	irClip ;
	int	nResult =
		CutImage( nAlign, nMargin, nThreshold, nFlags, irClip ) ;
	if ( !nResult && (pPoint != NULL) )
	{
		pPoint->SetMemberAsInt( L"x", irClip.x ) ;
		pPoint->SetMemberAsInt( L"y", irClip.y ) ;
	}
	return	app->PushObject( new ECSInteger( nResult ) ) ;
}

// �����o�֐� :
// arrange( integer way_vert := 0,
//			integer put_right := 0, integer align := 1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::Call_arrange
	( ECSContext & context, EObjArray<ECSObject> & lstArg )
{
	EConverterApp *	app = ESLTypeCast<EConverterApp>( &context ) ;
	ESLAssert( app != NULL ) ;
	//
	ESLError	err ;
	err = app->VerifyArgumentCount( lstArg, 1, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	int	fWayVert, fPutRight, nAlign ;
	err = app->GetArgumentAsInt( fWayVert, lstArg, 1, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsInt( fPutRight, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsInt( nAlign, lstArg, 3, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	//
	return	app->PushObject
		( new ECSInteger( ArrangeImage
			( (fWayVert != 0), (fPutRight != 0), nAlign ) ) ) ;
}

// �����o�֐� : animation( string seq_list := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::Call_animation
	( ECSContext & context, EObjArray<ECSObject> & lstArg )
{
	EConverterApp *	app = ESLTypeCast<EConverterApp>( &context ) ;
	ESLAssert( app != NULL ) ;
	//
	ESLError	err ;
	err = app->VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrSeqList ;
	err = app->GetArgumentAsStr( wstrSeqList, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	return	app->PushObject( new ECSInteger
		( MakeAnimation( wstrSeqList, m_intDuration.m_varInt ) ) ) ;
}

// �����o�֐� :
//	merge( reference imgctx, integer first := 0,
//			integer count := -1, string name := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSImageContext::Call_merge
	( ECSContext & context, EObjArray<ECSObject> & lstArg )
{
	EConverterApp *	app = ESLTypeCast<EConverterApp>( &context ) ;
	ESLAssert( app != NULL ) ;
	//
	ESLError	err ;
	err = app->VerifyArgumentCount( lstArg, 2, 5 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSImageContext *	pimgctx =
		ESLTypeCast<ECSImageContext>( lstArg.GetAt(1)->GetObjectEntity() ) ;
	if ( pimgctx == NULL )
	{
		return	ESLErrorMsg
			( "������ ImageContext �I�u�W�F�N�g���w�肳��Ă��܂���B" ) ;
	}
	ECSWideString	wstrName ;
	int				iFirst, nCount ;
	err = app->GetArgumentAsInt( iFirst, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsInt( nCount, lstArg, 3, -1 ) ;
	if ( err )
	{
		return	err ;
	}
	err = app->GetArgumentAsStr( wstrName, lstArg, 4, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	MergeContext( *pimgctx, iFirst, nCount, wstrName ) ;
	//
	return	app->PushObject( new ECSInteger ) ;
}
