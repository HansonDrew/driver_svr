#include "UpdSocket.h"
#include <stdio.h>
#include <memory.h>
#include <process.h>
#include <map>
#include <stdlib.h>
#include "../../GlobalDLLContext.h"

#include "../pxrTool/TimeTool.h"
#include "../../GlobalDLLContext.h"
#include "../pxrTool/config_reader.h"
#include "../pxrTool/TimeTool.h"
int UpdSocket::InitSocket(std::string ip, u_short port) 
{
	
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	WSAStartup(socketVersion, &wsaData);
	m_BaseSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (m_BaseSocket == INVALID_SOCKET)
	{
		return -1;
	}
	
	m_Local_Addr.sin_family = AF_INET;
	m_Local_Addr.sin_port = htons(port);
	m_Local_Addr.sin_addr.s_addr = INADDR_ANY;

	m_Dst_Addr.sin_family = AF_INET;
	m_Dst_Addr.sin_port = htons(port);
	m_Dst_Addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	if (bind(m_BaseSocket, (struct sockaddr*)&m_Local_Addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("RtpQuality socket bind error");
	}
}
int UpdSocket::RecvData(char buf[], int& len) 
{
	int flag = 0;
	int socket_len = sizeof(struct sockaddr);;
	sockaddr_in addr;
	len=recvfrom(m_BaseSocket, buf, len, flag, (sockaddr*)&addr, &socket_len);
	return len;
}
int UpdSocket::SendData(char *buf, int& len) 
{
	int ret=sendto(m_BaseSocket,buf,len,0, (sockaddr*)&m_Dst_Addr, sizeof(m_Dst_Addr));
	return ret;
}
int UpdSocket::SendData(char buf[], int& len, std::string dstip, u_short dstport)
{
	m_Dst_Addr.sin_family = AF_INET;
	m_Dst_Addr.sin_addr.S_un.S_addr = inet_addr(dstip.c_str()); 
	m_Dst_Addr.sin_port = htons(dstport);
	int ret = sendto(m_BaseSocket, buf, len, 0, (sockaddr*)&m_Dst_Addr, sizeof(m_Dst_Addr));
	return ret;
}
int UpdSocket::SendData(char buf[], int& len, std::string dstip, u_short dstport,  SOCKET clientsocket)
{
	return 0;
	m_Dst_Addr.sin_family = AF_INET;
	m_Dst_Addr.sin_addr.S_un.S_addr = inet_addr(dstip.c_str()); 
	m_Dst_Addr.sin_port = htons(dstport);
	int ret = sendto(clientsocket, buf, len, 0, (sockaddr*)&m_Dst_Addr, sizeof(m_Dst_Addr));
	return ret;
}