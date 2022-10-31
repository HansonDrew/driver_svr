
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
	HANDLE info_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //拍摄系统中所有进程的快照
	if (info_handle == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot fail!!\n\n");
		return false;
	}

	PROCESSENTRY32W program_info;
	program_info.dwSize = sizeof(PROCESSENTRY32W);  //设置结构体大小
	int bResult = Process32FirstW(info_handle, &program_info); //获取所有进程中第一个进程的信息
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
		//获得下一个进程的进程信息
		bResult = Process32Next(info_handle, &program_info);
	}
	CloseHandle(info_handle);//关闭句柄
	return ret;
}
int CopyFilec(char *SourceFile, char *NewFile)
{
	ifstream in;
	ofstream out;
	in.open(SourceFile, ios::binary);//打开源文件
	if (in.fail())//打开源文件失败
	{
		cout << "Error 1: Fail to open the source file." << endl;
		in.close();
		out.close();
		return 0;
	}
	out.open(NewFile, ios::binary);//创建目标文件
	if (out.fail())//创建文件失败
	{
		cout << "Error 2: Fail to create the new file." << endl;
		out.close();
		in.close();
		return 0;
	}
	else//复制文件
	{
		out << in.rdbuf();
		out.close();
		in.close();
		return 1;
	}
}


bool DeleteFileFuncW(wchar_t  *source)
{

	/* _access(char *,int) 判断文件是否存在
	存在 返回0;不存在 返回-1.
	_access(const char *path,int mode)
	mode的值:
	00 是否存在
	02 写权限
	04 读权限
	06 读写权限
	*/
	if (!_waccess(source, 0))//如果文件存在:文件为只读无法删除
	{
		//去掉文件只读属性
		SetFileAttributes(source, 0);
		if (DeleteFile(source))//删除成功
		{
			return true;
		}
		else//无法删除:文件只读或无权限执行删除
		{
			return false;
		}
	}
	else//文件不存在
	{
		return true;
	}
}

bool DeleteFileFunc(string source)
{

	if (!_access(source.c_str(), 0))//如果文件存在:文件为只读无法删除
	{
		//去掉文件只读属性
		wstring wsource = String2WString(source);
		SetFileAttributes(wsource.c_str(), 0);
		if (DeleteFile(wsource.c_str()))//删除成功
		{
			return true;
		}
		else//无法删除:文件只读或无权限执行删除
		{
			return false;
		}
	}
	else//文件不存在
	{
		return true;
	}
}

bool ReNameFun(char *source, char *newname)
{
	if (!_access(source, 0))//如果文件存在:
	{
		if (!rename(source, newname))//删除成功
		{
			return true;
		}
		else//无法重命名:文件打开或无权限执行重命名
		{
			return false;
		}
	}
	else//文件不存在
	{
		return false;
	}
}

bool ReNameFun(string source, string newname)
{
	if (!_access(source.c_str(), 0))//如果文件存在:
	{
		if (!rename(source.c_str(), newname.c_str()))//删除成功
		{
			return true;
		}
		else//无法重命名:文件打开或无权限执行重命名
		{
			return false;
		}
	}
	else//文件不存在
	{
		return false;
	}
}

int CopyFilec(string SourceFile, string NewFile)
{
	ifstream in;
	ofstream out;
	in.open(SourceFile, ios::binary);//打开源文件
	if (in.fail())//打开源文件失败
	{
		cout << "Error 1: Fail to open the source file." << endl;
		in.close();
		out.close();
		return 0;
	}
	out.open(NewFile, ios::binary);//创建目标文件
	if (out.fail())//创建文件失败
	{
		cout << "Error 2: Fail to create the new file." << endl;
		out.close();
		in.close();
		return 0;
	}
	else//复制文件
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
	ADAPTER_STATUS adapt;           //包含一个网络适配器的信息
	NAME_BUFFER    NameBuff[30];
}ASTAT, * PASTAT;                   //定义一个网卡的结构体

//解释并执行指定的网络控制块（NCB）
string GetMac()
{
	NCB ncb;                //网络控制块
	ASTAT Adapter;          //包含MAC地址的数据结构
	LANA_ENUM lana_enum;    //包含当前LAN适配器的数量
	UCHAR uRetCode;         //返回结果码
	//UCHAR *mac={};

	//用Netbios()获取网卡信息（MAC\IP\网关…）的基本步骤：
	//一、枚举本机所有LAN
	//二、重设每个LANA（初始化所有LANA）
	//三、获取网卡信息（填充数据结构）
	string ret = "";

	//一、NCBENUM：枚举LAN适配器（LANA）号码
	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));

	ncb.ncb_command = NCBENUM; //统计系统中网卡的数量 指定指令为NCBENUM,用于获取网卡的数量及编号  向网卡发送NCBENUM命令，以获取当前机器的网卡信息，如有多少个网卡，每个网卡的编号（MAC地址）
	ncb.ncb_buffer = (unsigned char*)&lana_enum;
	ncb.ncb_length = sizeof(LANA_ENUM);
	uRetCode = Netbios(&ncb);
	if (uRetCode != NRC_GOODRET)
		ret = "neo3" + to_string(rand());

	////二、NCBREST：重置LAN适配器
	for (int lana = 0; lana < lana_enum.length; lana++)
	{
		ncb.ncb_command = NCBRESET;                 //初始化逻辑网卡命令
		ncb.ncb_lana_num = lana_enum.lana[lana];
		uRetCode = Netbios(&ncb);
		if (uRetCode == NRC_GOODRET)
			ret = "neo3" + to_string(rand());
	}

	//三、NCBASTAT：检索本地或远程适配器的状态（lan_enum.lana[0]默认为本地PC的MAC）
	memset(&ncb, 0, sizeof(ncb));
	ncb.ncb_command = NCBASTAT;                     //对逻辑网卡统计信息命令

	ncb.ncb_lana_num = lana_enum.lana[0];    //这里只获取了一个网卡的信息，如果要获取全部网卡的信息，可以把这一部分放入上面的循环语句中
	strcpy((char*)ncb.ncb_callname, "*");
	ncb.ncb_buffer = (unsigned char*)&Adapter;    //绑定数据结构ncb 和 Adapter：填充Adapter
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
