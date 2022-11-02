
#include <iostream>
#include <fstream>
#include <windows.h>
#include <stdlib.h>
#include <io.h>
#include <stdio.h>
#include <time.h>

#include "filetool.h"
#include "stringtool.h"
#include <Windows.h>
#include <process.h>
#include <tlhelp32.h>
#pragma comment(lib, "NetAPI32.Lib")

using namespace std;

bool  isProgramRunning(string program_name)
{
	bool ret = false;
	HANDLE info_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //����ϵͳ�����н��̵Ŀ���
	if (info_handle == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot fail!!\n\n");
		return false;
	}

	PROCESSENTRY32W program_info;
	program_info.dwSize = sizeof(PROCESSENTRY32W);  //���ýṹ���С
	int bResult = Process32FirstW(info_handle, &program_info); //��ȡ���н����е�һ�����̵���Ϣ
	if (!bResult)
	{
		printf("Process32FirstW fail!!\n\n");
		return false;
	}

	while (bResult)
	{
		string pro_name = WString2String(program_info.szExeFile);
		if (pro_name.compare(program_name) == 0)
		{
			ret = true;
			break;
		}
		//�����һ�����̵Ľ�����Ϣ
		bResult = Process32Next(info_handle, &program_info);
	}
	CloseHandle(info_handle);//�رվ��
	return ret;
}
int CopyFilec(char *SourceFile, char *NewFile)
{
	ifstream in;
	ofstream out;
	in.open(SourceFile, ios::binary);//��Դ�ļ�
	if (in.fail())//��Դ�ļ�ʧ��
	{
		cout << "Error 1: Fail to open the source file." << endl;
		in.close();
		out.close();
		return 0;
	}
	out.open(NewFile, ios::binary);//����Ŀ���ļ�
	if (out.fail())//�����ļ�ʧ��
	{
		cout << "Error 2: Fail to create the new file." << endl;
		out.close();
		in.close();
		return 0;
	}
	else//�����ļ�
	{
		out << in.rdbuf();
		out.close();
		in.close();
		return 1;
	}
}


bool DeleteFileFuncW(wchar_t  *source)
{

	/* _access(char *,int) �ж��ļ��Ƿ����
	���� ����0;������ ����-1.
	_access(const char *path,int mode)
	mode��ֵ:
	00 �Ƿ����
	02 дȨ��
	04 ��Ȩ��
	06 ��дȨ��
	*/
	if (!_waccess(source, 0))//����ļ�����:�ļ�Ϊֻ���޷�ɾ��
	{
		//ȥ���ļ�ֻ������
		SetFileAttributes(source, 0);
		if (DeleteFile(source))//ɾ���ɹ�
		{
			return true;
		}
		else//�޷�ɾ��:�ļ�ֻ������Ȩ��ִ��ɾ��
		{
			return false;
		}
	}
	else//�ļ�������
	{
		return true;
	}
}

bool DeleteFileFunc(string source)
{

	if (!_access(source.c_str(), 0))//����ļ�����:�ļ�Ϊֻ���޷�ɾ��
	{
		//ȥ���ļ�ֻ������
		wstring wsource = String2WString(source);
		SetFileAttributes(wsource.c_str(), 0);
		if (DeleteFile(wsource.c_str()))//ɾ���ɹ�
		{
			return true;
		}
		else//�޷�ɾ��:�ļ�ֻ������Ȩ��ִ��ɾ��
		{
			return false;
		}
	}
	else//�ļ�������
	{
		return true;
	}
}

bool ReNameFun(char *source, char *newname)
{
	if (!_access(source, 0))//����ļ�����:
	{
		if (!rename(source, newname))//ɾ���ɹ�
		{
			return true;
		}
		else//�޷�������:�ļ��򿪻���Ȩ��ִ��������
		{
			return false;
		}
	}
	else//�ļ�������
	{
		return false;
	}
}

bool ReNameFun(string source, string newname)
{
	if (!_access(source.c_str(), 0))//����ļ�����:
	{
		if (!rename(source.c_str(), newname.c_str()))//ɾ���ɹ�
		{
			return true;
		}
		else//�޷�������:�ļ��򿪻���Ȩ��ִ��������
		{
			return false;
		}
	}
	else//�ļ�������
	{
		return false;
	}
}

int CopyFilec(string SourceFile, string NewFile)
{
	ifstream in;
	ofstream out;
	in.open(SourceFile, ios::binary);//��Դ�ļ�
	if (in.fail())//��Դ�ļ�ʧ��
	{
		cout << "Error 1: Fail to open the source file." << endl;
		in.close();
		out.close();
		return 0;
	}
	out.open(NewFile, ios::binary);//����Ŀ���ļ�
	if (out.fail())//�����ļ�ʧ��
	{
		cout << "Error 2: Fail to create the new file." << endl;
		out.close();
		in.close();
		return 0;
	}
	else//�����ļ�
	{
		out << in.rdbuf();
		out.close();
		in.close();
		return 1;
	}
}

bool GetSelfModulePath(char* path)
{
	MEMORY_BASIC_INFORMATION mbi;
	HMODULE dllHandle = ((::VirtualQuery(GetSelfModulePath, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
	TCHAR t_path[MAX_PATH] = { 0 };
	GetModuleFileName(dllHandle, t_path, MAX_PATH);
	int iLength = WideCharToMultiByte(CP_ACP, 0, t_path, -1, NULL, 0, NULL, NULL);
	return WideCharToMultiByte(CP_ACP, 0, t_path, -1, path, iLength, NULL, NULL);
}

typedef struct _ASTAT_
{
	ADAPTER_STATUS adapt;           //����һ����������������Ϣ
	NAME_BUFFER    NameBuff[30];
}ASTAT, * PASTAT;                   //����һ�������Ľṹ��

//���Ͳ�ִ��ָ����������ƿ飨NCB��
string GetMac()
{
	NCB ncb;                //������ƿ�
	ASTAT Adapter;          //����MAC��ַ�����ݽṹ
	LANA_ENUM lana_enum;    //������ǰLAN������������
	UCHAR uRetCode;         //���ؽ����
	//UCHAR *mac={};

	//��Netbios()��ȡ������Ϣ��MAC\IP\���ء����Ļ������裺
	//һ��ö�ٱ�������LAN
	//��������ÿ��LANA����ʼ������LANA��
	//������ȡ������Ϣ��������ݽṹ��
	string ret = "";

	//һ��NCBENUM��ö��LAN��������LANA������
	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));

	ncb.ncb_command = NCBENUM; //ͳ��ϵͳ������������ ָ��ָ��ΪNCBENUM,���ڻ�ȡ���������������  ����������NCBENUM����Ի�ȡ��ǰ������������Ϣ�����ж��ٸ�������ÿ�������ı�ţ�MAC��ַ��
	ncb.ncb_buffer = (unsigned char*)&lana_enum;
	ncb.ncb_length = sizeof(LANA_ENUM);
	uRetCode = Netbios(&ncb);
	if (uRetCode != NRC_GOODRET)
		ret = "neo3" + to_string(rand());

	////����NCBREST������LAN������
	for (int lana = 0; lana < lana_enum.length; lana++)
	{
		ncb.ncb_command = NCBRESET;                 //��ʼ���߼���������
		ncb.ncb_lana_num = lana_enum.lana[lana];
		uRetCode = Netbios(&ncb);
		if (uRetCode == NRC_GOODRET)
			ret = "neo3" + to_string(rand());
	}

	//����NCBASTAT���������ػ�Զ����������״̬��lan_enum.lana[0]Ĭ��Ϊ����PC��MAC��
	memset(&ncb, 0, sizeof(ncb));
	ncb.ncb_command = NCBASTAT;                     //���߼�����ͳ����Ϣ����

	ncb.ncb_lana_num = lana_enum.lana[0];    //����ֻ��ȡ��һ����������Ϣ�����Ҫ��ȡȫ����������Ϣ�����԰���һ���ַ��������ѭ�������
	strcpy((char*)ncb.ncb_callname, "*");
	ncb.ncb_buffer = (unsigned char*)&Adapter;    //�����ݽṹncb �� Adapter�����Adapter
	ncb.ncb_length = sizeof(Adapter);
	uRetCode = Netbios(&ncb);

	char mac[256] = { 0 };
	if (uRetCode != NRC_GOODRET)
	{
		ret = "neo3" + to_string(rand());

	}
	else
	{
		sprintf_s(mac, "%02X-%02X-%02X-%02X-%02X-%02X",
			Adapter.adapt.adapter_address[0],
			Adapter.adapt.adapter_address[1],
			Adapter.adapt.adapter_address[2],
			Adapter.adapt.adapter_address[3],
			Adapter.adapt.adapter_address[4],
			Adapter.adapt.adapter_address[5]);
		ret = mac;
	}
	return ret;
}
