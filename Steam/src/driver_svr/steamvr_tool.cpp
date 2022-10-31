#include "steamvr_tool.h"
#include <Windows.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "driver_define.h"
#include "stringtool.h"
#include "driverlog.h"
#include ",,/../../json/json.h"
Json::String readInputFile(const char* path) {
	FILE* file = fopen(path, "rb");
	if (!file)
		return "";
	fseek(file, 0, SEEK_END);
	long const size = ftell(file);
	size_t const usize = static_cast<unsigned long>(size);
	fseek(file, 0, SEEK_SET);
	char* buffer = new char[size + 1];
	buffer[size] = 0;
	Json::String text;
	if (fread(buffer, 1, usize, file) == usize)
		text = buffer;
	fclose(file);
	delete[] buffer;
	return text;
}

void writeJsonToFile(Json::Value jsonValue, const char* path) {
	Json::StyledWriter sw;
	cout << sw.write(jsonValue) << endl << endl;

	ofstream os;
	os.open(path, std::ios::out | std::ios::binary);
	if (!os.is_open())
		cout << "error£ºcan not find or create the file which named \" demo.json\"."
		<< endl;
	os << sw.write(jsonValue);
	os.close();
}


void ChangeVRSettingJson()
{
	char str[256];
	wchar_t* wstr = reinterpret_cast<wchar_t*>(str);
	GetSystemDirectory(wstr, 50);
	std::wstring pathStr = wstr;

	pathStr.replace(1, pathStr.length() - 1, L"");
	DWORD len = 50;
	GetUserName(wstr, &len);
	pathStr = pathStr + L"://Users//" + wstr +
		L"//AppData//Local//openvr//openvrpaths.vrpath";
	std::string openvrpath = WString2String(pathStr);

	Json::Features features;
	Json::Reader reader(features);
	Json::Value root;
	Json::String input = readInputFile(openvrpath.c_str());
	bool parsingSuccessful =
		reader.parse(input.data(), input.data() + input.size(), root);


	if (root["config"].isNull())
	{
		DriverLog("root config error");
		return;
	}
	vector<string> configlist;
	int k = root["config"].size();
	for (int index = 0; index < root["config"].size(); ++index) {
		Json::Value universevar = root["config"][index];
		const Json::String configPath =
			root["config"][index].asString();
		configlist.push_back(configPath);
	}
	for (int i = 0; i < configlist.size(); i++)
	{
		string configpath = configlist[i] + "//steamvr.vrsettings";
		Json::Features features;
		Json::Reader reader(features);
		Json::Value root;
		Json::String input = readInputFile(configpath.c_str());
		bool parsingSuccessful =
			reader.parse(input.data(), input.data() + input.size(), root);
		if (root["steamvr"].isNull()) {
			DriverLog("root steamvr error");
			continue;
		}
		if (root["steamvr"]["motionSmoothing"].isNull()) {
			Json::Value motionSmoothing;
			root["steamvr"]["motionSmoothing"] = Json::Value("false");
		}
		root["steamvr"]["motionSmoothing"] = "false";
		writeJsonToFile(root, configpath.c_str());
	}

}
int GetSteamvrVersion()
{

	char str[50];
	wchar_t* wstr = reinterpret_cast<wchar_t*>(str);
	GetSystemDirectory(wstr, 50);
	wstring pathStr = wstr;

	pathStr.replace(1, pathStr.length() - 1, L"");
	DWORD len = 50;
	GetUserName(wstr, &len);
	pathStr = pathStr + L"://Users//" + wstr +
		L"//AppData//Local//openvr//openvrpaths.vrpath";
	string openvrpath = WString2String(pathStr);

	Json::Features features;
	Json::Reader reader(features);
	Json::Value root;
	Json::String input = readInputFile(openvrpath.c_str());
	bool parsingSuccessful =
		reader.parse(input.data(), input.data() + input.size(), root);


	if (root["config"].isNull())
	{
		DriverLog("root config error");
		return STEAMVRVERSIONDOOR;
	}
	vector<string> configlist;
	int k = root["config"].size();
	for (int index = 0; index < root["config"].size(); ++index) {
		Json::Value universevar = root["config"][index];
		const Json::String configPath =
			root["config"][index].asString();
		configlist.push_back(configPath);
	}
	int ret = STEAMVRVERSIONDOOR;
	for (int i = 0; i < configlist.size(); i++)
	{
		string last_version = "";
		string configpath = configlist[i] + "//steamvr.vrsettings";
		Json::Features features;
		Json::Reader reader(features);
		Json::Value root;
		Json::String input = readInputFile(configpath.c_str());
		bool parsingSuccessful =
			reader.parse(input.data(), input.data() + input.size(), root);
		if (root["lastVersionNotice"].isNull()) {
			DriverLog("get lastVersionNotice error");
			continue;
		}
		last_version = root["lastVersionNotice"].asString();
		if (last_version.length() > 0)
		{
			last_version.erase(std::remove(last_version.begin(), last_version.end(), '.'), last_version.end());
			ret = atoi(last_version.c_str());
			break;
		}
	}

	return ret;
}
