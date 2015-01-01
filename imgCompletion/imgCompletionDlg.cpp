
// imgCompletionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "imgCompletion.h"
#include "imgCompletionDlg.h"
#include "afxdialogex.h"

#include "work.h"
#include "graphcut.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CimgCompletionDlg dialog

CimgCompletionDlg::CimgCompletionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CimgCompletionDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CimgCompletionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CimgCompletionDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_bOpen, &CimgCompletionDlg::OnBnClickedbopen)
	ON_BN_CLICKED(IDC_bRun, &CimgCompletionDlg::OnBnClickedbrun)
	ON_BN_CLICKED(IDC_bOpen_t0, &CimgCompletionDlg::OnBnClickedbopent0)
	ON_BN_CLICKED(IDC_bRun_t0, &CimgCompletionDlg::OnBnClickedbrunt0)
	ON_BN_CLICKED(IDC_bOpen_gc, &CimgCompletionDlg::OnBnClickedbopengc)
	ON_BN_CLICKED(IDC_bRun_gc, &CimgCompletionDlg::OnBnClickedbrungc)
END_MESSAGE_MAP()


// CimgCompletionDlg message handlers

BOOL CimgCompletionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	onInit();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CimgCompletionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CimgCompletionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CimgCompletionDlg::onInit()
{
	// DEBUG
	graphcut::graphcut_init("imgs/D20.bmp");
	graphcut::runGraphcut();
}


void CimgCompletionDlg::OnBnClickedbopen()
{
	CFileDialog dlgFile(1);
	if(dlgFile.DoModal() == IDOK)
	{
		CString pathname = dlgFile.GetPathName();
		chrim_init(pathname);
	}
}


void CimgCompletionDlg::OnBnClickedbrun()
{
	brute_force();
}


void CimgCompletionDlg::OnBnClickedbopent0()
{
	CFileDialog dlgFile(1);
	if(dlgFile.DoModal() == IDOK)
	{
		CString pathname = dlgFile.GetPathName();
		texture_init(pathname);
	}
}


void CimgCompletionDlg::OnBnClickedbrunt0()
{
	brute_force();
}


void CimgCompletionDlg::OnBnClickedbopengc()
{
	CFileDialog dlgFile(1);
	if(dlgFile.DoModal() == IDOK)
	{
		CString pathname = dlgFile.GetPathName();
		graphcut::graphcut_init(pathname);
	}
}

void CimgCompletionDlg::OnBnClickedbrungc()
{
	graphcut::runGraphcut();
}
