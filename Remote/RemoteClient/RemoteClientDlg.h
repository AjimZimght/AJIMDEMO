
// RemoteClientDlg.h: 头文件
//

#include "ClientSocket.h"
#include "Statusdlg.h"
#pragma once


#define WM_SEND_PACKET (WM_USER+1) //发送数据包消息


// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
public:
	//判断缓冲区是否满了
	bool isFull() const 
	{
		return m_isFull;
	}
	//得到图片
	CImage& GetImage()
	{
		return m_image;
	}
	//设置缓冲区状态
	void SetImageStatus(bool isFull = false)
	{
		m_isFull = isFull;
	}

private:
	CImage m_image;//缓存
	bool m_isFull;//是否满了 true=有缓存   false=没有
	bool m_isClosed;//监视是否关闭
private:
	//
	static void threadEntryForWatchData(void* arg);//静态函数不能使用this指针
	void threadWatchData();//成员函数可以用this指针
	//下载文件的线程
	static void threadEntryForDownFile(void* arg);
	//下载文件函数
	void threadDownFile();

	//加载当前文件的函数
	void LoadFileCurrent();
	//查看文件列表的函数
	void LoadFileInfo();
	//得到路径
	CString GetPath(HTREEITEM hTree);
	//删除子树上的内容
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	//1、查看磁盘分区  2、查看指定目录下文件  3、打开文件  4、下载文件
	//5、鼠标操作 6、发送屏幕内容  7、锁机  8、解锁  1981、测试链接
	//9、删除文件
	
	//返回值:是一个命令号,<0是错误。
	int SendCommandPacket(int nCmd,bool bAutoClose = true,BYTE* pData=NULL,size_t nLength=0);

// 实现
protected:
	HICON m_hIcon;

	CStatusdlg m_dlgStatus;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_nPort;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
