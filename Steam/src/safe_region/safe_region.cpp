#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )// 设置入口地址
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <D3Dcompiler.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include "process.h"
#include <openvr.h>
#include "stdafx.h"
#include "Calibration.h"
//#include "logger.h"
#include <iostream>
#include<fstream>
#include<vector>
#include <iomanip>
#include "logerSafeRegion.h"

#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
//using namespace pxr_base;
FILE* receive_addlog = fopen("receive_log.txt", "wb+");

#define Np       328
#define NpSafe       800
#define N        (Np*3)
#define Nsize  (Np/2-1)

#define N_Total  NpSafe*8+4

using Microsoft::WRL::ComPtr;

struct Vector2F {
	float x;
	float y;
};

//vr::HmdVector3_t  p[Np];
vr::HmdVector3_t  p[NpSafe];
uint32_t  mSafeRegionPointNum = 4;

uint32_t quadCount = 4;
uint32_t timeoutCount = 0;
bool flag_safeRegion = false;

static bool g_bPrintf = true;
static const int g_nFrameCount = 2; // Swapchain depth
vr::EVRInitError eError = vr::VRInitError_None;

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
	CMainApplication(int argc, char* argv[]);
	virtual ~CMainApplication();

	bool BInit();
	void Shutdown();

private:

	vr::IVRSystem* m_pHMD;
	vr::IVRRenderModels* m_pRenderModels;

};

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------

void dprintf(const char* fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	if (g_bPrintf)
		printf("%s", buffer);

	OutputDebugStringA(buffer);
}

void fun2(std::vector<vr::HmdQuad_t>* arrayList)
{
	//读入数据文件
	ifstream in("F:\\PICO\\safeRegion\\sparrow_driver\\RVRSDK_2.3.8_Source\\RVRRenderer\\Steam\\etc\\driver\\pico\\bin\\win64\\stdata.txt", ios::in);
	if (!in.is_open())
	{
		cout << "open error!" << endl;
		exit(0);
	}
	//讲数据文件数据放入数组
	int i = 0;
	vector<double> v1(N);
	while (!in.eof() && i < N)
	{
		in >> v1[i];
		i++;
	}

	vr::HmdVector3_t  p[Np];

	for (int i = 0; i < Np; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			p[i].v[j] = v1[j + i * 3];
			//dprintf("================p is %d , v is %d ====np   is  %f====================.\n", i,j,p[i].v[j]);

		}
	}
	// dprintf("====================np====================.\n");

	(*arrayList)[0] = { p[0],p[1], p[2], p[3] };

	for (int m = 1; m < Nsize; m++)
	{
		(*arrayList)[m] = { p[2 * m + 1],p[2 * m], p[2 * m + 2], p[2 * m + 3] };
	}
	//p1 = p4;
	//p2 = p3;

	LOG("================end fun2====================.\n");


}

void GetSafeRegionXYtoZero(Vector2F* buf_safe, int bufsize)
{
	LOG("================GetSafeRegionXYtoZero   enter====================.\n");
	for (int i = 0; i < bufsize; i++)
	{
		p[i].v[0] = 0;
		p[i].v[1] = 0;
		p[i].v[2] = 0;
	}
	flag_safeRegion = true;

}


void GetSafeRegionXY(Vector2F* buf_safe, int bufsize)
{
	LOG("================GetSafeRegionXY   enter====================.\n");
	int m = 1;
	for (int i = 0; i < bufsize; i++)
	{
		p[i].v[0] = buf_safe[i].x;
		if (m % 2)
		{
			p[i].v[1] = 0;//buf_safe[i].y;			
		}
		else
		{
			p[i].v[1] = 2.5;
		}
		p[i].v[2] = buf_safe[i].y;
		m++;
	}
	//for (int i = 0; i < bufsize; i++)
	//{
	//	LOG2("====index  i  is ====", i);
	//	for (int j = 0; j < 3; j++)
	//	{
	//		LOG2("================ p[i].v[j]====================",p[i].v[j]);
	//		//cout << " ====p[i].v[j]====" << p[i].v[j] << endl;
	//	}
	//}

	flag_safeRegion = true;
	//LOG("================ flag_safeRegion==== true=================");

}
// dprintf("====================np====================.\n");
void funChangeAndApplySafeRegion(std::vector<vr::HmdQuad_t>* arrayList)
{
	(*arrayList)[0] = { p[0],p[1], p[2], p[3] };

	//for (int m = 1; m < NsizeSafe; m++)
	int m_size = (mSafeRegionPointNum / 2 - 1);
	for (int m = 1; m < m_size; m++)
	{
		(*arrayList)[m] = { p[2 * m], p[2 * m + 1],p[2 * m + 2], p[2 * m + 3] };
		//(*arrayList)[m] = { p[2*m+1],p[2*m], p[2*m+2], p[2 * m + 3] };
	}

	LOG("================funChangeAndApplySafeRegion====================.\n");
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication(int argc, char* argv[])
	: m_pHMD(NULL)
	, m_pRenderModels(NULL)
{

};

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication()
{
	// work is done in Shutdown
	LOG("Shutdown");
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit()
{

	// Loading the SteamVR Runtime
	//vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Other);
	//m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
	//m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Overlay);//  

	if (eError != vr::VRInitError_None)
	{
		m_pHMD = NULL;
		char buf[1024];
		//sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		LOG("=========eError != vr::VRInitError_None=========！");

		//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
		return false;
	}

	//vr::VRChaperoneSetup()->RoomSetupStarting();
	//LOG("=========RoomSetupStarting=========！");

	//m_pRenderModels = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	//if (!m_pRenderModels)
	//{
	//	m_pHMD = NULL;
	//	vr::VR_Shutdown();

	//	char buf[1024];
	//	sprintf_s(buf, sizeof(buf), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
	//	//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
	//	return false;
	//}


	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	if (m_pHMD)
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

}



void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		LOG("初始化套接字库失败！");
	}
	else {
		LOG("初始化套接字库成功！");
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		LOG("套接字库版本号不符！");
		WSACleanup();
	}
	else {
		LOG("套接字库版本正确！");
	}
	//填充服务端地址信息

}

//int server_socket() {
unsigned int __stdcall serverSocketThread(PVOID param_s)
{
	//if (flag_safeRegion)
	//{
	//	flag_safeRegion = false;
	//}
	//定义长度变量
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	int f_size = 0;
	//定义发送缓冲区和接受缓冲区
	//char send_buf[100];
	char recv_buf[N_Total];
	Vector2F* buf_safe = new Vector2F[NpSafe];

	//定义服务端套接字，接受请求套接字
	SOCKET s_server;
	SOCKET s_accept;
	//服务端地址客户端地址
	SOCKADDR_IN server_addr;
	SOCKADDR_IN accept_addr;
	initialization();
	//填充服务端信息
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(29789);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		LOG("套接字绑定失败！");
		WSACleanup();
	}
	else {
		LOG("套接字绑定成功！");
	}
	//设置套接字为监听状态
	if (listen(s_server, SOMAXCONN) < 0) {
		LOG("设置监听状态失败！");
		WSACleanup();
	}
	else {
		LOG("设置监听状态成功！");
	}
	LOG("服务端正在监听连接，请稍候....");
	//接受连接请求
	len = sizeof(SOCKADDR);
	s_accept = accept(s_server, (SOCKADDR*)&accept_addr, &len);
	if (s_accept == SOCKET_ERROR) {
		LOG("连接失败！");
		WSACleanup();
		return 0;
	}
	LOG("连接建立，准备接受数据");
	//接收数据
	while (1)
	{
		recv_len = recv(s_accept, recv_buf, N_Total, 0);
		//if (receive_addlog != NULL)
		if (recv_len > 0)
		{
			//fwrite(recv_buf, sizeof(char), recv_len, receive_addlog);
			//LOG("fwrite    接受数据.......");

			memcpy(&(f_size), recv_buf, sizeof(int));
			//mSafeRegionPointNum = f_size;
			//LOG2("====mSafeRegionPointNum is ====", f_size);

			LOG("接受数据.......");
			for (int i = 0; i < f_size; i++)
			{
				memcpy(&(buf_safe[i].x), (recv_buf + 4 + 8 * i), sizeof(float));
				memcpy(&(buf_safe[i].y), (recv_buf + 8 + 8 * i), sizeof(float));
				auto tex_x = buf_safe[i].x;
				auto tex_y = buf_safe[i].y;

				//LOG2("====index  is ====", i);
				//LOG2("====f_size is ====", f_size);
				//LOG2("====rec_v_buf->x is ====", tex_x);
				//LOG2("====rec_v_buf->y is ====", tex_y);
				//cout << "====rec_v_buf->y is ====" << tex_y << " ====buf_safe[i].y====" << buf_safe[i].y << endl;
			}
			/*mSafeRegionPointNum = f_size;
			LOG2("====mSafeRegionPointNum is ====", f_size);*/

			//GetSafeRegionXY(buf_safe, f_size);
		
			if (f_size == 0)
			{
				LOG("=====GetSafeRegionXYtoZero.......");
				mSafeRegionPointNum = 16;
				GetSafeRegionXYtoZero(buf_safe, 16);
			}
			else
			{
				mSafeRegionPointNum = f_size;
				GetSafeRegionXY(buf_safe, f_size);
			}

		}
		else
		{
			//LOG("=====recv_len<=0.......");
			//flag_safeRegion = false;
			Sleep(1);
		}
	}
	//关闭套接字
	closesocket(s_server);
	closesocket(s_accept);
	//释放DLL资源
	WSACleanup();
	return 0;
}


CalibrationContext CalCtx;

void LoadChaperoneBounds()
{
	vr::VRChaperoneSetup()->RevertWorkingCopy();

	//uint32_t quadCount = 0;
	vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(nullptr, &quadCount);

	if (mSafeRegionPointNum < 4)
	{
		quadCount = 4;
		LOG("====   quadCount=4   ==");
	}
	else
	{
		quadCount = (mSafeRegionPointNum / 2 - 1);//NsizeSafe;//400;
		LOG2("====   quadCount = (mSafeRegionPointNum / 2 - 1) ==", quadCount);

		//quadCount = 400;
	}
	CalCtx.chaperone.geometry.resize(quadCount);
	LOG2("====   quadCount = resize ==", quadCount);
	vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(&CalCtx.chaperone.geometry[0], &quadCount);
	vr::VRChaperoneSetup()->GetWorkingStandingZeroPoseToRawTrackingPose(&CalCtx.chaperone.standingCenter);
	vr::VRChaperoneSetup()->GetWorkingPlayAreaSize(&CalCtx.chaperone.playSpaceSize.v[0], &CalCtx.chaperone.playSpaceSize.v[1]);
	CalCtx.chaperone.valid = true;
	LOG("====   LoadChaperoneBounds=end   ==");

}

void ApplyChaperoneBounds()
{
	vr::VRChaperoneSetup()->RevertWorkingCopy();
	vr::VRChaperoneSetup()->SetWorkingStandingZeroPoseToRawTrackingPose(&CalCtx.chaperone.standingCenter);
	vr::VRChaperoneSetup()->SetWorkingPlayAreaSize(CalCtx.chaperone.playSpaceSize.v[0], CalCtx.chaperone.playSpaceSize.v[1]);

	//fun2(&CalCtx.chaperone.geometry);
	funChangeAndApplySafeRegion(&CalCtx.chaperone.geometry);

	vr::VRChaperoneSetup()->SetWorkingCollisionBoundsInfo(&CalCtx.chaperone.geometry[0], CalCtx.chaperone.geometry.size());
	LOG2("====CalCtx.chaperone.geometry.size()=====", CalCtx.chaperone.geometry.size());
	LOG2("====CalCtx.chaperone.geometry[0].vCorners[0].v[0]=====", CalCtx.chaperone.geometry[0].vCorners[0].v[0]);
	LOG2("====CalCtx.chaperone.geometry[0].vCorners[0].v[2]=====", CalCtx.chaperone.geometry[0].vCorners[0].v[2]);

	//vr::VRChaperoneSetup()->SetWorkingStandingZeroPoseToRawTrackingPose(&CalCtx.chaperone.standingCenter);
	//vr::VRChaperoneSetup()->SetWorkingPlayAreaSize(CalCtx.chaperone.playSpaceSize.v[0], CalCtx.chaperone.playSpaceSize.v[1]);

	//vr::VRChaperoneSetup()->ShowWorkingSetPreview();
	bool ret=vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Live);

	//vr::VRChaperone()->ForceBoundsVisible(true);
	LOG("================ForceBoundsVisible====================.\n");
	LOG("================end  ApplyChaperoneBounds====================.\n");

}


int main(int argc, char* argv[])
{
	CMainApplication* pMainApplication = new CMainApplication(argc, argv);

	//LogWriter::SetLogFilePrefix("DocumentSafeRegion");
	//LogWriter::SetOuputLogLevel(LogLevel::kLogLevelDebug);
	LOG("safe region===begin main ===");

	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		return 1;
	}

	//server_socket();
	HANDLE rethandle = (HANDLE)_beginthreadex(NULL, 0, &serverSocketThread, NULL, 0, NULL);

	//LoadChaperoneBounds();
	//flag_safeRegion = true;
	while (1)
	{
		//if (CalCtx.chaperone.valid && flag_safeRegion)
		if (flag_safeRegion)
		{
			LOG("= flag_safeRegion   true   MAIN==");
			//uint32_t quadCount = 0;
			vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(nullptr, &quadCount);
			LOG2("==main  while(1)==   quadCount ==", quadCount);

			LoadChaperoneBounds();
			//LOG2("==main   after  LoadChaperoneBounds ==   quadCount ==", quadCount);
			if (CalCtx.chaperone.valid)
			{
				ApplyChaperoneBounds();
				flag_safeRegion = false;
				Sleep(1000);
				break;
			}
			//else
			//{
			//	LOG("====CalCtx.chaperone.valid is not ready====");
			//}

		}
		else
		{
			LOG("====flag_safeRegion is false in main====");
			timeoutCount++;
			Sleep(1000);
			if (timeoutCount > 100)
			{
				break;
			}
		}
	}

	pMainApplication->Shutdown();
	LOG("====Shutdown====");
	delete pMainApplication;
	LOG("====end of  main====");
	return 0;
}
