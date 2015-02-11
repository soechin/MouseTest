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

	if (dlg != NULL)
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

	InitializeCriticalSection(&m_critsect);
	SetTimer(1, USER_TIMER_MINIMUM, NULL);

	m_buttons = false;
	m_cursorShow = true;
	m_cursorHide = false;

	m_cursorChk.SetCheck(TRUE);
	m_input.open(NULL, MouseCallback, this);

	OnUpdateRate();
	OnUpdateSpeed();

	return TRUE;
}

void CMouseTestDlg::OnMouse(int dx, int dy, int dz, int btns,
	double delay)
{
	HWND hwnd;
	HDC hdc;
	POINT point;
	RECT rect;
	COLORREF color;

	// check foreground
	if (::GetForegroundWindow() != GetSafeHwnd())
	{
		return;
	}

	EnterCriticalSection(&m_critsect);
	color = 0;

	// check button state
	if (btns == 0)
	{
		goto OnExit;
	}

	// check previous button state
	if (!m_buttons)
	{
		GetCursorPos(&m_point);

		dx = 0;
		dy = 0;
	}

	// offset cursor position
	m_point.x += dx;
	m_point.y += dy;

	// check cursor position
	hwnd = m_canvasBox.GetSafeHwnd();
	point = m_point;

	::ScreenToClient(hwnd, &point);
	::GetClientRect(hwnd, &rect);

	if (!PtInRect(&rect, point))
	{
		goto OnExit;
	}

	// point color
	if (btns & MK_LBUTTON) color |= 0x0000ff;
	if (btns & MK_RBUTTON) color |= 0xff0000;
	if (btns & MK_MBUTTON) color |= 0x00ff00;

	// backup point
	m_points.push_back(MousePoint(point, color, delay));

	// draw point
	hdc = ::GetDC(hwnd);

	if (m_brushes.find(color) == m_brushes.end())
	{
		m_brushes[color] = CreateSolidBrush(color);
	}

	rect.left = __max(rect.left, point.x - 1);
	rect.top = __max(rect.top, point.y - 1);
	rect.right = __min(rect.right, point.x + 1);
	rect.bottom = __min(rect.bottom, point.y + 1);

	FillRect(hdc, &rect, m_brushes[color]);
	::ReleaseDC(hwnd, hdc);
OnExit:
	// backup button state
	m_buttons = btns != 0;

	// show cursor
	if (m_cursorHide && color == 0)
	{
		PostMessage(WM_CURSOR_SHOW, 0, 0);
		m_cursorHide = false;
	}
	// hide cursor
	else if (!m_cursorHide && color != 0)
	{
		PostMessage(WM_CURSOR_HIDE, 0, 0);
		m_cursorHide = true;
	}

	LeaveCriticalSection(&m_critsect);
}

void CMouseTestDlg::OnUpdateRate()
{
	std::vector<double> delays;
	CString text;
	double rate, time;

	if (!TryEnterCriticalSection(&m_critsect))
	{
		return;
	}

	rate = 0;
	time = 0;

	for (size_t i = m_points.size(); i > 0; i--)
	{
		double d = m_points[i - 1].delay;
		delays.push_back(d);
		time += d;

		// only few seconds
		if (time >= 3)
		{
			break;
		}
	}

	LeaveCriticalSection(&m_critsect);

	if (!delays.empty())
	{
		std::sort(delays.begin(), delays.end());
		rate = 1 / delays[delays.size() >> 1];
	}

	text.Format(TEXT("%.0lf"), rate);
	m_rateEdt.SetWindowText(text);
}

void CMouseTestDlg::OnUpdateSpeed()
{
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
	m_input.close();

	KillTimer(1);
	DeleteCriticalSection(&m_critsect);

	for (MouseBrushes::iterator it = m_brushes.begin();
		it != m_brushes.end(); it++)
	{
		DeleteObject(it->second);
	}

	CDialog::OnDestroy();
}

void CMouseTestDlg::OnDrawItem(int id, LPDRAWITEMSTRUCT data)
{
	if (id == m_canvasBox.GetDlgCtrlID())
	{
		RECT rect;
		POINT point;
		COLORREF color;

		EnterCriticalSection(&m_critsect);

		if (m_brushes.find(0) == m_brushes.end())
		{
			m_brushes[0] = CreateSolidBrush(0);
		}

		FillRect(data->hDC, &data->rcItem, m_brushes[0]);

		for (size_t i = 0; i < m_points.size(); i++)
		{
			point = m_points[i].point;
			color = m_points[i].color;

			if (!PtInRect(&data->rcItem, point))
			{
				continue;
			}

			if (m_brushes.find(color) == m_brushes.end())
			{
				m_brushes[color] = CreateSolidBrush(color);
			}

			rect.left = __max(data->rcItem.left, point.x - 1);
			rect.top = __max(data->rcItem.top, point.y - 1);
			rect.right = __min(data->rcItem.right, point.x + 1);
			rect.bottom = __min(data->rcItem.bottom, point.y + 1);

			FillRect(data->hDC, &rect, m_brushes[color]);
		}

		LeaveCriticalSection(&m_critsect);
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
	EnterCriticalSection(&m_critsect);

	if (m_cursorShow)
	{
		if (m_cursorChk.GetCheck() == 0)
		{
			ShowCursor(FALSE);
			m_cursorShow = false;
		}
	}

	LeaveCriticalSection(&m_critsect);
}

void CMouseTestDlg::OnCursorShow()
{
	EnterCriticalSection(&m_critsect);

	if (!m_cursorShow)
	{
		ShowCursor(TRUE);
		m_cursorShow = true;
	}

	LeaveCriticalSection(&m_critsect);
}

void CMouseTestDlg::OnBnClickedClearBtn()
{
	EnterCriticalSection(&m_critsect);

	m_points.clear();
	m_canvasBox.InvalidateRect(NULL, TRUE);

	LeaveCriticalSection(&m_critsect);
}

void CMouseTestDlg::OnCbnSelchangeSpeedLst()
{
	CString text;
	UINT speed;

	m_speedLst.GetWindowText(text);
	speed = _ttoi(text);
	SystemParametersInfo(SPI_SETMOUSESPEED, 0, (void*)speed, SPIF_SENDCHANGE);
	OnUpdateSpeed();
}

void CMouseTestDlg::OnCbnSelchangeAccelLst()
{
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
