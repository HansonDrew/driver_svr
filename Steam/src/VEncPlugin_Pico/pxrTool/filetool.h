#pragma once


#include <iostream>

using namespace std;
int CopyFilec(char *SourceFile, char *NewFile);
bool DeleteFileFuncW(wchar_t  *source);

bool DeleteFileFunc(string source);
bool ReNameFun(char *source, char *newname);
bool ReNameFun(string source, string newname);
int CopyFilec(string SourceFile, string NewFile);


bool GetSelfModulePath(char* path);