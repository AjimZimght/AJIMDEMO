#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	//���캯��
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	//������װ
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
	{
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
		{
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}

	//������
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}

	//����
	CPacket(const BYTE* pData, size_t& nSize)
	{
		size_t i = 0;
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF)
			{
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize)//�����ݿ��ܲ�ȫ�����߰�ͷδ��ȫ�����յ�
		{
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (nLength + i > nSize)
		{//��δ��ȫ���յ������أ�����ʧ��
			nSize = 0;
			TRACE("12#############BEGIN CPacket nSize = %d,nLength = %d\n", nSize, nLength);
			return;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			TRACE("%s\r\n", strData.c_str()+12);
			TRACE("######%s\r\n", strData.c_str());
			i += nLength - 4;
		}

		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)
		{
			nSize = i;
			return;
		}
		nSize = 0;
		TRACE("############END CPacket\n");
	}

	//����������
	//----------~CPacket() {}
	//����=��
	CPacket& operator=(const CPacket& pack)
	{
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size()//�����ݵĴ�С
	{
		return nLength + 6;
	}

	//����������ȡ
	const char* Data()
	{
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

public:
	WORD sHead;//�̶�λFEFF
	DWORD nLength;//�����ȣ��ӿ������У��ͽ���
	WORD sCmd;//��������
	std::string strData;//������
	WORD sSum;//��У��
	std::string strOut; //������������

};
#pragma pack(pop)

typedef struct MouseEvent {
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//���Ķ���:������ƶ���˫��
	WORD nButton;//��� �Ҽ� �м�
	POINT ptXY;//����

}MOUSEEV, * PMOUSEEV;

//�ļ��ṹ��
typedef struct file_info {
	file_info()
	{
		IsInvalid = 0;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//�Ƿ�����Ч������Ч
	BOOL IsDirectory;//�Ƿ�ΪĿ¼ 0 �� 1 ��
	BOOL HasNext;//�Ƿ��к��� 0 û�� 1��
	char szFileName[256];//�ļ���  
}FILEINFO, * PFILEINFO;


//�жϴ�����
std::string GetErrorInfo(int wsaErrCode);


class CClientSocket
{
public:
	//��ȡʵ����һʵ��;
	static CClientSocket* getInstance()
	{
		if (m_instance == NULL)//��̬����û��thisָ��  ����ֱ�ӷ��ʳ�Ա����
		{
			m_instance = new CClientSocket();
		}
		return m_instance;

	}

	//��ʼ���׽���SOCK
	bool InitSocket(int nIP ,int nPort)
	{
		if (m_sock != INVALID_SOCKET)CloseSocket();
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sock == -1)return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = htonl(nIP);
		serv_adr.sin_port = htons(nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			AfxMessageBox("ָ����IP��ַ������!");
			return false;
		}
		int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
		if (ret == -1)
		{
			AfxMessageBox("����ʧ��!");
			TRACE("����ʧ��,%d\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()));
			return false;
		}
		return true;
	}



#define BUFFER_SIZE 2048000
	//��������ĺ���
	int DealCommand()
	{
		if (m_sock == -1) return -1;
		//char buffer[1024]="";
		char* buffer = m_buffer.data();
		//memset(buffer, 0, BUFFER_SIZE);  ������0��BUG
		static size_t index = 0;
		while (true)
		{
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
			
			if (len <= 0 && (index <=0))
			{
				return -1;
			}
			//Dump(buffer, len);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			
			if (len > 0)
			{
				memmove(buffer, buffer + len,index - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	//�����ļ�
	bool Send(const char* pData, size_t nSize)
	{
		if (m_sock == -1) return false;
		return send(m_sock, pData, nSize, 0) > 0;
	}

	//�����ļ�(����ʽ)
	bool Send(CPacket& pack)
	{
		TRACE("m_sock %d\r\n", m_sock);
		if (m_sock == -1) return false;
		return send(m_sock, pack.Data(), pack.Size(), 0) > 0;
	}

	//��ȡ�ļ���
	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4))
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	//����¼�
	bool GetMouseEvent(MOUSEEV& mouse)
	{
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	//�õ�����,����һ����
	CPacket& GetPacket()
	{
		return m_packet;
	}

	void CloseSocket()
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}

private:
	std::vector<char> m_buffer;
	SOCKET m_sock;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& ss) {}
	CClientSocket(const CClientSocket& ss)
	{
		m_sock = ss.m_sock;

	}
	CClientSocket()
	{
		m_sock = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���,������������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);
		
	}
	~CClientSocket() {
		closesocket(m_sock);
		WSACleanup();
	}
	BOOL InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
		{
			return FALSE;
		}
		return TRUE;
	}
	static void releaseInstrance()
	{
		if (m_instance != NULL)
		{
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	static CClientSocket* m_instance;
	class CHelper
	{
	public:
		CHelper()
		{
			CClientSocket::getInstance();
		}
		~CHelper()
		{
			CClientSocket::releaseInstrance();
		}
	};
	static CHelper m_helper;
};

