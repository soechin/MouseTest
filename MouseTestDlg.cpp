#include "stdafx.h"
#include "MouseTest.h"
#include "MouseTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_CURSOR_HIDE (WM_APP + 1)
#define WM_CURSOR_SHOW (WM_APP + 2)

BEGIN_MESSAGE_MAP(CMouseTestDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_DRAWITEM()
	ON_WM_HSCROLL()
	ON_WM_SETTINGCHANGE()
	ON_WM_TIMER()
	ON_MESSAGE_VOID(WM_CURSOR_HIDE, OnCursorHide)
	ON_MESSAGE_VOID(WM_CURSOR_SHOW, OnCursorShow)
	ON_BN_CLICKED(IDC_CLEAR_BTN, OnBnClickedClearBtn)
	ON_CBN_SELCHANGE(IDC_SPEED_LST, OnCbnSelchangeSpeedLst)
	ON_CBN_SELCHANGE(IDC_ACCEL_LST, OnCbnSelchangeAccelLst)
	ON_CBN_SELCHANGE(IDC_THRESHOLD1_LST, OnCbnSelchangeAccelLst)
	ON_CBN_SELCHANGE(IDC_THRESHOLD2_LST, OnCbnSelchangeAccelLst)
END_MESSAGE_MAP()

void __stdcall MouseCallback(int dx, int dy, int dz, int btns,
	double delay, void* param)
{
	CMouseTestDlg* dlg = (CMouseTestDlg*)param;

	if (dlg != NULL && dlg->GetSafeHwnd() == GetForegroundWindow())
	{
		dlg->OnMouse(dx, dy, dz, btns, delay);
	}
}

CMouseTestDlg::CMouseTestDlg() : CDialog(IDD, NULL)
{
}

void CMouseTestDlg::DoDataExchange(CDataExchange* dx)
{
	CDialog::DoDataExchange(dx);
	DDX_Control(dx, IDC_CANVAS_BOX, m_canvasBox);
	DDX_Control(dx, IDC_CLEAR_BTN, m_clearBtn);
	DDX_Control(dx, IDC_RATE_EDT, m_rateEdt);
	DDX_Control(dx, IDC_SPEED_LST, m_speedLst);
	DDX_Control(dx, IDC_ACCEL_LST, m_accelLst);
	DDX_Control(dx, IDC_THRESHOLD1_LST, m_threshold1Lst);
	DDX_Control(dx, IDC_THRESHOLD2_LST, m_threshold2Lst);
	DDX_Control(dx, IDC_CURSOR_CHK, m_cursorChk);
}

BOOL CMouseTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);

	GetCursorPos(&m_point);
	m_cursor = true;
	m_first = true;
	m_delay = 0;

	m_cursorChk.SetCheck(TRUE);
	m_input.open(NULL, MouseCallback, this);

	OnUpdateRate();
	OnUpdateSpeed();
	SetTimer(1, USER_TIMER_MINIMUM, NULL);

	return TRUE;
}

void CMouseTestDlg::OnMouse(int dx, int dy, int dz, int btns,
	double delay)
{
	CSingleLock critsect(&m_critsect, true);
	CDC* pdc;
	CBrush brush;
	CRect rect;
	CPoint point;
	COLORREF color;
	bool first;

	// check button down
	if (btns == 0)
	{
		m_first = true;

		// show cursor
		if (!m_cursor)
		{
			PostMessage(WM_CURSOR_SHOW);
		}

		return;
	}

	// check first move
	first = false;

	if (m_first)
	{
		GetCursorPos(&m_point);
		m_first = false;

		dx = 0;
		dy = 0;
		dz = 0;

		first = true;
	}

	// offset raw point(screen coord)
	m_point.Offset(dx, dy);
	m_canvasBox.GetWindowRect(&rect);

	if (!rect.PtInRect(m_point))
	{
		return;
	}

	point = m_point;
	m_canvasBox.ScreenToClient(&point);
	m_canvasBox.ScreenToClient(&rect);

	// hide cursor
	if (m_cursor && first)
	{
		PostMessage(WM_CURSOR_HIDE);
	}

	// set point color
	color = 0;
	if (btns & MK_LBUTTON) color |= 0x0000ff;
	if (btns & MK_RBUTTON) color |= 0xff0000;
	if (btns & MK_MBUTTON) color |= 0x00ff00;

	// save point
	m_list.push_back(std::make_pair(point, color));

	// response delay (running average)
	m_delay = first ? delay : (m_delay * 0.9 + delay * 0.1);

	// draw point
	pdc = m_canvasBox.GetDC();
	brush.CreateSolidBrush(color);
	rect.left = __max(rect.left, point.x - 1);
	rect.top = __max(rect.top, point.y - 1);
	rect.right = __min(rect.right, point.x + 1);
	rect.bottom = __min(rect.bottom, point.y + 1);
	pdc->FillRect(&rect, &brush);
	m_canvasBox.ReleaseDC(pdc);
}

void CMouseTestDlg::OnUpdateRate()
{
	CSingleLock critsect(&m_critsect, true);
	CString text;
	double rate;

	rate = (m_delay != 0) ? (1 / m_delay) : 0;
	text.Format(TEXT("%.0lf"), rate);
	m_rateEdt.SetWindowText(text);
}

void CMouseTestDlg::OnUpdateSpeed()
{
	CSingleLock critsect(&m_critsect, true);
	CString text;
	UINT speed, accel[3];

	// speed
	SystemParametersInfo(SPI_GETMOUSESPEED, 0, &speed, 0);
	m_speedLst.SetCurSel(speed >> 1);

	// accel
	SystemParametersInfo(SPI_GETMOUSE, 0, accel, 0);
	m_threshold1Lst.SetCurSel(accel[0] >> 1);
	m_threshold2Lst.SetCurSel(accel[1] >> 1);
	m_accelLst.SetCurSel(accel[2]);
}

void CMouseTestDlg::OnOK()
{
}

void CMouseTestDlg::OnCancel()
{
}

void CMouseTestDlg::OnClose()
{
	DestroyWindow();
	CDialog::OnClose();
}

void CMouseTestDlg::OnDestroy()
{
	KillTimer(1);
	m_input.close();

	CDialog::OnDestroy();
}

void CMouseTestDlg::OnDrawItem(int id, LPDRAWITEMSTRUCT data)
{
	if (id == m_canvasBox.GetDlgCtrlID())
	{
		std::map<COLORREF, CBrush> brushes;
		CSingleLock critsect(&m_critsect, true);
		CDC dc;
		CBrush brush;
		CRect rect, draw;
		CPoint point;
		COLORREF color;

		dc.Attach(data->hDC);
		draw = data->rcItem;

		brush.CreateSolidBrush(0x000000);
		dc.FillRect(&draw, &brush);

		for (int i = 0; i < (int)m_list.size(); i++)
		{
			point = m_list[i].first;
			color = m_list[i].second;
			if (!draw.PtInRect(point)) continue;

			if (brushes.find(color) == brushes.end())
			{
				brushes[color].CreateSolidBrush(color);
			}

			rect.left = __max(draw.left, point.x - 1);
			rect.top = __max(draw.top, point.y - 1);
			rect.right = __min(draw.right, point.x + 1);
			rect.bottom = __min(draw.bottom, point.y + 1);
			dc.FillRect(&rect, &brushes[color]);
		}

		brushes.clear();
		dc.Detach();
	}
	else
	{
		CDialog::OnDrawItem(id, data);
	}
}

void CMouseTestDlg::OnSettingChange(UINT flags, LPCTSTR section)
{
	CDialog::OnSettingChange(flags, section);

	if (flags == SPI_SETMOUSESPEED || flags == SPI_SETMOUSE)
	{
		OnUpdateSpeed();
	}
}

void CMouseTestDlg::OnTimer(UINT_PTR id)
{
	if (id == 1)
	{
		OnUpdateRate();
	}

	CDialog::OnTimer(id);
}

void CMouseTestDlg::OnCursorHide()
{
	CSingleLock critsect(&m_critsect, true);

	if (m_cursor)
	{
		if (m_cursorChk.GetCheck() == 0)
		{
			ShowCursor(FALSE);
			m_cursor = false;
		}
	}
}

void CMouseTestDlg::OnCursorShow()
{
	CSingleLock critsect(&m_critsect, true);

	if (!m_cursor)
	{
		ShowCursor(TRUE);
		m_cursor = true;
	}
}

void CMouseTestDlg::OnBnClickedClearBtn()
{
	CSingleLock critsect(&m_critsect, true);

	m_list.clear();
	m_delay = 0;
	m_canvasBox.InvalidateRect(NULL, TRUE);
}

void CMouseTestDlg::OnCbnSelchangeSpeedLst()
{
	CSingleLock critsect(&m_critsect, true);
	CString text;
	UINT speed;

	m_speedLst.GetWindowText(text);
	speed = _ttoi(text);
	SystemParametersInfo(SPI_SETMOUSESPEED, 0, (void*)speed, SPIF_SENDCHANGE);
	OnUpdateSpeed();
}

void CMouseTestDlg::OnCbnSelchangeAccelLst()
{
	CSingleLock critsect(&m_critsect, true);
	CString text;
	UINT accel[3];

	if (SystemParametersInfo(SPI_GETMOUSE, 0, accel, 0))
	{
		m_threshold1Lst.GetWindowText(text);
		accel[0] = _ttoi(text);
		m_threshold2Lst.GetWindowText(text);
		accel[1] = _ttoi(text);
		m_accelLst.GetWindowText(text);
		accel[2] = _ttoi(text);

		SystemParametersInfo(SPI_SETMOUSE, 0, accel, SPIF_SENDCHANGE);
		OnUpdateSpeed();
	}
}
