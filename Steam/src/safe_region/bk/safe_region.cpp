//========= Copyright Valve Corporation ============//

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <D3Dcompiler.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <stdio.h>
#include <string>
#include <cstdlib>

#include <openvr.h>
#include "stdafx.h"
#include "Calibration.h"

#include <iostream>
#include<fstream>
#include<vector>
#include <iomanip>
using namespace std;

#define Np       328
#define N        (Np*3)
#define Nsize  (Np/2-1)

using Microsoft::WRL::ComPtr;

void ThreadSleep(unsigned long nMilliseconds)
{
	::Sleep(nMilliseconds);
}

// Slots in the RenderTargetView descriptor heap
enum RTVIndex_t
{
	RTV_LEFT_EYE = 0,
	RTV_RIGHT_EYE,
	RTV_SWAPCHAIN0,
	RTV_SWAPCHAIN1,
	NUM_RTVS
};

// Slots in the ConstantBufferView/ShaderResourceView descriptor heap
enum CBVSRVIndex_t
{
	CBV_LEFT_EYE = 0,
	CBV_RIGHT_EYE,
	SRV_LEFT_EYE,
	SRV_RIGHT_EYE,
	SRV_TEXTURE_MAP,
	// Slot for texture in each possible render model
	SRV_TEXTURE_RENDER_MODEL0,
	SRV_TEXTURE_RENDER_MODEL_MAX = SRV_TEXTURE_RENDER_MODEL0 + vr::k_unMaxTrackedDeviceCount,
	// Slot for transform in each possible rendermodel
	CBV_LEFT_EYE_RENDER_MODEL0,
	CBV_LEFT_EYE_RENDER_MODEL_MAX = CBV_LEFT_EYE_RENDER_MODEL0 + vr::k_unMaxTrackedDeviceCount,
	CBV_RIGHT_EYE_RENDER_MODEL0,
	CBV_RIGHT_EYE_RENDER_MODEL_MAX = CBV_RIGHT_EYE_RENDER_MODEL0 + vr::k_unMaxTrackedDeviceCount,
	NUM_SRV_CBVS
};

static bool g_bPrintf = true;
static const int g_nFrameCount = 2; // Swapchain depth

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
	CMainApplication(int argc, char* argv[]);
	virtual ~CMainApplication();

	bool BInit();
	bool BInitD3D12();
	bool BInitCompositor();

	void SetupRenderModels();

	void Shutdown();

	void RunMainLoop();
	bool HandleInput();
	void ProcessVREvent(const vr::VREvent_t& event);
	void RenderFrame();

	bool SetupTexturemaps();
	static void GenMipMapRGBA(const UINT8* pSrc, UINT8** ppDst, int nSrcWidth, int nSrcHeight, int* pDstWidthOut, int* pDstHeightOut);

	void SetupScene();

	void UpdateControllerAxes();

	bool SetupStereoRenderTargets();
	void SetupCompanionWindow();
	void SetupCameras();

	void RenderStereoTargets();
	void RenderCompanionWindow();
	void RenderScene(vr::Hmd_Eye nEye);

	void UpdateHMDMatrixPose();


	bool CreateAllShaders();

	//void SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);

private:
	bool m_bDebugD3D12;
	bool m_bVerbose;
	bool m_bPerf;
	bool m_bVblank;
	int m_nMSAASampleCount;
	// Optional scaling factor to render with supersampling (defaults off, use -scale)
	float m_flSuperSampleScale;

	vr::IVRSystem* m_pHMD;
	vr::IVRRenderModels* m_pRenderModels;
	std::string m_strDriver;
	std::string m_strDisplay;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];

private: // SDL bookkeeping
	SDL_Window* m_pCompanionWindow;
	uint32_t m_nCompanionWindowWidth;
	uint32_t m_nCompanionWindowHeight;

private:
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	bool m_bShowCubes;

	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class

	int m_iSceneVolumeWidth;
	int m_iSceneVolumeHeight;
	int m_iSceneVolumeDepth;
	float m_fScaleSpacing;
	float m_fScale;

	int m_iSceneVolumeInit;                                  // if you want something other than the default 20x20x20

	float m_fNearClip;
	float m_fFarClip;

	unsigned int m_uiVertcount;
	unsigned int m_uiCompanionWindowIndexSize;



	unsigned int m_uiControllerVertcount;


	struct FramebufferDesc
	{
		ComPtr< ID3D12Resource > m_pTexture;
		ComPtr< ID3D12Resource > m_pDepthStencil;
	};
	FramebufferDesc m_leftEyeDesc;
	FramebufferDesc m_rightEyeDesc;

	bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc& framebufferDesc, RTVIndex_t nRTVIndex);

	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

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

//vr::HmdVector3_t  p[N] = { 0 };
//float t0[3] = { 0 };
//float t1[3] = { 0 };
//float t2[3] = { 0 };
//
//p[0].v[0] = t0[0];
//p[0].v[1] = t0[1];
//p[0].v[2] = t0[2];

//void fun1(std::vector<vr::HmdQuad_t> arrayList, int size, double x1, double y1, double x2, double y2, double z1, double z2, int begin)
void fun1(std::vector<vr::HmdQuad_t>* arrayList, int size, double x1, double y1, double x2, double y2, double z1, double z2, int begin)
{
	double dx = 0;
	double dy = 0;
	if (x1 != x2) {
		dx = (x2 - x1) / size;
	}
	if (y1 != y2) {
		dy = (y2 - y1) / size;
	}
	vr::HmdVector3_t  p1;
	vr::HmdVector3_t  p2;
	vr::HmdVector3_t  p3;
	vr::HmdVector3_t  p4;

	p1.v[0] = x1;
	p1.v[1] = z1;
	p1.v[2] = y1;

	p2.v[0] = x1;
	p2.v[1] = z2;
	p2.v[2] = y1;

	for (int i = 1; i < size + 1; i++)
	{
		double x_new = x1 + dx * i;
		double y_new = y1 + dy * i;
		if (abs(x_new - x2) < dx)
		{
			x_new = x2;
		}
		if (abs(y_new - y2) < dy)
		{
			y_new = y2;
		}

		//p3 = new Position(x_new, z2, y_new);
		//p4 = new Position(x_new, z1, y_new);

		p3.v[0] = x_new;
		p3.v[1] = z2;
		p3.v[2] = y_new;

		p4.v[0] = x_new;
		p4.v[1] = z1;
		p4.v[2] = y_new;

		//FourPosition fourPosition = new FourPosition(p1, p2, p3, p4);

		dprintf("====================begin  %d====================.\n", begin);
		(*arrayList)[begin] = { p1,p2,p3,p4 };
		begin = begin + 1;
		if (x_new == x2 && y_new == y2) {
			break;
		}
		p1 = p4;
		p2 = p3;
	}

}

void fun2(std::vector<vr::HmdQuad_t>* arrayList)
{
	//读入数据文件
	ifstream in("stdata.txt", ios::in);
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
			dprintf("================p is %d , v is %d ====np   is  %f====================.\n", i,j,p[i].v[j]);

		}
	}
	// dprintf("====================np====================.\n");

	(*arrayList)[0] = { p[0],p[1], p[2], p[3] };

	for (int m = 1; m < Nsize; m++)
	{
		(*arrayList)[m] = { p[2*m+1],p[2*m], p[2*m+2], p[2 * m + 3] };
	}
		//p1 = p4;
		//p2 = p3;
	


}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication(int argc, char* argv[])
	: m_pCompanionWindow(NULL)
	, m_nCompanionWindowWidth(640)
	, m_nCompanionWindowHeight(240)
	, m_pHMD(NULL)
	, m_pRenderModels(NULL)
	, m_bDebugD3D12(false)
	, m_bVerbose(false)
	, m_bPerf(false)
	, m_bVblank(false)
	, m_nMSAASampleCount(4)
	, m_flSuperSampleScale(1.0f)
	, m_iTrackedControllerCount(0)
	, m_iTrackedControllerCount_Last(-1)
	, m_iValidPoseCount(0)
	, m_iValidPoseCount_Last(-1)
	, m_iSceneVolumeInit(20)//nums of cubes
	, m_strPoseClasses("")
	, m_bShowCubes(true)
	//, m_nFrameIndex(0)
	//, m_fenceEvent(NULL)
	//, m_nRTVDescriptorSize(0)
	//, m_nCBVSRVDescriptorSize(0)
	//, m_nDSVDescriptorSize(0)
{
	//memset(m_pSceneConstantBufferData, 0, sizeof(m_pSceneConstantBufferData));

	for (int i = 1; i < argc; i++)
	{
		if (!stricmp(argv[i], "-dxdebug"))
		{
			m_bDebugD3D12 = true;
		}
		else if (!stricmp(argv[i], "-verbose"))
		{
			m_bVerbose = true;
		}
		else if (!stricmp(argv[i], "-novblank"))
		{
			m_bVblank = false;
		}
		else if (!stricmp(argv[i], "-msaa") && (argc > i + 1) && (*argv[i + 1] != '-'))
		{
			m_nMSAASampleCount = atoi(argv[i + 1]);
			i++;
		}
		else if (!stricmp(argv[i], "-supersample") && (argc > i + 1) && (*argv[i + 1] != '-'))
		{
			m_flSuperSampleScale = (float)atof(argv[i + 1]);
			i++;
		}
		else if (!stricmp(argv[i], "-noprintf"))
		{
			g_bPrintf = false;
		}
		else if (!stricmp(argv[i], "-cubevolume") && (argc > i + 1) && (*argv[i + 1] != '-'))
		{
			m_iSceneVolumeInit = atoi(argv[i + 1]);
			i++;
		}
	}
	// other initialization tasks are done in BInit
	memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));
};

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication()
{
	// work is done in Shutdown
	dprintf("Shutdown");
}

//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = NULL)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		dprintf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	//m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Other);
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
	//m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Overlay);//  

	if (eError != vr::VRInitError_None)
	{
		m_pHMD = NULL;
		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
		return false;
	}


	m_pRenderModels = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!m_pRenderModels)
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
		return false;
	}

	int nWindowPosX = 700;
	int nWindowPosY = 100;
	Uint32 unWindowFlags = SDL_WINDOW_SHOWN;//SDL_WINDOW_RESIZABLE;//SDL_WINDOW_SHOWN;

	m_pCompanionWindow = SDL_CreateWindow("hellovr [D3D12]", nWindowPosX, nWindowPosY, m_nCompanionWindowWidth, m_nCompanionWindowHeight, unWindowFlags);
	if (m_pCompanionWindow == NULL)
	{
		dprintf("%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	m_strDriver = "No Driver";
	m_strDisplay = "No Display";

	m_strDriver = GetTrackedDeviceString(m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	m_strDisplay = GetTrackedDeviceString(m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	std::string strWindowTitle = "hellovr [D3D12] - " + m_strDriver + " " + m_strDisplay;
	SDL_SetWindowTitle(m_pCompanionWindow, strWindowTitle.c_str());

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

	//if (m_pCompanionWindow)
	//{
	//	SDL_DestroyWindow(m_pCompanionWindow);
	//	m_pCompanionWindow = NULL;
	//}

	//SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	while (SDL_PollEvent(&sdlEvent) != 0)
	{
		if (sdlEvent.type == SDL_QUIT)
		{
			bRet = true;
		}
		else if (sdlEvent.type == SDL_KEYDOWN)
		{
			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE
				|| sdlEvent.key.keysym.sym == SDLK_q)
			{
				bRet = true;
			}
			if (sdlEvent.key.keysym.sym == SDLK_c)
			{
				m_bShowCubes = !m_bShowCubes;
			}
		}
	}

	// Process SteamVR events
	vr::VREvent_t event;
	while (m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		ProcessVREvent(event);
	}

	// Process SteamVR controller state
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(unDevice, &state, sizeof(state)))
		{
			m_rbShowTrackedDevice[unDevice] = state.ulButtonPressed == 0;
		}
	}

	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
	bool bQuit = false;

	SDL_StartTextInput();
	//SDL_ShowCursor( SDL_DISABLE );
	SDL_ShowCursor(SDL_ENABLE);

	while (!bQuit)
	{
		bQuit = HandleInput();

		//RenderFrame();
	}

	SDL_StopTextInput();
}

//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void CMainApplication::ProcessVREvent(const vr::VREvent_t& event)
{
	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
	{
		//SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
		dprintf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		dprintf("Device %u detached.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		dprintf("Device %u updated.\n", event.trackedDeviceIndex);
	}
	break;
	}
}

CalibrationContext CalCtx;

void LoadChaperoneBounds()
{
	vr::VRChaperoneSetup()->RevertWorkingCopy();

	uint32_t quadCount = 0;
	vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(nullptr, &quadCount);

	quadCount = Nsize;//400;
	//quadCount = 400;
	CalCtx.chaperone.geometry.resize(quadCount);
	vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(&CalCtx.chaperone.geometry[0], &quadCount);
	vr::VRChaperoneSetup()->GetWorkingStandingZeroPoseToRawTrackingPose(&CalCtx.chaperone.standingCenter);
	vr::VRChaperoneSetup()->GetWorkingPlayAreaSize(&CalCtx.chaperone.playSpaceSize.v[0], &CalCtx.chaperone.playSpaceSize.v[1]);
	CalCtx.chaperone.valid = true;
}

void ApplyChaperoneBounds()
{
	vr::VRChaperoneSetup()->RevertWorkingCopy();


	//fun1(&CalCtx.chaperone.geometry, 100, -2, -2, 2, -2, 0, 2.5, 0);
	//fun1(&CalCtx.chaperone.geometry, 100, 2, -2, 2, 2, 0, 2.5, 100);
	//fun1(&CalCtx.chaperone.geometry, 100, 2, 2, -2, 2, 0, 2.5, 200);
	//fun1(&CalCtx.chaperone.geometry, 100, -2, 2, -2, -2, 0, 2.5, 300);

	fun2(&CalCtx.chaperone.geometry);

	vr::VRChaperoneSetup()->SetWorkingCollisionBoundsInfo(&CalCtx.chaperone.geometry[0], CalCtx.chaperone.geometry.size());
	vr::VRChaperoneSetup()->SetWorkingStandingZeroPoseToRawTrackingPose(&CalCtx.chaperone.standingCenter);
	vr::VRChaperoneSetup()->SetWorkingPlayAreaSize(CalCtx.chaperone.playSpaceSize.v[0], CalCtx.chaperone.playSpaceSize.v[1]);
	vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Live);
}


int main(int argc, char* argv[])
{
	CMainApplication* pMainApplication = new CMainApplication(argc, argv);

	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		return 1;
	}

	LoadChaperoneBounds();

	if (CalCtx.chaperone.valid)
	{
		uint32_t quadCount = 0;
		vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(nullptr, &quadCount);
		//vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(&CalCtx.chaperone.geometry[0], &quadCount);
		// 
		//if (quadCount != CalCtx.chaperone.geometry.size())
		{
			ApplyChaperoneBounds();
			//LoadChaperoneBounds();
		}
	}
	while (1)
	{
		Sleep(1000);
	}
	//pMainApplication->RunMainLoop();

	pMainApplication->Shutdown();

	delete pMainApplication;

	return 0;
}
