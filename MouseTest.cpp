#include "stdafx.h"
#include "MouseTest.h"
#include "MouseTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMouseTestApp, CWinApp)
END_MESSAGE_MAP()

CMouseTestApp theApp;

CMouseTestApp::CMouseTestApp()
{
}

BOOL CMouseTestApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	CMouseTestDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
