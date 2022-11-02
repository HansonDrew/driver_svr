
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