//file add by dzhuang
#pragma once

#include<string>
#include <vector>

using namespace std;

string& ltrim(string &str);
string& rtrim(string &str); 
string& trim(string &str);
void Split(const std::string& src, const std::string& separator, std::vector<std::string>& dest);
void wcharTochar(const wchar_t *wchar, char *chr, int length);
std::string WString2String(const std::wstring& ws);
// string => wstring
std::wstring String2WString(const std::string& s);
void deletesub(string &str, const string &sub, int sublen);