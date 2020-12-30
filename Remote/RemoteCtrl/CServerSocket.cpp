#include "pch.h"
#include "CServerSocket.h"

//CServerSocket server
CServerSocket* CServerSocket::m_instance = NULL;
CServerSocket::CHelper CServerSocket::m_helper;

CServerSocket* pserver = CServerSocket::getInstance();