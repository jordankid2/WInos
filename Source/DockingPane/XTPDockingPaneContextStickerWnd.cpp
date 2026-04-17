// XTPDockingPaneContextStickerWnd.cpp : implementation of the CXTPDockingPaneContextStickerWnd class.
//
// (c)1998-2018 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "Common/XTPFramework.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPGdiObjects.h"
#include "Common/XTPColorManager.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPResourceManager.h"
#include "Common/XTPImageManager.h"

#include "DockingPane/Resource.h"
#include "DockingPane/XTPDockingPaneDefines.h"
#include "DockingPane/XTPDockingPaneBase.h"
#include "DockingPane/XTPDockingPaneContext.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////////
// CXTPDockingPaneContextStickerWnd

CXTPDockingPaneContextStickerWnd::CXTPDockingPaneContextStickerWnd(CXTPDockingPaneContext* pContext)
	: m_pContext(pContext)
{
	m_typeSticker = m_selectedSticker = xtpPaneStickerNone;
}

CXTPDockingPaneContextStickerWnd::~CXTPDockingPaneContextStickerWnd()
{
}

BEGIN_MESSAGE_MAP(CXTPDockingPaneContextStickerWnd, CWnd)
	//{{AFX_MSG_MAP(CXTPDockingPaneContextStickerWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CXTPDockingPaneContextStickerWnd message handlers

BOOL CXTPDockingPaneContextStickerWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

struct CXTPDockingPaneContextStickerWnd::SPRITEINFO
{
	SPRITEINFO(int x, int y, int left, int top, int cx, int cy)
	{
		ptDest = CPoint(x, y);
		rcSrc.SetRect(left, top, left + cx, top + cy);
	}
	CPoint ptDest;
	CRect rcSrc;
};

enum XTPSpriteSticker
{
	xtpSpriteStickerTop,
	xtpSpriteStickerLeft,
	xtpSpriteStickerBottom,
	xtpSpriteStickerRight,
	xtpSpriteStickerTopSelected,
	xtpSpriteStickerLeftSelected,
	xtpSpriteStickerBottomSelected,
	xtpSpriteStickerRightSelected,
	xtpSpriteStickerCenter,
	xtpSpriteStickerCenterSelected,
	xtpSpriteStickerClient,
};

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2005Beta[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25,  0,  0,  0, 43, 30), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 25, 30, 33, 30, 43), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25, 63, 43,  0, 43, 30), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(63, 25,  0, 33, 30, 43), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25,  0,  0, 76, 43, 30), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 25, 90, 33, 30, 43), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25, 63, 43, 76, 43, 30), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(63, 25, 60, 33, 30, 43), // Right Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30, 30, 86,  0, 33, 33), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30, 30, 86, 76, 33, 33), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25, 25,  0,  0, 43, 43), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25,  0,  0,  0, 43, 30), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 25, 30, 33, 30, 43), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25, 63, 43,  0, 43, 30), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(63, 25,  0, 33, 30, 43), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25,  0,  0, 76, 43, 30), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 25, 90, 33, 30, 43), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25, 63, 43, 76, 43, 30), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(63, 25, 60, 33, 30, 43), // Right Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30, 30, 86,  0, 33, 33), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30, 30, 86, 76, 33, 33), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(25, 25,  0,  0, 43, 43), // Client
};

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2005_100[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30,  0,  61, 29, 29, 30), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 30,  90, 30, 30, 29), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30, 59,  91,  0, 29, 30), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(60, 30,  62,  0, 30, 29), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30,  0,  61, 90, 29, 30), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 30,  90, 91, 30, 29), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30, 59,  91, 61, 29, 30), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(60, 30,  62, 61, 30, 29), // Right Selected
 
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30, 30, 120, 82, 28, 29), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(24, 24, 120, 41, 41, 41), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(24, 24, 120,  0, 41, 41), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 29, 29, 32), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  29, 32, 32, 29), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  32,  0, 29, 32), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  0, 32, 29), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 90, 29, 32), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  29, 93, 32, 29), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  32, 61, 29, 32), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 61, 32, 29), // Right Selected
};

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2005_125[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(37,  0,  76,  36, 36, 38), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 37, 112,  38, 38, 36), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(37, 71, 114,   0, 36, 38), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(73, 37,  76,   0, 38, 36), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(37,  0,  76, 112, 36, 38), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 37, 112, 114, 38, 36), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(37, 71, 114,  76, 36, 38), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(73, 37,  76,  76, 38, 36), // Right Selected
 
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(36, 36, 150, 100, 35, 36), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30, 30, 150, 50,  50, 50), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(30, 30, 150,  0,  50, 50), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  0,  36, 36, 40), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 36,  40, 40, 36), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 40,   0, 36, 40), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  0,   0, 40, 36), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  0, 112, 36, 40), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 36, 116, 40, 36), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 40,  76, 36, 40), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  0,  76, 40, 36), // Right Selected
};

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2005_150[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(43,  0,  91,  42, 42, 45), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 43, 133,  45, 45, 42), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(43, 83, 136,   0, 42, 45), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(83, 43,  91,   0, 45, 42), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(43,  0,  91, 133, 42, 45), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 43, 133, 136, 45, 42), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(43, 83, 136,  91, 42, 45), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(83, 43,  91,  91, 45, 42), // Right Selected
 
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(43, 43, 178, 124, 42, 42), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(33, 33, 178,  62, 62, 62), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(33, 33, 178,   0, 62, 62), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  43, 43, 48), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  43,  48, 48, 43), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  48,   0, 43, 48), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,   0, 48, 43), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 134, 43, 48), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  43, 139, 48, 43), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  48,  91, 43, 48), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  91, 48, 43), // Right Selected
};

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2005_200[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(60,  0,   122,  58, 58, 60), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  60,  180,  60, 60, 58), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(60,  118, 182,  0,  58, 60), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(120, 60,  124,  0,  60, 58), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(60,  0,   122, 180, 58, 60), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  60,  180, 182, 60, 58), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(60,  118, 182, 122, 58, 60), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(120, 60,  124, 122, 60, 58), // Right Selected
 
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(60, 60, 240, 164, 56, 58), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(48, 48, 240, 82, 82, 82), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(48, 48, 240,  0, 82, 82), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 58, 58, 64), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  58, 64, 64, 58), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  64,  0, 58, 64), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  0, 64, 58), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 180, 58, 64), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  58, 186, 64, 58), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  64, 122, 58, 64), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 122, 64, 58), // Right Selected
};

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2008_100[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(36,  0, 142, 35, 35, 35), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 35, 177, 35, 35, 35), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(36, 70, 177,  0, 35, 35), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(72, 35, 142,  0, 35, 35), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(36,  0, 212, 35, 35, 35), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 35, 247, 35, 35, 35), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(36, 70, 247,  0, 35, 35), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(72, 35, 212,  0, 35, 35), // Right Selected

	CXTPDockingPaneContextStickerWnd::SPRITEINFO(37, 38, 359,  0, 32, 31), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(37, 38, 359, 31, 32, 31), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(15, 15, 282,  0, 77, 75), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 35, 35, 36), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  35, 36, 36, 35), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  36,  0, 35, 36), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  0, 36, 35), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  71, 35, 35, 36), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 106, 36, 36, 35), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 107,  0, 35, 36), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  71,  0, 36, 35), // Right Selected
};

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2010_100[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(36,  0,  80,  40,  40,  40), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 36, 120,  40,  40,  40), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(36, 72, 120,   0,  40,  40), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(72, 36,  80,   0,  40,  40), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(36,  0,  80, 120,  40,  40), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 36, 120, 120,  40,  40), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(36, 72, 120,  80,  40,  40), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(72, 36,  80,  80,  40,  40), // Right Selected

	CXTPDockingPaneContextStickerWnd::SPRITEINFO(40, 40, 160, 120,  32,  32), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(40, 40, 192, 120,  32,  32), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 160,   0, 112, 112), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  40,  40,  40), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  40,  40,  40,  40), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  40,   0,  40,  40), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,   0,  40,  40), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 120,  40,  40), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  40, 120,  40,  40), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  40,  80,  40,  40), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  80,  40,  40), // Right Selected
};

#define arrSpritesStickerVisualStudio2012_100 arrSpritesStickerVisualStudio2010_100
#define arrSpritesStickerVisualStudio2015_100 arrSpritesStickerVisualStudio2010_100

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2010_125[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(45,  0, 100,  50,  50,  50), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 45, 150,  50,  50,  50), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(45, 90, 150,   0,  50,  50), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(90, 45, 100,   0,  50,  50), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(45,  0, 100, 150,  50,  50), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0, 45, 150, 150,  50,  50), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(45, 90, 150, 100,  50,  50), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(90, 45, 100, 100,  50,  50), // Right Selected

	CXTPDockingPaneContextStickerWnd::SPRITEINFO(50, 50, 200, 150,  40,  40), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(50, 50, 240, 150,  40,  40), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 200,   0, 140, 140), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  50,  50,  50), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  50,  50,  50,  50), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  50,   0,  50,  50), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,   0,  50,  50), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 150,  50,  50), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  50, 150,  50,  50), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  50, 100,  50,  50), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 100,  50,  50), // Right Selected
};

#define arrSpritesStickerVisualStudio2012_125 arrSpritesStickerVisualStudio2010_125
#define arrSpritesStickerVisualStudio2015_125 arrSpritesStickerVisualStudio2010_125

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2010_150[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 54,   0,   120,  60,  60,  60), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(  0,  54,   180,  60,  60,  60), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 54, 108,   180,   0,  60,  60), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(108,  54,   120,   0,  60,  60), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 54,   0,   120, 180,  60,  60), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(  0,  54,   180, 180,  60,  60), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 54, 108,   180, 120,  60,  60), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(108,  54,   120, 120,  60,  60), // Right Selected

	CXTPDockingPaneContextStickerWnd::SPRITEINFO(60, 60, 240, 180,  48,  48), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(60, 60, 288, 180,  48,  48), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 240,   0, 168, 168), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  60,  60,  60), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  60,  60,  60,  60), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  60,   0,  60,  60), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,   0,  60,  60), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 180,  60,  60), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  60, 180,  60,  60), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  60, 120,  60,  60), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 120,  60,  60), // Right Selected
};

#define arrSpritesStickerVisualStudio2012_150 arrSpritesStickerVisualStudio2010_150
#define arrSpritesStickerVisualStudio2015_150 arrSpritesStickerVisualStudio2010_150

static CXTPDockingPaneContextStickerWnd::SPRITEINFO arrSpritesStickerVisualStudio2010_200[] =
{
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(72,  0,   160,  80,  80,  80), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  72,  240,  80,  80,  80), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(72,  144, 240,   0,  80,  80), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(144, 72,  160,   0,  80,  80), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(72,  0,   160, 240,  80,  80), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  72,  240, 240,  80,  80), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(72,  144, 240, 160,  80,  80), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(144, 72,  160, 160,  80,  80), // Right Selected

	CXTPDockingPaneContextStickerWnd::SPRITEINFO(80, 80, 320, 240,  64,  64), // Center
	CXTPDockingPaneContextStickerWnd::SPRITEINFO(80, 80, 384, 240,  64,  64), // Center Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0, 320,   0, 224, 224), // Client

	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,  80,  80,  80), // Top
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  80,  80,  80,  80), // Left
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  80,   0,  80,  80), // Bottom
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0,   0,  80,  80), // Right
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 240,  80,  80), // Top Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  80, 240,  80,  80), // Left Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,  80, 160,  80,  80), // Bottom Selected
	CXTPDockingPaneContextStickerWnd::SPRITEINFO( 0,  0,   0, 160,  80,  80), // Right Selected
};

#define arrSpritesStickerVisualStudio2012_200 arrSpritesStickerVisualStudio2010_200
#define arrSpritesStickerVisualStudio2015_200 arrSpritesStickerVisualStudio2010_200

void CXTPDockingPaneContextStickerWnd::DrawTransparent(CDC* pDC , const CPoint& ptDest, const CSize& sz, CBitmap* pBitmap)
{
	CImageList il;
	il.Create(sz.cx, sz.cy, ILC_COLOR24 | ILC_MASK, 0, 1);
	il.Add(pBitmap, RGB(0, 0xFF, 0));

	il.Draw(pDC, 0, ptDest, ILD_NORMAL);
}

void CXTPDockingPaneContextStickerWnd::DrawSprite(CDC* pDC, UINT nID, SPRITEINFO* pSpriteInfo, BOOL bClientBitmap)
{
	CBitmap bmp;

	{
		CXTPResourceManager::CManageState state;
		HBITMAP hBitmap = CXTPImageManagerIcon::LoadBitmapFromResource(MAKEINTRESOURCE(nID), NULL);
		ASSERT(NULL != hBitmap); // Bitmap can not be loaded
		bmp.Attach(hBitmap);
	}

	CSize sz(pSpriteInfo->rcSrc.Width(), pSpriteInfo->rcSrc.Height());

	CBitmap bmpSprite;
	bmpSprite.CreateCompatibleBitmap(pDC, sz.cx, sz.cy);

	if (bmpSprite.GetSafeHandle())
	{
		CXTPCompatibleDC dcSprite(pDC, &bmpSprite);
		CXTPCompatibleDC dc(pDC, &bmp);
		dcSprite.BitBlt(0, 0, sz.cx, sz.cy, &dc, pSpriteInfo->rcSrc.left, pSpriteInfo->rcSrc.top, SRCCOPY);
	}

	CPoint ptDest = bClientBitmap ? pSpriteInfo->ptDest : CPoint(0, 0);

	DrawTransparent(pDC, ptDest, sz, &bmpSprite);
}

UINT CXTPDockingPaneContextStickerWnd::GetStickersID(XTPDockingContextStickerStyle nStyle)
{
	switch (nStyle)
	{
	case xtpPaneStickerStyleVisualStudio2005Beta:
		{
			switch (XTPColorManager()->GetCurrentSystemTheme())
			{
			case xtpSystemThemeBlue:   return XTP_IDB_DOCKINGPANE_STICKERS_BLUE;
			case xtpSystemThemeOlive:  return XTP_IDB_DOCKINGPANE_STICKERS_OLIVE;
			case xtpSystemThemeSilver: return XTP_IDB_DOCKINGPANE_STICKERS_SILVER;
			}
			return XTP_IDB_DOCKINGPANE_STICKERS_BLUE;
		}
	case xtpPaneStickerStyleVisualStudio2005:
		return XTPDpiHelper()->SelectDpiSpecific(XTP_IDB_DOCKINGPANE_STICKERS_VS_2005_100, XTP_IDB_DOCKINGPANE_STICKERS_VS_2005_125, XTP_IDB_DOCKINGPANE_STICKERS_VS_2005_150, XTP_IDB_DOCKINGPANE_STICKERS_VS_2005_200);

	case xtpPaneStickerStyleVisualStudio2008:      return XTP_IDB_DOCKINGPANE_STICKERS_2008;

	case xtpPaneStickerStyleVisualStudio2010:
	case xtpPaneStickerStyleVisualStudio2015Blue:
		return XTPDpiHelper()->SelectDpiSpecific(XTP_IDB_DOCKINGPANE_STICKERS_VS_BLUE_100, XTP_IDB_DOCKINGPANE_STICKERS_VS_BLUE_125, XTP_IDB_DOCKINGPANE_STICKERS_VS_BLUE_150, XTP_IDB_DOCKINGPANE_STICKERS_VS_BLUE_200);

	case xtpPaneStickerStyleVisualStudio2012Light:
	case xtpPaneStickerStyleVisualStudio2015Light:
		return XTPDpiHelper()->SelectDpiSpecific(XTP_IDB_DOCKINGPANE_STICKERS_VS_LIGHT_100, XTP_IDB_DOCKINGPANE_STICKERS_VS_LIGHT_125, XTP_IDB_DOCKINGPANE_STICKERS_VS_LIGHT_150, XTP_IDB_DOCKINGPANE_STICKERS_VS_LIGHT_200);
		
	case xtpPaneStickerStyleVisualStudio2012Dark:
	case xtpPaneStickerStyleVisualStudio2015Dark:
		return XTPDpiHelper()->SelectDpiSpecific(XTP_IDB_DOCKINGPANE_STICKERS_VS_DARK_100, XTP_IDB_DOCKINGPANE_STICKERS_VS_DARK_125, XTP_IDB_DOCKINGPANE_STICKERS_VS_DARK_150, XTP_IDB_DOCKINGPANE_STICKERS_VS_DARK_200);
	}
	return 0;
}

UINT CXTPDockingPaneContextStickerWnd::GetClientID(XTPDockingContextStickerStyle nStyle)
{
	if (nStyle == xtpPaneStickerStyleVisualStudio2005Beta)
	{
		return XTP_IDB_DOCKINGPANE_STICKER_CLIENT;
	}

	return GetStickersID(nStyle);
}

CXTPDockingPaneContextStickerWnd::SPRITEINFO* CXTPDockingPaneContextStickerWnd::GetSpriteInfo(XTPDockingContextStickerStyle nStyle)
{
	switch (nStyle)
	{
	case xtpPaneStickerStyleVisualStudio2005Beta:  return arrSpritesStickerVisualStudio2005Beta;
	case xtpPaneStickerStyleVisualStudio2005:
		return (CXTPDockingPaneContextStickerWnd::SPRITEINFO*)XTPDpiHelper()->SelectDpiSpecific(
			arrSpritesStickerVisualStudio2005_100, arrSpritesStickerVisualStudio2005_125, arrSpritesStickerVisualStudio2005_150, arrSpritesStickerVisualStudio2005_200);

	case xtpPaneStickerStyleVisualStudio2008:      return arrSpritesStickerVisualStudio2008_100;
	case xtpPaneStickerStyleVisualStudio2010:
		return (CXTPDockingPaneContextStickerWnd::SPRITEINFO*)XTPDpiHelper()->SelectDpiSpecific(
			arrSpritesStickerVisualStudio2010_100, arrSpritesStickerVisualStudio2010_125, arrSpritesStickerVisualStudio2010_150, arrSpritesStickerVisualStudio2010_200);
	
	case xtpPaneStickerStyleVisualStudio2012Light:
	case xtpPaneStickerStyleVisualStudio2012Dark:
		return (CXTPDockingPaneContextStickerWnd::SPRITEINFO*)XTPDpiHelper()->SelectDpiSpecific(
			arrSpritesStickerVisualStudio2012_100, arrSpritesStickerVisualStudio2012_125, arrSpritesStickerVisualStudio2012_150, arrSpritesStickerVisualStudio2012_200);
	
	case xtpPaneStickerStyleVisualStudio2015Light:
	case xtpPaneStickerStyleVisualStudio2015Dark:
	case xtpPaneStickerStyleVisualStudio2015Blue:
		return (CXTPDockingPaneContextStickerWnd::SPRITEINFO*)XTPDpiHelper()->SelectDpiSpecific(
			arrSpritesStickerVisualStudio2015_100, arrSpritesStickerVisualStudio2015_125, arrSpritesStickerVisualStudio2015_150, arrSpritesStickerVisualStudio2015_200);
	}
	return NULL;
}

void CXTPDockingPaneContextStickerWnd::OnDraw(CDC* pDC)
{
	BOOL bClient = (m_typeSticker & xtpPaneStickerClient) == xtpPaneStickerClient;
	XTPDockingContextStickerStyle nStyle = m_pContext->GetStickerStyle();
	int nClientSprite = bClient ? 0 : 11;
	UINT nIDBitmap = GetStickersID(nStyle);
	SPRITEINFO* pSprites = GetSpriteInfo(nStyle);
	
	if (nStyle == xtpPaneStickerStyleVisualStudio2005Beta)
	{
		if (bClient)
			DrawSprite(pDC, XTP_IDB_DOCKINGPANE_STICKER_CLIENT, &pSprites[xtpSpriteStickerClient]);
		if (m_typeSticker & xtpPaneStickerTop)
			DrawSprite(pDC, nIDBitmap, &pSprites[m_selectedSticker == xtpPaneStickerTop ? xtpSpriteStickerTopSelected : xtpSpriteStickerTop], bClient);
		if (m_typeSticker & xtpPaneStickerLeft)
			DrawSprite(pDC, nIDBitmap, &pSprites[m_selectedSticker == xtpPaneStickerLeft ? xtpSpriteStickerLeftSelected : xtpSpriteStickerLeft], bClient);
		if (m_typeSticker & xtpPaneStickerBottom)
			DrawSprite(pDC, nIDBitmap, &pSprites[m_selectedSticker == xtpPaneStickerBottom ? xtpSpriteStickerBottomSelected : xtpSpriteStickerBottom], bClient);
		if (m_typeSticker & xtpPaneStickerRight)
			DrawSprite(pDC, nIDBitmap, &pSprites[m_selectedSticker == xtpPaneStickerRight ? xtpSpriteStickerRightSelected : xtpSpriteStickerRight], bClient);
	}
	else
	{
		if (bClient)
			DrawSprite(pDC, nIDBitmap, &pSprites[m_selectedSticker == xtpPaneStickerCenter ? xtpSpriteStickerCenterSelected : xtpSpriteStickerClient]);
		if (m_typeSticker & xtpPaneStickerTop)
			DrawSprite(pDC, nIDBitmap, &pSprites[nClientSprite + (m_selectedSticker == xtpPaneStickerTop ? xtpSpriteStickerTopSelected : xtpSpriteStickerTop)], bClient);
		if (m_typeSticker & xtpPaneStickerLeft)
			DrawSprite(pDC, nIDBitmap, &pSprites[nClientSprite + (m_selectedSticker == xtpPaneStickerLeft ? xtpSpriteStickerLeftSelected : xtpSpriteStickerLeft)], bClient);
		if (m_typeSticker & xtpPaneStickerBottom)
			DrawSprite(pDC, nIDBitmap, &pSprites[nClientSprite + (m_selectedSticker == xtpPaneStickerBottom ? xtpSpriteStickerBottomSelected : xtpSpriteStickerBottom)], bClient);
		if (m_typeSticker & xtpPaneStickerRight)
			DrawSprite(pDC, nIDBitmap, &pSprites[nClientSprite + (m_selectedSticker == xtpPaneStickerRight ? xtpSpriteStickerRightSelected : xtpSpriteStickerRight)], bClient);
	}

	if (nStyle == xtpPaneStickerStyleVisualStudio2005)
	{
		if (m_typeSticker & xtpPaneStickerCenter)
			DrawSprite(pDC, nIDBitmap, &pSprites[xtpSpriteStickerCenter]);
	}
	else
	{
		if (m_typeSticker & xtpPaneStickerCenter)
			DrawSprite(pDC, nIDBitmap, &pSprites[m_selectedSticker == xtpPaneStickerCenter ? xtpSpriteStickerCenterSelected : xtpSpriteStickerCenter]);
	}
}

void CXTPDockingPaneContextStickerWnd::OnPaint()
{
	CPaintDC dcPaint(this); // device context for painting
	CXTPBufferDC dc(dcPaint, CXTPClientRect(this));
	OnDraw(&dc);
}

XTPDockingPaneStickerType CXTPDockingPaneContextStickerWnd::HitTest(CPoint pt)
{
	CXTPClientRect rc(this);
	ScreenToClient(&pt);

	if (!rc.PtInRect(pt))
		return xtpPaneStickerNone;

	CClientDC dcClient(this);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dcClient, rc.Width(), rc.Height());

	CXTPCompatibleDC dc(&dcClient, &bmp);
	dc.FillSolidRect(rc, 0);

	BOOL bClient = (m_typeSticker & xtpPaneStickerClient) == xtpPaneStickerClient;
	XTPDockingContextStickerStyle nStyle = m_pContext->GetStickerStyle();
	UINT nIDBitmap = GetStickersID(nStyle);
	SPRITEINFO* pSprites = GetSpriteInfo(nStyle);

	if (m_typeSticker & xtpPaneStickerTop)
	{
		DrawSprite(&dc, nIDBitmap, &pSprites[xtpSpriteStickerTop], bClient);
		if (dc.GetPixel(pt) != 0)
			return xtpPaneStickerTop;
	}
	if (m_typeSticker & xtpPaneStickerLeft)
	{
		DrawSprite(&dc, nIDBitmap, &pSprites[xtpSpriteStickerLeft], bClient);
		if (dc.GetPixel(pt) != 0)
			return xtpPaneStickerLeft;
	}
	if (m_typeSticker & xtpPaneStickerBottom)
	{
		DrawSprite(&dc, nIDBitmap, &pSprites[xtpSpriteStickerBottom], bClient);
		if (dc.GetPixel(pt) != 0)
			return xtpPaneStickerBottom;
	}
	if (m_typeSticker & xtpPaneStickerRight)
	{
		DrawSprite(&dc, nIDBitmap, &pSprites[xtpSpriteStickerRight], bClient);
		if (dc.GetPixel(pt) != 0)
			return xtpPaneStickerRight;
	}
	if (m_typeSticker & xtpPaneStickerCenter)
	{
		DrawSprite(&dc, GetClientID(nStyle), &pSprites[xtpSpriteStickerClient]);
		if (dc.GetPixel(pt) != 0)
			return xtpPaneStickerCenter;
	}

	return xtpPaneStickerNone;
}
