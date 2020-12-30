#pragma once
#include "pch.h"
#include "framework.h"

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
		//4=�����ȣ�2=����ȣ�2=У���
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
			return;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
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
	}

	//����������
	~CPacket() {}
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
	std::string strData;//����Ҫ������������
	WORD sSum;//��У��
	std::string strOut; //������������(��ͷ+������+��������+����+��У��)

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


//�ļ��ṹ�ýṹ��
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


//SOCKET��������
class CServerSocket
{
public:
	//��ȡʵ����һʵ��;
	static CServerSocket* getInstance()
	{
		if (m_instance == NULL)//��̬����û��thisָ��  ����ֱ�ӷ��ʳ�Ա����
		{
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	//��ʼ���׽���SOCK
	bool InitSocket()
	{
		if (m_sock == -1)return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		//��
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) return false;
		//TODO:
		if (listen(m_sock, 1) == -1) return false;
		return true;
	}

	//������������
	bool AcceptClient()
	{
		TRACE("enter AcceptClient\r\n");
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		TRACE("m_client = %d\r\n", m_client);
		if (m_client == -1)return false;
		return true;
		//recv(serv_sock, buffer, sizeof(buffer),0);
		//send(serv_sock, buffer, sizeof(buffer), 0);
	}

#define BUFFER_SIZE 4096
	//��������ĺ���
	int DealCommand()
	{
		if (m_client == -1) return -1;
		//char buffer[1024]="";
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL)
		{
			TRACE("�ڴ治��");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
			{
				delete[]buffer;
				return -1;
			}
			TRACE("recv %d\r\n", len);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[]buffer;
				return m_packet.sCmd;
			}
		}
		delete[]buffer;
		return -1;
	}

	//�����ļ�
	bool Send(const char* pData, size_t nSize)
	{
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}

	//�����ļ�(����ʽ)
	bool Send(CPacket& pack)
	{
		if (m_client == -1) return false;
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;
	}

	//��ȡ�ļ���
	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4) || 
			(m_packet.sCmd==9))
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
	CPacket GetPacket()
	{
		return m_packet;
	}

	//�����ӵĹر��׽���
	void CloseClient()
	{
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}

private:
	SOCKET m_sock, m_client;
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& ss) {}
	CServerSocket(const CServerSocket& ss)
	{
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerSocket()
	{
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���,������������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServerSocket() {
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
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	static CServerSocket* m_instance;
	class CHelper
	{
	public:
		CHelper()
		{
			CServerSocket::getInstance();
		}
		~CHelper()
		{
			CServerSocket::releaseInstrance();
		}
	};
	static CHelper m_helper;
};
