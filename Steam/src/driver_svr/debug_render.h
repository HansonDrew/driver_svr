#pragma once
#include <windows.h>
#include <d3d11.h>
class DebugRender
{
public:
	DebugRender();
	~DebugRender();
	bool InitWindows(int fps, int width, int height);
	bool CreateD3DComponent();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	int fps_;
	int w_width_;
	int w_height_;
	HWND      hwnd_;
	HINSTANCE  app_inst_;
	ID3D11Device* device_;
	ID3D11DeviceContext* d3d_context_;
};

