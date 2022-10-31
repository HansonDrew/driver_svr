#include "stringtool.h"
#include<string>
#include <vector>
#include <cctype>
#include <iostream>
#include <algorithm>
#include <functional>  
#include <atlbase.h>
using namespace std;

string& ltrim(string &str) {
	string::iterator p = find_if(str.begin(), str.end(), not1(ptr_fun<int, int>(isspace)));
	str.erase(str.begin(), p);
	return str;
}

string& rtrim(string &str) {
	string::reverse_iterator p = find_if(str.rbegin(), str.rend(), not1(ptr_fun<int, int>(isspace)));
	str.erase(p.base(), str.end());
	return str;
}

 string& trim(string &str) {
	ltrim(rtrim(str));
	return str;
}

void Split(const std::string& src, const std::string& separator, std::vector<std::string>& dest) //字符串分割到数组
{
	//参数1：要分割的字符串；参数2：作为分隔符的字符；参数3：存放分割后的字符串的vector向量
	string str = src;
	string substring = "";
	string::size_type start = 0, index;
	dest.clear();
	index = str.find_first_of(separator, start);
	do
	{
		if (index != string::npos)
		{
			substring = str.substr(start, index - start);
			dest.push_back(substring);
			start = index + separator.size();
			index = str.find(separator, start);
			if (start == string::npos) break;
		}
	} while (index != string::npos);

	//the last part
	substring = str.substr(start);
	dest.push_back(substring);
}
void wcharTochar(const wchar_t *wchar, char *chr, int length)
{
	WideCharToMultiByte(CP_ACP, 0, wchar, -1,
		chr, length, NULL, NULL);
}
std::string WString2String(const std::wstring& ws)
{
	size_t len = ws.length() + 1;
	size_t converted = 0;
	char*CStr;
	CStr = (char*)malloc(len * sizeof(char));
	wcstombs_s(&converted, CStr, len, ws.c_str(), _TRUNCATE);
	std::string strResult = CStr;
	free(CStr);
	return strResult;
}

// string => wstring
std::wstring String2WString(const std::string& s)
{
	size_t len = s.length() + 1;
	size_t converted = 0;
	wchar_t*WStr;
	WStr = (wchar_t*)malloc(len * sizeof(wchar_t));
	mbstowcs_s(&converted, WStr, len, s.c_str(), _TRUNCATE);
	wstring wstrResult(WStr);
	free(WStr);
	return wstrResult;
}


void deletesub(string &str, const string &sub, int sublen)
{
	int m, flag = 0, num = 0;           //num是子串出现的次数
	while (flag == 0)
	{
		m = str.find(sub);
		if (m < 0)
			flag = 1;
		else
		{
			str.erase(m, sublen);           //删除子串
			num++;
		}
	}
	
}
