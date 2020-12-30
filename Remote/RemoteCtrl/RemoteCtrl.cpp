// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
void Dump(BYTE* pData, size_t nSize)
{
    std::string strOut;
    for (size_t i = 0; i < nSize; i++)
    {
        char buf[8] = "";
        if ((i > 0) && (i % 16 == 0))strOut += "\n";
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}

//查看磁盘分区
int MaKeDriverInfo() //1-->A盘 2-->b盘 3-->C盘 ..26-->Z
{
    std::string result;
    for (int i = 1; i <= 26; i++)
    {
        if (_chdrive(i) == 0)
        {
            if(result.size()>0)
                result += ',';
            result += 'A' + i - 1;
        }
    }
    result += ',';
    CPacket pack(1, (BYTE*)result.c_str(), result.size());//打包数据包
    Dump((BYTE*)pack.Data(), pack.Size());
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

//查找指定目录文件
#include <stdio.h>
#include <io.h>
#include <list>
#include <atlimage.h>



int MaKeDirectoryInfo()
{
    //std::list<FILEINFO> listFileInfos;
    std::string strPath;
    if(CServerSocket::getInstance()->GetFilePath(strPath) == false)
    {
        OutputDebugString(_T("当前命令，不是获取文件列表，命令解析错误!!"));
        return -1;
    }
    if (_chdir(strPath.c_str())!=0)
    {
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限,访问目录"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1)
    {
        OutputDebugString(_T("没有找到任何文件！！"));
		FILEINFO finfo;
		finfo.HasNext = FALSE;
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
        return -3;
    }
    int count = 0;
   do 
   {
       FILEINFO finfo;
       finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
       memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
       TRACE("[%s]\r\n", finfo.szFileName);
	   CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	   CServerSocket::getInstance()->Send(pack);
       count++;
   } while (!_findnext(hfind,&fdata));
   TRACE("server:count=%d\r\n", count);
   //发送信息到控制端
   FILEINFO finfo;
   finfo.HasNext = FALSE;
   CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
   CServerSocket::getInstance()->Send(pack);
   return 0;
}

//运行文件
int RunFile()
{
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(),NULL,NULL,SW_SHOWNORMAL);
	CPacket pack(3, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
    return 0;
}

//下载文件
int DownloadFile()
{
	std::string strPath;
    long long data = 0;
	CServerSocket::getInstance()->GetFilePath(strPath);
    FILE* pFile = NULL;
    int err = fopen_s(&pFile,strPath.c_str(), "rb");
    if (err != 0)
    {
        CPacket pack(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->Send(pack);
        return -1;
    }
    if (pFile != NULL)
    {
        //先将偏移移到最后，用来获取它的大小
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        CPacket head(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->Send(head);
        //再将文件内容指针置头
        fseek(pFile, 0, SEEK_SET);
        char buffer[1024] = "";
        size_t rlen = 0;
        do {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);
        fclose(pFile);
    }
	CPacket pack(4, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
    return 0;
}


//鼠标的各种事件
int MouseEvent()
{
    MOUSEEV mouse;
    if (CServerSocket::getInstance()->GetMouseEvent(mouse))
    {
        DWORD nFlags = 0;
        switch (mouse.nButton)
        {
        case 0://左键
            nFlags = 1;
            break;
        case 1://右键
            nFlags = 2;
            break;
        case 2://中键
            nFlags = 4;
            break;
        case 3://单纯移动鼠标
            nFlags = 8;
            break;
        }

        if(nFlags==8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);//
        switch (mouse.nAction)
        {
        case 0://单击
            nFlags |= 0x10;
            break;
        case 1://双击
            nFlags |= 0x20;
            break;
        case 2://按下
            nFlags |= 0x40;
            break;
        case 3://放开
            nFlags |= 0x80;
            break;
        default:
            break;
        }
        TRACE("mouse event:%08X x=%d y=%d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
        switch (nFlags)
        {
		case 0x21://左键双击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11://左键单击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
		case 0x41://左键按下
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://左键放开
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;

		case 0x22://右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://右键按下
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://右键放开
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;

        case 0x24://中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://中键按下
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://中建松开
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
        case 0x08://单纯鼠标移动
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        }
        CPacket pack(4, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
    }
    else
    {
        OutputDebugString(_T("获取鼠标操作参数失败!!"));
        return -1;
    }
    return 0;
}

//发送屏幕的信息
int SendScreen()
{
    CImage screen;//GDI
    HDC hScreen = ::GetDC(NULL);
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL); //获得多少比特的
    int nWidth = GetDeviceCaps(hScreen, HORZRES);//获取宽度;
    int nHeight = GetDeviceCaps(hScreen, VERTRES);//获取高度
    screen.Create(nWidth, nHeight, nBitPerPixel);//创建一个一样的CIMAGE
    BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//在内存中创建一块可以改变的分区
    if (hMem == NULL) return -1;
    IStream* pStream = NULL;
    HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//为创建的内存绑定一个流指针
    if (ret == S_OK)
    {
        screen.Save(pStream, Gdiplus::ImageFormatPNG);
        LARGE_INTEGER bg = { 0 };
        pStream->Seek(bg, STREAM_SEEK_SET, NULL);//在每次stream后，里面的文件指针会直到最后，因此要将它重新指向开头
        PBYTE pData = (PBYTE)GlobalLock(hMem);//锁定这块内存区域，让指针指向这块内存已进行使用
        SIZE_T nSize = GlobalSize(hMem);//获取内存块大小
        CPacket pack(6, pData, nSize);
        CServerSocket::getInstance()->Send(pack);
        GlobalUnlock(hMem);
    }
    pStream->Release();
    GlobalFree(hMem);
    screen.ReleaseDC();
    return 0;
}


#include "LockDialog.h"
CLockDialog dlg;
unsigned threadid = 0;

//创造子线程来实现锁住机子
unsigned __stdcall threadLockDlg(void* arg)
{
	dlg.Create(IDD_DIALOG_INFO, NULL);
	dlg.ShowWindow(SW_SHOW);
	//遮蔽后台窗口
	CRect rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
	rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN)*1.2;
	dlg.MoveWindow(rect);
	//窗口置顶
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	//限制鼠标不可见,限制鼠标功能
	ShowCursor(false);
	//隐藏任务栏
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
	//限制鼠标活动范围

	//dlg.GetWindowRect(rect);
	rect.left = 0;
	rect.top = 0;
	rect.right = 1;
	rect.bottom = 1;
	ClipCursor(rect);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_KEYDOWN)
		{
			TRACE("msg:%08X wparam:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
			if (msg.wParam == 0x1b) //按下回车退出 1b
			{
				break;
			}
		}
	}
    //恢复鼠标
	ShowCursor(true);
    //恢复任务栏
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
    dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}

//锁住机子
int LockMachine()
{
    if (((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)))
    {
		//_beginthread(threadLockDlg, 0, NULL);
        _beginthreadex(NULL, 0, threadLockDlg,NULL, 0, &threadid);
    }
	CPacket pack(7, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
    return 0;
}

//解开机子
int UnLockMachine()
{
    //dlg.SendMessage(WM_KEYDOWN, 0x1b, 001E0001);
    //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1b, 0x1E0001);
    PostThreadMessage(threadid,WM_KEYDOWN, 0x1b, 0);
	CPacket pack(8, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
    return 0;
}

//测试连接命令
int TestConnect()
{
    
    CPacket pack(1981, NULL, 0);
    bool ret = CServerSocket::getInstance()->Send(pack);
    TRACE("Send ret = %d\r\n", ret);
    return 0;
}


int DeleteLocalFile()
{
	std::string strPath;
	CServerSocket::getInstance()->GetFilePath(strPath);
	TCHAR sPath[MAX_PATH] = _T("");
	//宽字节to多字节 
	//wcstombs();
	//多字节to宽字节
	//mbstowcs(sPath, strPath.c_str(), strPath.size());  //中文容易乱码
    MultiByteToWideChar(CP_ACP,0,strPath.c_str(),strPath.size(),sPath,
        sizeof(sPath)/sizeof(TCHAR));
	DeleteFile(sPath);
	CPacket pack(9, NULL, 0);
	bool ret = CServerSocket::getInstance()->Send(pack);
    TRACE("Send ret = %d\r\n", ret);
    return 0;
}

int ExcuteCommand(int nCmd)
{
    int ret = 0;
    //静态变量  在首次调用初始化   如果是全局则是创建则初始化
    switch (nCmd)
    {
    case 1://查看磁盘分区
        ret =  MaKeDriverInfo();
        break;
    case 2://查看指定目录下的文件
        ret = MaKeDirectoryInfo();
        break;
    case 3://打开文件
        ret = RunFile();
        break;
    case 4://下载文件
        ret = DownloadFile();
    case 5://鼠标事件
        ret = MouseEvent();
        break;
    case 6://获取屏幕信息
        ret = SendScreen();
        break;
    case 7://锁机
        ret =  LockMachine();
        break;
    case 8://解锁机
        ret = UnLockMachine();
        break;
    case 9://删除文件
        ret = DeleteLocalFile();
        break;
    case 1981:
        ret = TestConnect();
        break;
    }
    return ret;
}

int main()
{

    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: 在此处为应用程序的行为编写代码。
            //套接字初始化
            //单一实例的全局唯一的
            CServerSocket* pserver = CServerSocket::getInstance();
            int count = 0;
            if (pserver->InitSocket() == false)
            {
                MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检查网络状态"), _T("初始化网络失败"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            while (CServerSocket::getInstance() != NULL)
            {
                if (pserver->AcceptClient() == false)
                {
                    if (count >= 3)
                    {
                        MessageBox(NULL, _T("多次无法正常接入用户，自动退出"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(NULL, _T("无法正常接入用户，自动重试"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                    count++;
                }
                TRACE("AcceptClient return true\r\n");
                int ret = pserver->DealCommand();
                TRACE("DealCommand ret %d\r\n", ret);
                if (ret > 0)
                {
                    //ret = ExcuteCommand(pserver->GetPacket().sCmd);
                    ret = ExcuteCommand(ret);
                    if (ret != 0)
                    {
                        TRACE("执行命令失败,%d ret = %d\r\n", pserver->GetPacket().sCmd, ret);
                    }
                    pserver->CloseClient();
                    TRACE("Command has done!\r\n");
                }
            }
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
