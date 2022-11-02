#include "saferegion_manger.h"
#include<iostream>
#include<winsock.h>
#include "filetool.h"

#include <Windows.h>
#include   "shellapi.h " 
#include "atlstr.h"//CString
#include "Psapi.h"
#include "libloaderapi.h"
#include "filetool.h"
#include "stringtool.h"
#include "driverlog.h"
 
using namespace std;

#define  MAXBUF  1200*8+4
//���巢�ͻ������ͽ��ܻ�����
char send_buf[MAXBUF];
//char recv_buf[MAXBUF];


static bool g_bPrintf = true;

//void dprintf(const char* fmt, ...)
//{
//	va_list args;
//	char buffer[2048];
//
//	va_start(args, fmt);
//	vsprintf_s(buffer, fmt, args);
//	va_end(args);
//
//	if (g_bPrintf)
//		printf("%s", buffer);
//
//	OutputDebugStringA(buffer);
//}


int saferegion_mamger::testSafeRegion()
{
	return 0;
}

FILE* sendlog2 = fopen("sendlog2.txt", "wb+");
FILE* sendlog1 = fopen("sendlog1.txt", "wb+");

void initialization();
int client_send(RVR::RVRVector2* buf, int write_buf_len)
{
	int send_len = 0;
	int recv_len = 0;
	////���巢�ͻ������ͽ��ܻ�����
	//char send_buf[100];
	//char recv_buf[100];
	//���������׽��֣����������׽���
	SOCKET s_server;
	//����˵�ַ�ͻ��˵�ַ
	SOCKADDR_IN server_addr;
	initialization();
	//���������Ϣ
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(29789);
	//�����׽���
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	while (1)
	{
		if (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
			//cout << "����������ʧ�ܣ�" << endl;
			DriverLog("===socket  connect  failed===\n");
			//WSACleanup();
			closesocket(s_server);
			s_server = socket(AF_INET, SOCK_STREAM, 0);
			DriverLog("===socket  connect  failed  &&  create socket again===\n");
		}
		else
		{
			//cout << "���������ӳɹ���" << endl;
			DriverLog("===socket  connect success===\n");
			send_len = send(s_server, send_buf, write_buf_len, 0);
			if (sendlog2 != NULL)
			{
				fwrite(send_buf, send_len, sizeof(char), sendlog2);
			}

			DriverLog("====================send  buf ====================.\n");
			break;
		}
	}
	closesocket(s_server);
	//WSACleanup();
	return 0;
}
void initialization() {
	//��ʼ���׽��ֿ�
	WORD w_req = MAKEWORD(2, 2);//�汾��
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		DriverLog("===socket  initi failed===\n");
	}
	else {
		//cout << "��ʼ���׽��ֿ�ɹ���" << endl;
		DriverLog("===socket  initi success===\n");
	}
	//���汾��
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		//cout << "�׽��ֿ�汾�Ų�����" << endl;
		DriverLog("===socket  version not match===\n");
		WSACleanup();
	}
	else {
		//cout << "�׽��ֿ�汾��ȷ��" << endl;
		DriverLog("===socket  version  match!!===\n");
	}
	//������˵�ַ��Ϣ

}

int saferegion_mamger::ChangeChaperOneSafeRegion(RVR::RVRVector2* buf, int buflen)
{
	DriverLog("====================ChangeChaperOne   begin  ====================\n");

	auto mSendBuf = send_buf;
	memmove(mSendBuf, &buflen, sizeof(int));
	if (buflen>0)
	{
		memmove(mSendBuf + 4, buf, sizeof(RVR::RVRVector2) * buflen);

	}

	for (int i = 0; i < buflen; i++)
	{
		DriverLog("==================index= %d ==buflen = %d, buf.x=%f, buf.y=%f ====================.\n", i, buflen, buf[i].x, buf[i].y);
	}
	int  write_buf_len = sizeof(RVR::RVRVector2) * buflen + sizeof(int);
	if (sendlog1!=NULL)
	{
		fwrite(send_buf, write_buf_len, sizeof(char), sendlog1);
	}
	
	DriverLog("====================write_buf_len = %d====================.\n", write_buf_len);

	char driverpath[1024] = { 0 };
	GetSelfModulePath(driverpath);
	string audiodriverpath = driverpath;
	deletesub(audiodriverpath, "driver_pico.dll", strlen("driver_pico.dll"));
	audiodriverpath = audiodriverpath + "\\safe_region.exe";
	DriverLog(audiodriverpath.c_str());
	bool ret = isProgramRunning("safe_region.exe");
	if (ret == false)
	{
		HINSTANCE handleExe = ShellExecuteA(NULL, "open", audiodriverpath.c_str(), NULL, NULL, SW_SHOW);

		if ((DWORD)handleExe <= 32)
		{
			DriverLog("============exe ShellExecuteA failed===========\n");
		}
		else
		{
			DriverLog("============exe ShellExecuteA success===========\n");
			client_send(buf, write_buf_len);
		}
	}
	else
	{
		DriverLog("============exe ShellExecuteA already running ===========\n");
		client_send(buf, write_buf_len);
	}

	//client_send(buf, write_buf_len);

	DriverLog("====================client_send    ChangeChaperOne   end  ====================.\n");
	return 1;
}
