// CWatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "CWatchDialog.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"

// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialogEx)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_WATCH, pParent)
{

}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialogEx)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序

//鼠标位置转换
CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen)
{
	//CPoint cur = point;
	CRect clientRect;
	if(isScreen)ScreenToClient(&point);//全局坐标到客户区域坐标
	//本地坐标到远程坐标
	m_picture.GetWindowRect(clientRect);
	/*int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	int width = 1920, height = 1080;
	int x = point.x * width / width0;
	int y = point.y * height / height0;*/
	return CPoint(point.x * m_nObjWidth/clientRect.Width(),point.y * m_nObjHeight/clientRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_nObjWidth = -1;
	m_nObjHeight = -1;

	// TODO:  在此添加额外的初始化
	SetTimer(0, 50,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0)
	{
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull())
		{
			CRect rect;
			m_picture.GetWindowRect(rect);
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			if (m_nObjWidth == -1)
			{
				m_nObjWidth = pParent->GetImage().GetWidth();
			}if (m_nObjHeight == -1)
			{
				m_nObjHeight = pParent->GetImage().GetHeight();
			}
			pParent->GetImage().StretchBlt(
				m_picture.GetDC()->GetSafeHdc(), 0, 0,rect.Width(),rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);
			pParent->GetImage().Destroy();
			pParent->SetImageStatus();
			TRACE("######\n");
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


//左键双键
void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		// 坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 1;//双击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}

//左键按下
void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		TRACE("x=%d y=%d\r\n", point.x, point.y);
		// 坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		TRACE("x=%d y=%d\r\n", point.x, point.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//按下
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

//左键释放
void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		// 坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 3;//弹起
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

//右键按下
void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		// 坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 2;//按下 //TODO:服务端要做对应修改
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}


//右键释放
void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		// 坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 3;//弹起
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialogEx::OnRButtonUp(nFlags, point);
}

//右键两下
void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		// 坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 1;//双击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialogEx::OnRButtonDblClk(nFlags, point);
}


//鼠标移动
void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		// 坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 3;//没有按键
		event.nAction = 0;//移动
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();//设计隐患  网络通信和对话框有耦合
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

//单击监控屏幕的事件
void CWatchDialog::OnStnClickedWatch()
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint point;
		GetCursorPos(&point);
		// 坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 0;//单击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
}


void CWatchDialog::OnOK()
{
	

	//CDialogEx::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1);
}
