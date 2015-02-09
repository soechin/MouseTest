#pragma once

#include "MouseInput.h"
#include <map>
#include <vector>
#include "afxwin.h"

class CMouseTestDlg : public CDialog
{
	DECLARE_MESSAGE_MAP()
protected:
	enum { IDD = IDD_MOUSETEST_DIALOG };
	std::vector<std::pair<POINT, COLORREF>> m_list;
	CCriticalSection m_critsect;
	MouseInput m_input;
	bool m_cursor;
	bool m_first;
	double m_delay;
	CPoint m_point;
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
