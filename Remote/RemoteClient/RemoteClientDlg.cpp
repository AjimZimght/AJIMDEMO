
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include "ClientSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "CWatchDialog.h"


// 用于应用程序  '关于'  菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
//-------------------------------插入了修改的信息 用于测试是否能进行git的同步测试。
// 实现 
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_PORT, m_nPort);
	DDX_Control(pDX, IDC_TREE_DIR, m_tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

//发包函数里已经 处理了返回的命令，把包给解析完了 并且存放在了pClient。bAutoClose参数是是否关闭套接字
int CRemoteClientDlg::SendCommandPacket(int nCmd, bool bAutoClose,BYTE* pData, size_t nLength)
{
	UpdateData();
	CClientSocket* pClient = CClientSocket::getInstance();
	bool ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_nPort));
	if (!ret)
	{
		AfxMessageBox("网络初始化失败");
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	ret = pClient->Send(pack);
	TRACE("Send ret %d\r\n", ret);
	int cmd = pClient->DealCommand();
	TRACE("ACK:%d\r\n", cmd);
	if(bAutoClose)
		pClient->CloseSocket();
	return cmd;
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	ON_MESSAGE(WM_SEND_PACKET,&CRemoteClientDlg::OnSendPacket)
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_server_address = 0x7f000001;
	//m_nPort = _T("9527");
	//m_server_address =0xC0A85B80 ;
	m_nPort = _T("9527");
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DLG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);

	m_isFull = false;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//连接测试
void CRemoteClientDlg::OnBnClickedBtnTest()
{
	SendCommandPacket(1981);
}

//获取当前文件
CString CRemoteClientDlg::GetPath(HTREEITEM hTree)
{
	//strRet是最后返回的地址路径，而strTmp中间过程获得的路径。
	CString strRet, strTmp;
	do 
	{
		strTmp = m_tree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		hTree = m_tree.GetParentItem(hTree);
	} while(hTree!=NULL);
	return strRet;
}

//清除子树中查找到的文件以免   双击后全部删除
void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do 
	{
		hSub = m_tree.GetChildItem(hTree);
		if(hSub!=NULL)m_tree.DeleteItem(hSub);
	} while (hSub!=NULL);
}

//监控屏幕的线程
void CRemoteClientDlg::threadEntryForWatchData(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadWatchData();
	_endthread();
}

//监控屏幕的函数实现
void CRemoteClientDlg::threadWatchData()
{//可能存在异步问题导致程序崩溃
	Sleep(50);
	CClientSocket* pClient = NULL;
	do {
		pClient = CClientSocket::getInstance();
	} while (pClient == NULL);
	while(m_isClosed == false)
	{
		//CPacket pack(6, NULL, 0);
		if (m_isFull == false)//更新数据到缓存
		{
			int ret = SendMessage(WM_SEND_PACKET, 6 << 1 | 1);
			//TRACE("##########WM_SEND_PACKET\n");
			if (ret == 6)
			{

				BYTE* pData = (BYTE*)pClient->GetPacket().strData.c_str();
				HGLOBAL hMen = GlobalAlloc(GMEM_MOVEABLE, 0);
				if (hMen == NULL)
				{
					TRACE("内存不足了!");
					Sleep(1);
					continue;
				}
				IStream* pStream = NULL;
				HRESULT hRet = CreateStreamOnHGlobal(hMen, TRUE, &pStream);
				if (hRet == S_OK)
				{
					ULONG length = 0;
					pStream->Write(pData, pClient->GetPacket().strData.size(), &length);
					LARGE_INTEGER bg = { 0 };
					pStream->Seek(bg, STREAM_SEEK_SET, NULL);
					if((HBITMAP)m_image!=NULL)m_image.Destroy();
					m_image.Load(pStream);
					m_isFull = true;
				}
			}
			else
			{
				Sleep(1);
			}
		}
		else Sleep(1);
		
	}
}


//下载文件的线程
void CRemoteClientDlg::threadEntryForDownFile(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadDownFile();
	_endthread();
}
//下载文件函数
void CRemoteClientDlg::threadDownFile()
{
	//选中目标
	int nListSelected = m_List.GetSelectionMark();
	//获得名称
	CString strFile = m_List.GetItemText(nListSelected, 0);
	//弹出下载对话框。
	//参数1、TRUE为打开对话框  FALSE为另存为对话框
	//参数2、默认的文件扩展名 如果为NULL则不追加扩展
	//参数3、初始的文件名
	//参数4、FALG标志位:OFN_OVERWRITEPROMPT  有重复的就覆盖   OFN_HIDEREADONLY 隐藏只读
	//参数5、是过滤器
	//参数6、父窗口
	CFileDialog dlg(FALSE, NULL, strFile,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() == IDOK)
	{
		FILE* pFile = fopen(dlg.GetPathName(), "wb+");
		if (pFile == NULL)
		{
			AfxMessageBox(_T("本地没有权限读取文件或者无法打开"));
			m_dlgStatus.ShowWindow(SW_HIDE);
			EndWaitCursor();
			return;
		}
		CClientSocket* pClient = CClientSocket::getInstance();
		HTREEITEM hSelected = m_tree.GetSelectedItem();
		strFile = GetPath(hSelected) + strFile;
		TRACE("%s\r\n", LPCSTR(strFile));
		do
		{
			//int ret = SendCommandPacket(4, false, (BYTE*)(LPCTSTR)strFile, strFile.GetLength());
			int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCTSTR)strFile);
			if (ret < 0)
			{
				AfxMessageBox("执行下载命令失败");
				//TRACE("执行下载失败:ret = %d\r\n", ret);
				//fclose(pFile);
				pClient->CloseSocket();
				return;
			}

			long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
			if (nLength == 0)
			{
				AfxMessageBox("文件长度为0 或者无法读取文件!");
				//fclose(pFile);
				//pClient->CloseSocket();
				return;
			}
			long long nCount = 0;
			while (nCount < nLength)
			{
				ret = pClient->DealCommand();
				if (ret < 0)
				{
					AfxMessageBox("传输失败!");
					TRACE("传输失败 ret = %d\r\n", ret);
					break;
				}
				fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
				nCount += pClient->GetPacket().strData.size();
			}
		} while (false);
		fclose(pFile);
		pClient->CloseSocket();
	}
	m_dlgStatus.ShowWindow(SW_HIDE);
	EndWaitCursor();
	MessageBox(_T("下载完成!!"), _T("完成"));
}

//加载当前文件函数  用于删除之后。刷新显示
void CRemoteClientDlg::LoadFileCurrent()
{
	HTREEITEM hTree = m_tree.GetSelectedItem();
	CString strPath = GetPath(hTree);
	m_List.DeleteAllItems();
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	//PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	CClientSocket* pClient = CClientSocket::getInstance();
	while (pInfo->HasNext)
	{
		TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (!pInfo->IsDirectory)
		{
			m_List.InsertItem(0, pInfo->szFileName);
		}
		int cmd = pClient->DealCommand();
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	pClient->CloseSocket();
}

//创建列出文件的
void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	//得到当前鼠标的位置
	GetCursorPos(&ptMouse);
	//讲鼠标位置对应到这个tree控件
	m_tree.ScreenToClient(&ptMouse);
	//通过HitTest函数来确定点了哪一个
	HTREEITEM hTreeSelected = m_tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL)
		return;
	//看它的子节点是不是为空，如果是，则返回
	if (m_tree.GetChildItem(hTreeSelected) == NULL)
		return;
	DeleteTreeChildrenItem(hTreeSelected);
	m_List.DeleteAllItems();
	//先得到当前的路径文件名字
	CString strPath = GetPath(hTreeSelected);
	//讲这个文件名发过去,得到这个文件的相关信息
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();

	//PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	CClientSocket* pClient = CClientSocket::getInstance();
	int count = 0;//文件数量
	while (pInfo->HasNext)
	{
		TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (pInfo->IsDirectory)
		{
			if ((CString(pInfo->szFileName) == ".") || (CString(pInfo->szFileName) == ".."))
			{
				//当目录是.或者..时  拿到下一个命令  然后取出文件内容  进行下一次循环
				int cmd = pClient->DealCommand();
				TRACE("ack:%d\r\n", cmd);
				if (cmd < 0)break;
				pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			m_tree.InsertItem("", hTemp, TVI_LAST);
		}
		else
		{
			m_List.InsertItem(0, pInfo->szFileName);

		}
		count++;
		int cmd = pClient->DealCommand();
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	pClient->CloseSocket();
	TRACE("Count=%d\r\n", count);
}


//显示驱动光盘的按钮
void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	int ret = SendCommandPacket(1);
	if (ret == -1)
	{
		AfxMessageBox(_T("命令处理失败"));
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	std::string drivers = pClient->GetPacket().strData;
	std::string dr;
	m_tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',')
		{
			dr += ":";
			HTREEITEM hTemp = m_tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			m_tree.InsertItem(NULL, hTemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
}


//双击树列表
void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码

	*pResult = 0;
	LoadFileInfo();
	
}

//单击树列表
void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}

//显示列表的节点。右键菜单。
void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse,ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);
	int ListSelectd = m_List.HitTest(ptList);
	if (ListSelectd < 0)return;
	//加载菜单栏
	CMenu menu;
	//加载RCLICK的菜单
	menu.LoadMenu(IDR_MENU_RCLICK);
	//因为有多行 所以加载第一行 0
	CMenu* pPupup = menu.GetSubMenu(0);
	if (pPupup != NULL)
	{
		//一个是左对齐，一个是响应右键
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
	}

}

//下载文件
void CRemoteClientDlg::OnDownloadFile()
{
	////////////添加线程函数
	_beginthread(CRemoteClientDlg::threadEntryForDownFile, 0, this);
	Sleep(50);
	//让鼠标处于等待状态。
	BeginWaitCursor();
	m_dlgStatus.m_info.SetWindowText(_T("命令正在执行中"));
	m_dlgStatus.ShowWindow(SW_SHOW);
	m_dlgStatus.CenterWindow(this);
	m_dlgStatus.SetActiveWindow();
	///大文件处理
}

//删除文件
void CRemoteClientDlg::OnDeleteFile()
{
	//获取树上选取的结点
	HTREEITEM hSelected = m_tree.GetSelectedItem();
	//获取树上的路径
	CString strPath = GetPath(hSelected);
	//获取列表文件
	int nSelected = m_List.GetSelectionMark();
	//获取列表
	CString strFile = m_List.GetItemText(nSelected, 0);
	strFile = strPath + strFile;
	int ret = SendCommandPacket(9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0)
	{
		AfxMessageBox("删除文件执行命令失败");
	}
	LoadFileCurrent();
}

//打开文件
void CRemoteClientDlg::OnRunFile()
{
	HTREEITEM hSelected = m_tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nSelected, 0);
	strFile = strPath + strFile;
	int ret = SendCommandPacket(3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0)
	{
		AfxMessageBox("打开文件失败");
	}
}

//自定义的消息函数
LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)
{
	int ret = 0;
	int cmd = wParam >> 1;
	switch (cmd)
	{
	case 4:
		{
			CString strFile = (LPCSTR)lParam;
			ret = SendCommandPacket(cmd, wParam & 1, (BYTE*)(LPCTSTR)strFile, strFile.GetLength());
		}
		break;
	case 5:
		{//鼠标操作
		ret = SendCommandPacket(cmd, wParam & 1, (BYTE*)lParam, sizeof(MOUSEEV));

		}
		break;
	case 6:
		{//画面传输
			TRACE("####\n");
			ret = SendCommandPacket(cmd, wParam & 1);
		}
		break;
	case 7:
		{
			//锁机操作
			ret = SendCommandPacket(cmd, wParam & 1);
		}
		break;
	case 8:
		{
			//解锁操作
			ret = SendCommandPacket(cmd, wParam & 1);
		}
		break;
	default:
		ret = -1;
	}
	
	return ret;
}



//点击按钮开始监控
void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	m_isClosed = false;
	CWatchDialog dlg(this);
	HANDLE hThread = (HANDLE)_beginthread(CRemoteClientDlg::threadEntryForWatchData, 0, this);
	//GetDlgItem(IDC_BTN_START_WATCH)->EnableWindow(FALSE);
	dlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(hThread,500);
}

//定时器
void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}
