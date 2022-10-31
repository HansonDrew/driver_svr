#pragma once

#include <d3d11_1.h>
#include <string>
namespace pxr
{
    class D3D11ContextManager
    {

    public:

        static bool GetHardwareAdapter(IDXGIAdapter1*& adapter);

        static bool CreateDevice(ID3D11Device*& device_out, ID3D11DeviceContext*& context_out, IDXGIAdapter* adapter_nullable, bool is_multi_thread, bool is_debug);
        static void* GetSharedResource(ID3D11Texture2D* texture);
        static bool GetTextureInfo(ID3D11Texture2D* texture, DXGI_FORMAT* format, float* width, float* height);
        static ID3D11Texture2D* CreateTexture2D(ID3D11Device* device, unsigned int bind_flag, DXGI_FORMAT format, int width, int height
            , int sample_count, bool is_shared, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, unsigned int cpu_access_flag = 0);
        static ID3D11Texture2D* CreateRenderTarget(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int sample_count = 1);
        static ID3D11Texture2D* CreateShaderResource(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int sample_count = 1);
        static ID3D11Texture2D* CreateSharedTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int sample_count = 1);
        static bool DeleteTexture2D(ID3D11Texture2D*& texture);

       

        static void ClearRenderTarget(ID3D11DeviceContext* context, ID3D11RenderTargetView* render_target, float* color);

        static bool IsDeviceDebugAvailable();

        static bool OpenSharedTexture(ID3D11Device* device, const void* shared_handle, ID3D11Texture2D*& shared_texture_out);
        static bool CloseSharedTexture(ID3D11Texture2D*& shared_texture);

        static IDXGIKeyedMutex* AcquireMutex(ID3D11Texture2D* keyed_texture);
        static bool ReleaseMutex(IDXGIKeyedMutex* keyed_mutex);
    };
}

