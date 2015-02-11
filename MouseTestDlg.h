#pragma once

#include "MouseInput.h"
#include <algorithm>
#include <map>
#include <vector>

class MousePoint
{
public:
	POINT point;
	COLORREF color;
	double delay;

	MousePoint()
	{
	}

	MousePoint(POINT p, COLORREF c, double d)
	{
		point = p;
		color = c;
		delay = d;
	}
};

typedef std::vector<MousePoint> MousePoints;
typedef std::map<COLORREF, HBRUSH> MouseBrushes;

class CMouseTestDlg : public CDialog
{
	DECLARE_MESSAGE_MAP()
	enum { IDD = IDD_MOUSETEST_DIALOG };
protected:
	MousePoints m_points;
	MouseBrushes m_brushes;
	MouseInput m_input;
	CRITICAL_SECTION m_critsect;
	POINT m_point;
	bool m_buttons;
	bool m_cursorHide;
	bool m_cursorShow;
	CStatic m_canvasBox;
	CButton m_clearBtn;
	CEdit m_rateEdt;
	CComboBox m_speedLst;
	CComboBox m_accelLst;
	CComboBox m_threshold1Lst;
	CComboBox m_threshold2Lst;
	CButton m_cursorChk;
public:
	CMouseTestDlg();
	virtual void DoDataExchange(CDataExchange* dx);
	virtual BOOL OnInitDialog();
	virtual void OnMouse(int dx, int dy, int dz, int btns, double delay);
	virtual void OnUpdateRate();
	virtual void OnUpdateSpeed();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnDrawItem(int id, LPDRAWITEMSTRUCT data);
	afx_msg void OnSettingChange(UINT flags, LPCTSTR section);
	afx_msg void OnTimer(UINT_PTR id);
	afx_msg void OnCursorHide();
	afx_msg void OnCursorShow();
	afx_msg void OnBnClickedClearBtn();
	afx_msg void OnCbnSelchangeSpeedLst();
	afx_msg void OnCbnSelchangeAccelLst();
};
