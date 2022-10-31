#include "debug_render.h"
#include "driverlog.h"
#include "D3DHelper.h"
DebugRender::DebugRender() 
{
	w_height_ =512;
	w_width_ = 512;
}
DebugRender::~DebugRender() 
{
}
DebugRender* gd3dApp = 0;
LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);//message ¼àÌý°´¼ü
}

LRESULT DebugRender::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)//wParam ´æ·Å°´¼ü
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPED | WS_VISIBLE | WS_CLIPSIBLINGS);
			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
			MoveWindow(hwnd, 0, 0, w_width_ , w_height_, true); 
		}


	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool DebugRender::InitWindows(int fps,int width,int height)
{
	w_width_ = width;
	w_height_ = height;
	fps_ = fps;
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = app_inst_;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"DebugRender";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}
	RECT R = { 0, 0,w_width_, w_height_ };
	
	DriverLog("[0316][DistortionRenderer::ConfigureWindows][AdjustWindowRect]  %d %d %d %d\n", R.right, R.left, R.bottom, R.top);
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	width = R.right - R.left;
	height = R.bottom - R.top;

	DriverLog("[0316][DistortionRenderer::ConfigureWindows][AdjustWindowRect]   %d %d\n", width, height);

	hwnd_ = CreateWindow(L"DebugRender", L"Pico Streamer",WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, app_inst_, 0);
	if (!hwnd_)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}
 
	ShowWindow(hwnd_, SW_RESTORE);
	UpdateWindow(hwnd_);
	return true;

}

bool DebugRender::CreateD3DComponent() 
{
	D3DHelper::CreateDevice0(device_, d3d_context_);

	return false;
}