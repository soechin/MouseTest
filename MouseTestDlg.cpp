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
END_MESSAGE_MAP()

void __stdcall MouseCallback(int dx, int dy, int dz, int btns,
	double time, void* param)
{
	if (param != NULL)
	{
		((CMouseTestDlg*)param)->OnMouse(dx, dy, dz, btns, time);
	}
}

CMouseTestDlg::CMouseTestDlg() : CDialog(IDD, NULL)
{
}

void CMouseTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CANVAS, m_canvas);
	DDX_Control(pDX, IDC_RATE, m_rateTxt);
}

BOOL CMouseTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);

	GetCursorPos(&m_point);
	m_time = 0;
	m_move = 0;
	m_rate = 0;

	m_input.open(NULL, MouseCallback, this);
	SetTimer(1, 10, NULL);

	return TRUE;
}

void CMouseTestDlg::OnDestroy()
{
	KillTimer(1);
	m_input.close();

	CDialog::OnDestroy();
}

void CMouseTestDlg::OnMouse(int dx, int dy, int dz, int btns,
	double time)
{
	CSingleLock critsect(&m_critsect, true);
	CDC* pdc;
	CBrush brush;
	CRect rect;
	CPoint point;
	COLORREF color;

	if (btns == 0)
	{
		m_move = 0;
		return;
	}
	else if (m_move == 0)
	{
		GetCursorPos(&m_point);

		dx = 0;
		dy = 0;
		dz = 0;
	}

	// point
	//----------------------------------------
	m_point.Offset(dx, dy);
	m_canvas.GetWindowRect(&rect);

	if (!rect.PtInRect(m_point))
	{
		return;
	}

	point = m_point;
	m_canvas.ScreenToClient(&point);
	m_canvas.ScreenToClient(&rect);

	// color
	//----------------------------------------
	color = 0;
	if (btns & MK_LBUTTON) color |= 0x0000ff;
	if (btns & MK_RBUTTON) color |= 0xff0000;
	if (btns & MK_MBUTTON) color |= 0x00ff00;

	// save
	//----------------------------------------
	m_list.push_back(std::make_pair(point, color));

	// draw
	//----------------------------------------
	pdc = m_canvas.GetDC();
	brush.CreateSolidBrush(color);
	rect.left = __max(rect.left, point.x - 1);
	rect.top = __max(rect.top, point.y - 1);
	rect.right = __min(rect.right, point.x + 1);
	rect.bottom = __min(rect.bottom, point.y + 1);
	pdc->FillRect(&rect, &brush);
	m_canvas.ReleaseDC(pdc);

	// rate
	//----------------------------------------
	if (m_move == 0)
	{
		m_move = 1;
		m_time = time;
		m_rate = 0;
	}
	else if (time != m_time)
	{
		m_rate = m_move / (time - m_time);
		m_move += 1;
	}
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

void CMouseTestDlg::OnTimer(UINT_PTR id)
{
	if (id == 1)
	{
		CString text;
		text.Format(TEXT("%.1lf hz"), m_rate);
		m_rateTxt.SetWindowText(text);
	}

	CDialog::OnTimer(id);
}

void CMouseTestDlg::OnBnClickedClear()
{
	m_list.clear();
	m_rate = 0;
	m_canvas.InvalidateRect(NULL, TRUE);
}
