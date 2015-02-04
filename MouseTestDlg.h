#pragma once

#include "MouseInput.h"
#include <map>
#include <vector>

class CMouseTestDlg : public CDialog
{
	DECLARE_MESSAGE_MAP()
protected:
	enum { IDD = IDD_MOUSETEST_DIALOG };
	std::vector<std::pair<POINT, COLORREF>> m_list;
	CCriticalSection m_critsect;
	MouseInput m_input;
	int m_move;
	double m_time, m_rate;
	CPoint m_point;
	CStatic m_canvas;
	CEdit m_rateTxt;
public:
	CMouseTestDlg();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnMouse(int dx, int dy, int dz, int btns, double time);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnDrawItem(int id, LPDRAWITEMSTRUCT data);
	afx_msg void OnTimer(UINT_PTR id);
	afx_msg void OnBnClickedClear();
};
