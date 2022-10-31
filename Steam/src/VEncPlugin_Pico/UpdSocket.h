#pragma once
#include<WinSock2.h>
#include <string>
class UpdSocket
{
public:
	SOCKET m_BaseSocket;
	sockaddr_in m_Dst_Addr;
	sockaddr_in m_Local_Addr;
	int InitSocket(std::string ip, u_short port);
	int RecvData(char buf[], int& len);
	int SendData(char* buf, int& len );
	int SendData(char buf[], int& len, std::string dstip, u_short dstport);
	int SendData(char buf[], int& len, std::string dstip,u_short dstport,SOCKET clientsocket);
};

