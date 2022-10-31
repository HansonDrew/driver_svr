
#include <iostream>
#include <fstream>
#include <windows.h>
#include <stdlib.h>
#include <io.h>
#include "filetool.h"
#include "stringtool.h"
using namespace std;
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