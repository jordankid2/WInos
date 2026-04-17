// XTPChartOpenGLDeviceContext.cpp
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

#include "stdafx.h"

#include "Common/Base/Diagnostic/XTPDisableAdvancedWarnings.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <ActivScp.h>
#include "GraphicLibrary/OpenGL/GL.h"
#include "GraphicLibrary/OpenGL/GLU.h"
#include "Common/Base/Diagnostic/XTPEnableAdvancedWarnings.h"

#include "GraphicLibrary/GdiPlus/XTPGdiPlus.h"

#include "Common/XTPFramework.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPGdiObjects.h"
#include "Common/Base/Types/XTPPoint2.h"
#include "Common/Base/Types/XTPPoint3.h"
#include "Common/Math/XTPMatrix.h"

#include "GraphicLibrary/OpenGL/XTPOpenGLUtils.h"

#include "Common/XTPSingleton.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPResourceManager.h"
#include "Common/XTPIntel80Helpers.h"
#include "Common/Base/Types/XTPPoint2.h"
#include "Common/Base/Types/XTPPoint3.h"
#include "Common/Base/Types/XTPSize.h"
#include "Common/Base/Types/XTPRect.h"
#include "Common/XTPTypeId.h"

#include "Markup/XTPMarkupObject.h"
#include "Markup/XTPMarkupContext.h"
#include "Markup/XTPMarkupInputElement.h"
#include "Markup/XTPMarkupUIElement.h"
#include "Markup/XTPMarkupDrawingContext.h"

#include "Chart/Types/XTPChartTypes.h"
#include "Chart/XTPChartDefines.h"
#include "Chart/XTPChartElement.h"
#include "Chart/XTPChartElementView.h"
#include "Chart/XTPChartPanel.h"
#include "Chart/XTPChartDiagram.h"
#include "Chart/Types/XTPChartTypes.h"

#include "Chart/Drawing/XTPChartDeviceContext.h"
#include "Chart/Drawing/XTPChartDeviceCommand.h"
#include "Chart/Drawing/OpenGL/XTPChartOpenGLDeviceContext.h"

#include "Chart/XTPChartObjectFactory.h"
#include "Chart/Drawing/XTPChartDrawingObjectFactory.h"

#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "OpenGL32.Lib")


#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
// CXTPChartOpenGLDeviceContext

IMPLEMENT_DYNAMIC(CXTPChartOpenGLDeviceContext, CXTPChart3dDeviceContext);

CXTPChartOpenGLDeviceContext::CXTPChartOpenGLDeviceContext(CXTPChartContainer* pContainer, HDC hDC, CRect rcBounds)
	: CXTPChart3dDeviceContext(pContainer, hDC, rcBounds, XTPChartObjectFactory()->Get3dDrawingObjectFactory())
	, m_hGLContext(NULL)
	, m_nExecuteNesting(0)
	, m_bHave2dCommands(FALSE)
	, m_bIn2dCommandExecutionMode(FALSE)
	, m_p2dDC(NULL)
{
}

CXTPChartOpenGLDeviceContext::~CXTPChartOpenGLDeviceContext()
{
	if (NULL != m_hGLContext)
	{
		wglDeleteContext(m_hGLContext);
	}
}

void CXTPChartOpenGLDeviceContext::Execute(CXTPChartDeviceCommand* pCommand)
{
	ASSERT_VALID(pCommand);

	++m_nExecuteNesting;

	if (1 == m_nExecuteNesting)
	{
		Execute3dCommandTree(pCommand);
		Execute2dCommandTree(pCommand);
	}
	else if (!m_bIn2dCommandExecutionMode)
	{
		m_bHave2dCommands |= !pCommand->Is3D();
		pCommand->Execute(this);
	}
	else
	{
		pCommand->Execute(m_p2dDC);
	}

	--m_nExecuteNesting;
	ASSERT(0 <= m_nExecuteNesting);
}

BOOL CXTPChartOpenGLDeviceContext::Initialize()
{
	BOOL bSuccess = FALSE;

	if (InitializePixelFormat())
	{
		m_hGLContext = ::wglCreateContext(GetHDC());
		if (NULL != m_hGLContext)
		{
			bSuccess = ::wglMakeCurrent(GetHDC(), m_hGLContext);
			if (bSuccess)
			{
				SetOpenGLParameters();
				::wglMakeCurrent(NULL, NULL);
			}
			else
			{
				TRACE(_T("Cannot activate WGL context [error %i]\n"), GetLastError());
			}
		}
		else
		{
			TRACE(_T("Cannot create WGL context [error %i]\n"), GetLastError());
		}
	}
	else
	{
		TRACE(_T("Cannot setup pixel format [error %i]\n"), GetLastError());
	}

	return bSuccess;
}

CXTPChartSizeF CXTPChartOpenGLDeviceContext::MeasureString(const CXTPChartString* pText, CXTPChartFont* pFont)
{
	// The text will always be rendered using 2D device context so it has to be measure using 2D DC as well.

	CXTPChartSizeF size;

	if (NULL == m_p2dDC)
	{
		CXTPChartDeviceContext* p2dDC = CXTPChartDrawingObjectFactory::Create2dDeviceContext(
			GetContainer(), GetHDC(), this);
		size = p2dDC->MeasureString(pText, pFont);
		delete p2dDC;
	}
	else
	{
		size = m_p2dDC->MeasureString(pText, pFont);
	}

	return size;
}

CXTPChartSizeF CXTPChartOpenGLDeviceContext::MeasureMarkupElement(CXTPMarkupUIElement* pMarkupUIElement, CXTPChartFont* pFont)
{
	ASSERT_VALID(pFont);

	CXTPChartSizeF size(0, 0);
	if (NULL != pMarkupUIElement)
	{
		pMarkupUIElement->GetMarkupContext()->SetDefaultFont(&pFont->m_lf);

		CXTPMarkupDrawingContext dc(pMarkupUIElement->GetMarkupContext(), GetHDC());
		pMarkupUIElement->Measure(&dc, CSize(INT_MAX, INT_MAX));

		CSize sz = pMarkupUIElement->GetDesiredSize();
		size.Width = static_cast<float>(sz.cx);
		size.Height = static_cast<float>(sz.cy);
	}

	return size;
}

void CXTPChartOpenGLDeviceContext::Rotate(const CXTPChart3dRotation& rotation)
{
	CXTPOpenGLUtils::Rotate(rotation.m_dYaw, rotation.m_dPitch, rotation.m_dRoll);
}

BOOL CXTPChartOpenGLDeviceContext::Project(const CXTPPoint3d& pt3d, CPoint& pt2d, double* pdWinZ /*= NULL*/)
{
	return CXTPOpenGLUtils::Project(pt3d, pt2d, pdWinZ);
}

BOOL CXTPChartOpenGLDeviceContext::Project(const CXTPChart3dBox& box, CRect& rc)
{
	BOOL bSuccess = TRUE;

	CXTPPoint3d boxCorners[8];
	box.GetCorners(boxCorners);

	rc = CRect(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
	for (int i = 0; i < _countof(boxCorners); ++i)
	{
		CPoint pt;
		if (!Project(boxCorners[i], pt))
		{
			bSuccess = FALSE;
			break;
		}

		if (pt.x < rc.left)
			rc.left = pt.x;
		if (rc.right < pt.x)
			rc.right = pt.x;

		if (pt.y < rc.top)
			rc.top = pt.y;
		if (rc.bottom < pt.y)
			rc.bottom = pt.y;
	}

	return bSuccess;
}

BOOL CXTPChartOpenGLDeviceContext::Unproject(const CPoint& pt2d, CXTPPoint3d& pt3d, double dWinZ /*= 0.*/)
{
	return CXTPOpenGLUtils::Unproject(pt2d, pt3d, dWinZ);
}

void CXTPChartOpenGLDeviceContext::Transform(CXTPPoint3d& v, const CXTPMatrix<double>& matrix)
{
	CXTPOpenGLUtils::MultiplyByMatrix(v, matrix);
}

void CXTPChartOpenGLDeviceContext::Transform(CXTPChart3dBox& box, const CXTPMatrix<double>& matrix)
{
	CXTPChart3dBox resultBox;
	CXTPPoint3d boxCorners[8];
	box.GetCorners(boxCorners);
	for (int i = 0; i < _countof(boxCorners); ++i)
	{
		CXTPOpenGLUtils::MultiplyByMatrix(boxCorners[i], matrix);
		resultBox.Include(boxCorners[i]);
	}

	const CXTPChart3dBox::PointList& centerCandidates = box.GetCenterCandidates();
	POSITION pos = centerCandidates.GetHeadPosition();
	while (NULL != pos)
	{
		CXTPPoint3d pt = centerCandidates.GetNext(pos);
		CXTPOpenGLUtils::MultiplyByMatrix(pt, matrix);
		resultBox.Include(pt, TRUE);
	}

	box = resultBox;
}

void CXTPChartOpenGLDeviceContext::Transform(CXTPPoint3d& v,
	const CXTPPoint3d* pTranslation /*= NULL*/,
	const CXTPChart3dRotation* pRotation /*= NULL*/)
{
#ifdef _DEBUG
	GLboolean listIndex;
	glGetBooleanv(GL_LIST_INDEX, &listIndex);
	ASSERT(0 == listIndex && "A list in currently under construction hence model view matrix is not affected");
#endif

	if (NULL != pTranslation || NULL != pRotation)
	{
		CXTPMatrix<double>* pMatrix = BuildTransformationMatrix(pTranslation, pRotation);
		Transform(v, *pMatrix);
		delete pMatrix;
	}
}

void CXTPChartOpenGLDeviceContext::Transform(CXTPChart3dBox& box,
	const CXTPPoint3d* pTranslation /*= NULL*/,
	const CXTPChart3dRotation* pRotation /*= NULL*/)
{
#ifdef _DEBUG
	GLboolean listIndex;
	glGetBooleanv(GL_LIST_INDEX, &listIndex);
	ASSERT(0 == listIndex && "A list in currently under construction hence model view matrix is not affected");
#endif

	if (NULL != pTranslation || NULL != pRotation)
	{
		CXTPMatrix<double>* pMatrix = BuildTransformationMatrix(pTranslation, pRotation);
		Transform(box, *pMatrix);
		delete pMatrix;
	}
}

BOOL CXTPChartOpenGLDeviceContext::DepthTest(CPoint point, double& dWinZ)
{
	BOOL bEnabled = glIsEnabled(GL_DEPTH_TEST);
	ASSERT(bEnabled);

	if (bEnabled)
	{
		float z = 0;
		glReadPixels(point.x, point.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
		dWinZ = z;
	}

	return bEnabled;
}

BOOL CXTPChartOpenGLDeviceContext::InitializePixelFormat()
{
	BOOL bSuccess = FALSE;

	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int nFormat = ::ChoosePixelFormat(GetHDC(), &pfd);
	if (0 != nFormat)
	{
		bSuccess = ::SetPixelFormat(GetHDC(), nFormat, &pfd);
	}

	return bSuccess;
}

void CXTPChartOpenGLDeviceContext::SetOpenGLParameters()
{
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1);
	glViewport(0, 0, m_rcBounds.Width(), m_rcBounds.Height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLfloat fAspect = m_rcBounds.IsRectEmpty() ? 1.f
		: static_cast<GLfloat>(m_rcBounds.Width()) / m_rcBounds.Height();
	gluPerspective(45.0f, fAspect, 1.0f, CXTPChart3dDiagram::m_fChartBoxSize * 10);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	const GLfloat lightPosition[] =
	{
		static_cast<float>(m_ptLightPosition.X),
		static_cast<float>(m_ptLightPosition.Y),
		static_cast<float>(m_ptLightPosition.Z),
		0
	};
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	const GLfloat ambient[] =
	{
		m_clrLightAmbientColor.ClampR(),
		m_clrLightAmbientColor.ClampG(),
		m_clrLightAmbientColor.ClampB(),
		m_clrLightAmbientColor.ClampA(),
	};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	const GLfloat diffuse[] =
	{
		m_clrLightDiffuseColor.ClampR(),
		m_clrLightDiffuseColor.ClampG(),
		m_clrLightDiffuseColor.ClampB(),
		m_clrLightDiffuseColor.ClampA(),
	};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	const GLfloat specular[] =
	{
		m_clrLightSpecularColor.ClampR(),
		m_clrLightSpecularColor.ClampG(),
		m_clrLightSpecularColor.ClampB(),
		m_clrLightSpecularColor.ClampA(),
	};
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	const GLfloat lightModelAmbient[] =
	{
		m_clrLightModelAmbientColor.ClampR(),
		m_clrLightModelAmbientColor.ClampG(),
		m_clrLightModelAmbientColor.ClampB(),
		m_clrLightModelAmbientColor.ClampA(),
	};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmbient);

	static const GLfloat emission[] =
	{
		m_clrMaterialEmissionColor.ClampR(),
		m_clrMaterialEmissionColor.ClampG(),
		m_clrMaterialEmissionColor.ClampB(),
		m_clrMaterialEmissionColor.ClampA(),
	};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);

	const GLfloat diffuseMaterial[] =
	{
		m_clrMaterialDiffuseColor.ClampR(),
		m_clrMaterialDiffuseColor.ClampG(),
		m_clrMaterialDiffuseColor.ClampB(),
		m_clrMaterialDiffuseColor.ClampA(),
	};
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseMaterial);

	const GLfloat specularMaterial[] =
	{
		m_clrMaterialSpecularColor.ClampR(),
		m_clrMaterialSpecularColor.ClampG(),
		m_clrMaterialSpecularColor.ClampB(),
		m_clrMaterialSpecularColor.ClampA(),
	};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularMaterial);

	const GLfloat shininess[] = { 128.f * m_fMaterialShininess };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	glClearColor(0, 0, 0, 0);
	glClearAccum(0, 0, 0, 0);
}

BOOL CXTPChartOpenGLDeviceContext::OnPreRender()
{
	BOOL bSuccess = ::wglMakeCurrent(GetHDC(), m_hGLContext);
	if (bSuccess)
	{
		ResetSavedCameraState();

		SetOpenGLParameters();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	}

	return bSuccess;
}

void CXTPChartOpenGLDeviceContext::OnPostRender()
{
	glFinish();
	glFlush();

	::wglMakeCurrent(NULL, NULL);
}

double CXTPChartOpenGLDeviceContext::ComputeAntialiasingJitterFactor() const
{
	double dFactor = 0.2; // default

	if (0.f != m_fMinModelDistance && 0.f != m_fMaxModelDistance)
	{
		// Compute jitter factor that depends on camera distance and viewport size.
		double dCameraDistance = sqrt((m_cameraState.ptPosition.X - m_cameraState.ptFocal.X) * (m_cameraState.ptPosition.X - m_cameraState.ptFocal.X)
			+ (m_cameraState.ptPosition.Y - m_cameraState.ptFocal.Y) * (m_cameraState.ptPosition.Y - m_cameraState.ptFocal.Y)
			+ (m_cameraState.ptPosition.Z - m_cameraState.ptFocal.Z) * (m_cameraState.ptPosition.Z - m_cameraState.ptFocal.Z));

		GLint arViewport[4];
		glGetIntegerv(GL_VIEWPORT, arViewport);
		double dMinSide = __min(arViewport[2], arViewport[3]);

		// The computational formula approximates factor values between test extreme values
		// extracted manually.
		dFactor = sqrt((((dCameraDistance / 100) - .8) * .1) * (.5 / (dMinSide / 100)));
	}

	return dFactor;
}

void CXTPChartOpenGLDeviceContext::Execute3dCommandTreeAntialiased(CXTPChartDeviceCommand* pCommand)
{
	m_bRenderingAntialiasing = TRUE;

	static const double dDiag = sqrt(8.) / 2; // hypotenuse of a right triangle with sides 1 and 1
	static const CXTPPoint3d arJitter[] =
	{
		CXTPPoint3d(1, -1, 1),
		CXTPPoint3d(1, 1, 1),

		CXTPPoint3d(-1, -1, 1),
		CXTPPoint3d(-1, 1, 1),

		CXTPPoint3d(-1, -1, -1),
		CXTPPoint3d(-1, 1, -1),

		CXTPPoint3d(1, -1, -1),
		CXTPPoint3d(1, 1, -1),

		CXTPPoint3d(0, -dDiag, 0),
		CXTPPoint3d(0, dDiag, 0),

		CXTPPoint3d(-dDiag, 0, 0),
		CXTPPoint3d(dDiag, 0, 0),

		CXTPPoint3d(0, 0, -dDiag),
		CXTPPoint3d(0, 0, dDiag),
	};

	static const float fAccumStep = 1.f / (1 + _countof(arJitter));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	pCommand->Execute(this);
	glAccum(GL_LOAD, fAccumStep);

	double dJitterFactor = ComputeAntialiasingJitterFactor();
	for (int i = 0; i < _countof(arJitter); ++i)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslated(
			arJitter[i].X * dJitterFactor,
			arJitter[i].Y * dJitterFactor,
			arJitter[i].Z * dJitterFactor);
		pCommand->Execute(this);
		glPopMatrix();

		glAccum(GL_ACCUM, fAccumStep);
	}

	glAccum(GL_RETURN, 1.f);
	glPopMatrix();

	m_bRenderingAntialiasing = FALSE;
}

void CXTPChartOpenGLDeviceContext::Execute3dCommandTree(CXTPChartDeviceCommand* pCommand)
{
	ASSERT_VALID(pCommand);
	ASSERT(NULL == m_p2dDC);

	OnPreRender();

	pCommand->Prepare(this);

	BOOL bBeingFrequentlyUpdated = GetContainer()->IsBeingFrequentlyUpdated();
	if (bBeingFrequentlyUpdated)
	{
		m_bSkippingAntialiasing = TRUE;
	}
	else
	{
		Execute3dCommandTreeAntialiased(pCommand);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	pCommand->Execute(this);
	glPopMatrix();

	if (bBeingFrequentlyUpdated)
	{
		m_bSkippingAntialiasing = FALSE;
	}

	OnPostRender();
}

void CXTPChartOpenGLDeviceContext::Execute2dCommandTree(CXTPChartDeviceCommand* pCommand)
{
	ASSERT_VALID(pCommand);
	ASSERT(NULL == m_p2dDC);

	if (m_bHave2dCommands)
	{
		m_bIn2dCommandExecutionMode = TRUE;

		m_p2dDC = CXTPChartDrawingObjectFactory::Create2dDeviceContext(
			GetContainer(), GetHDC(), this);

		pCommand->Prepare(m_p2dDC);
		pCommand->Execute(m_p2dDC);

		delete m_p2dDC;
		m_p2dDC = NULL;

		m_bIn2dCommandExecutionMode = FALSE;
		m_bHave2dCommands = FALSE;

		GdiFlush();
	}
}

CXTPMatrix<double>* CXTPChartOpenGLDeviceContext::BuildTransformationMatrix(
	const CXTPPoint3d* pTranslation /*= NULL*/,
	const CXTPChart3dRotation* pRotation /*= NULL*/)
{
	CXTPMatrix<double>* pMatrix = new CXTPMatrix<double>(4, 4);
	glPushMatrix();
	glLoadIdentity();

	if (NULL != pRotation)
	{
		CXTPOpenGLUtils::Rotate(pRotation->m_dYaw, pRotation->m_dPitch, pRotation->m_dRoll);
	}

	if (NULL != pTranslation)
	{
		glTranslated(pTranslation->X, pTranslation->Y, pTranslation->Z);
	}

	glGetDoublev(GL_MODELVIEW_MATRIX, pMatrix->GetData());
	glPopMatrix();

	return pMatrix;
}
