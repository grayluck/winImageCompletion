
// imgCompletionDlg.h : header file
//

#pragma once


// CimgCompletionDlg dialog
class CimgCompletionDlg : public CDialogEx
{
// Construction
public:
	CimgCompletionDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_IMGCOMPLETION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	void onInit();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedbopen();
	afx_msg void OnBnClickedbrun();
	afx_msg void OnBnClickedbopent0();
	afx_msg void OnBnClickedbrunt0();
	afx_msg void OnBnClickedbopengc();
	afx_msg void OnBnClickedbrungc();
	afx_msg void OnBnClickedbopenpoi();
	afx_msg void OnCbnEditchangeComboMethod();
	afx_msg void OnCbnSelchangeComboMethod();
	afx_msg void OnBnHotItemChangeMfccolorbutton1clrz(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedbopenclrz();
	afx_msg void OnBnClickedMfccolorbutton1clrz();
	afx_msg void OnNMReleasedcaptureSliderClrz(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton1();
};
