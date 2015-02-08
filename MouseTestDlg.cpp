#include "stdafx.h"
#include "MouseTest.h"
#include "MouseTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMouseTestDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_CLEAR, &CMouseTestDlg::OnBnClickedClear)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_SETTINGCHANGE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_ACCEL_CHK, &CMouseTestDlg::OnBnClickedAccelChk)
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
	DDX_Control(dx, IDC_CANVAS, m_canvas);
	DDX_Control(dx, IDC_RATE_EDT, m_rateEdt);
	DDX_Control(dx, IDC_SPEED_SLI, m_speedSli);
	DDX_Control(dx, IDC_SPEED_TXT, m_speedTxt);
	DDX_Control(dx, IDC_ACCEL_CHK, m_accelChk);
	DDX_Control(dx, IDC_ACCEL_EDT, m_accelEdt);
}

BOOL CMouseTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);

	GetCursorPos(&m_point);
	m_first = true;
	m_delay = 0;

	m_input.open(NULL, MouseCallback, this);
	SetTimer(1, USER_TIMER_MINIMUM, NULL);

	m_speedSli.SetRange(0, 20);
	m_speedSli.SetLineSize(1);
	m_speedSli.SetPageSize(2);
	m_speedSli.SetTicFreq(2);
	OnUpdateSpeed();

	return TRUE;
}

void CMouseTestDlg::OnDestroy()
{
	KillTimer(1);
	m_input.close();

	CDialog::OnDestroy();
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

	// check button down
	if (btns == 0)
	{
		m_first = true;
		return;
	}

	// check first move
	if (m_first)
	{
		GetCursorPos(&m_point);
		m_first = false;

		dx = 0;
		dy = 0;
		dz = 0;
	}

	// offset raw point(screen coord)
	m_point.Offset(dx, dy);
	m_canvas.GetWindowRect(&rect);

	if (!rect.PtInRect(m_point))
	{
		return;
	}

	point = m_point;
	m_canvas.ScreenToClient(&point);
	m_canvas.ScreenToClient(&rect);

	// set point color
	color = 0;
	if (btns & MK_LBUTTON) color |= 0x0000ff;
	if (btns & MK_RBUTTON) color |= 0xff0000;
	if (btns & MK_MBUTTON) color |= 0x00ff00;

	// save point
	m_list.push_back(std::make_pair(point, color));

	// response delay (running average)
	m_delay = m_delay * 0.9 + delay * 0.1;

	// draw point
	pdc = m_canvas.GetDC();
	brush.CreateSolidBrush(color);
	rect.left = __max(rect.left, point.x - 1);
	rect.top = __max(rect.top, point.y - 1);
	rect.right = __min(rect.right, point.x + 1);
	rect.bottom = __min(rect.bottom, point.y + 1);
	pdc->FillRect(&rect, &brush);
	m_canvas.ReleaseDC(pdc);
}

void CMouseTestDlg::OnUpdateRate()
{
	CSingleLock critsect(&m_critsect, true);
	CString text;
	double rate;

	rate = (m_delay != 0) ? (1 / m_delay) : 0;
	text.Format(TEXT("%.1lf"), rate);
	m_rateEdt.SetWindowText(text);
}

void CMouseTestDlg::OnUpdateSpeed()
{
	CSingleLock critsect(&m_critsect, true);
	CString text;
	UINT speed, accel[3];

	SystemParametersInfo(SPI_GETMOUSESPEED, 0, &speed, 0);
	text.Format(TEXT("%d"), speed);
	m_speedSli.SetPos(speed);
	m_speedTxt.SetWindowText(text);

	SystemParametersInfo(SPI_GETMOUSE, 0, accel, 0);
	text.Format(TEXT("L:%d, H:%d"), accel[0], accel[1]);
	m_accelEdt.SetWindowText(text);
	m_accelChk.SetCheck(accel[2] != 0);
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

void CMouseTestDlg::OnDrawItem(int id, LPDRAWITEMSTRUCT data)
{
	if (id == IDC_CANVAS)
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

void CMouseTestDlg::OnHScroll(UINT code, UINT pos, CScrollBar* scroll)
{
	CSingleLock critsect(&m_critsect, true);
	UINT speed;

	speed = m_speedSli.GetPos();
	SystemParametersInfo(SPI_SETMOUSESPEED, 0, (void*)speed, SPIF_SENDCHANGE);
	OnUpdateSpeed();

	CDialog::OnHScroll(code, pos, scroll);
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

void CMouseTestDlg::OnBnClickedClear()
{
	CSingleLock critsect(&m_critsect, true);

	m_list.clear();
	m_delay = 0;
	m_canvas.InvalidateRect(NULL, TRUE);
}

void CMouseTestDlg::OnBnClickedAccelChk()
{
	CSingleLock critsect(&m_critsect, true);
	UINT accel[3];

	if (SystemParametersInfo(SPI_GETMOUSE, 0, accel, 0))
	{
		accel[2] = m_accelChk.GetCheck() != 0;
		SystemParametersInfo(SPI_SETMOUSE, 0, accel, SPIF_SENDCHANGE);
		OnUpdateSpeed();
	}
}
