// Statusdlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "Statusdlg.h"
#include "afxdialogex.h"


// CStatusdlg 对话框

IMPLEMENT_DYNAMIC(CStatusdlg, CDialog)

CStatusdlg::CStatusdlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_STATUS, pParent)
{

}

CStatusdlg::~CStatusdlg()
{
}

void CStatusdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INFO, m_info);
}


BEGIN_MESSAGE_MAP(CStatusdlg, CDialog)
END_MESSAGE_MAP()


// CStatusdlg 消息处理程序
