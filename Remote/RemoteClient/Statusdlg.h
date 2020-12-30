#pragma once


// CStatusdlg 对话框

class CStatusdlg : public CDialog
{
	DECLARE_DYNAMIC(CStatusdlg)

public:
	CStatusdlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CStatusdlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_STATUS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_info;
};
