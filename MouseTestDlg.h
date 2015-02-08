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
	bool m_first;
	double m_delay;
	CPoint m_point;
	CStatic m_canvas;
	CEdit m_rateEdt;
	CSliderCtrl m_speedSli;
	CStatic m_speedTxt;
	CButton m_accelChk;
	CEdit m_accelEdt;
public:
	CMouseTestDlg();
	virtual void DoDataExchange(CDataExchange* dx);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnMouse(int dx, int dy, int dz, int btns, double delay);
	virtual void OnUpdateRate();
	virtual void OnUpdateSpeed();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnDrawItem(int id, LPDRAWITEMSTRUCT data);
	afx_msg void OnHScroll(UINT code, UINT pos, CScrollBar* scroll);
	afx_msg void OnSettingChange(UINT flags, LPCTSTR section);
	afx_msg void OnTimer(UINT_PTR id);
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedAccelChk();
};
