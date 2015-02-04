#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"

class CMouseTestApp : public CWinApp
{
	DECLARE_MESSAGE_MAP()
public:
	CMouseTestApp();
	virtual BOOL InitInstance();
};

extern CMouseTestApp theApp;
