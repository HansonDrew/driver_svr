#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )
 


#include "d3d11_context_manager.h"

 

#include <wincodec.h>
#include <dxgi.h>
#include <d3d11_4.h>


namespace pxr
{
    bool D3D11ContextManager::GetHardwareAdapter(IDXGIAdapter1*& adapter)
    {
        adapter = nullptr;

        IDXGIFactory1* dxgiFactory;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
        {
            return false;
        }

        IDXGIAdapter1* adapter1 = nullptr;
        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(adapterIndex, &adapter1); adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter1->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }
            break;
        }

        if (adapter1 == nullptr)
        {
            return false;
        }

        adapter = adapter1;
        return true;
    }

    bool D3D11ContextManager::CreateDevice(ID3D11Device*& device_out, ID3D11DeviceContext*& context_out, IDXGIAdapter* adapter_nullable, bool is_multi_thread, bool is_debug)
    {

        HRESULT hresult = E_FAIL;


        D3D_FEATURE_LEVEL feature_levels[] =
        {
            D3D_FEATURE_LEVEL_11_1
        };
        unsigned int feature_levels_count = ARRAYSIZE(feature_levels);

        unsigned int device_flags = 0;
        if (is_debug)
        {
            device_flags |= D3D11_CREATE_DEVICE_DEBUG;
        }

        D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_UNKNOWN;
        if (adapter_nullable == nullptr)
        {
            driver_type = D3D_DRIVER_TYPE_HARDWARE;
        }

        D3D_FEATURE_LEVEL actual_feature_level;
        // Create the device and device context objects
        hresult = D3D11CreateDevice(adapter_nullable, driver_type, nullptr,
            device_flags, feature_levels, feature_levels_count, D3D11_SDK_VERSION,
            &device_out, &actual_feature_level, &context_out);
        if (FAILED(hresult))
        {
            return false;
        }


        ID3D11Multithread* d3d11_multithread = nullptr;
        hresult = context_out->QueryInterface(__uuidof(ID3D11Multithread), (void**)&d3d11_multithread);
        if (SUCCEEDED(hresult)) {
            d3d11_multithread->SetMultithreadProtected(is_multi_thread);
            d3d11_multithread->Release();
        }

        if (is_debug)
        {
            ID3D11Debug* debug;
            if (SUCCEEDED(device_out->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug)))
            {
                ID3D11InfoQueue* info_queue;
                if (SUCCEEDED(debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info_queue)))
                {
                    info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                    info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
                    D3D11_MESSAGE_ID hide[] =
                    {
                        D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                    };
                    D3D11_INFO_QUEUE_FILTER filter = {};
                    filter.DenyList.NumIDs = ARRAYSIZE(hide);
                    filter.DenyList.pIDList = hide;
                    info_queue->AddStorageFilterEntries(&filter);
                }
            }
        }

        return true;
    }

    void* D3D11ContextManager::GetSharedResource(ID3D11Texture2D* texture)
    {
        void* handle_ptr = nullptr;

        IDXGIResource1* resource;
        texture->QueryInterface(__uuidof(IDXGIResource1), (void**)&resource);

        resource->CreateSharedHandle(nullptr,
            DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, NULL,
            &handle_ptr);

        resource->Release();
        return handle_ptr;
    }

    bool D3D11ContextManager::GetTextureInfo(ID3D11Texture2D* texture, DXGI_FORMAT* format, float* width, float* height)
    {

        if (texture == nullptr)
        {
            return false;
        }

        D3D11_TEXTURE2D_DESC desc;
        texture->GetDesc(&desc);
        if (width != nullptr)
            *width = (float)desc.Width;
        if (height != nullptr)
            *height = (float)desc.Height;
        if (format != nullptr)
            *format = desc.Format;

        return true;
    }

    ID3D11Texture2D* D3D11ContextManager::CreateTexture2D(ID3D11Device* device, unsigned int bind_flag, DXGI_FORMAT format, int width, int height,
        int sample_count, bool is_shared, D3D11_USAGE usage, unsigned int cpu_access_flag)
    {
        ID3D11Texture2D* texture = nullptr;

        DXGI_SAMPLE_DESC sampledesc;
        sampledesc.Count = sample_count;
        sampledesc.Quality = 0;

        D3D11_TEXTURE2D_DESC desc;
        desc.ArraySize = 1;
        desc.CPUAccessFlags = cpu_access_flag > 0 ? cpu_access_flag : 0;
        desc.BindFlags = bind_flag;
        desc.Format = format;
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.MiscFlags = is_shared ? D3D11_RESOURCE_MISC_SHARED : 0;
        desc.SampleDesc = sampledesc;
        desc.Usage = usage;
        device->CreateTexture2D(&desc, 0, &texture);

        return texture;
    }

    ID3D11Texture2D* D3D11ContextManager::CreateRenderTarget(ID3D11Device* device, DXGI_FORMAT format, int width,
        int height, int sample_count)
    {
        return CreateTexture2D(device, D3D11_BIND_RENDER_TARGET, format, width, height, sample_count, false);
    }

    ID3D11Texture2D* D3D11ContextManager::CreateShaderResource(ID3D11Device* device, DXGI_FORMAT format, int width,
        int height, int sample_count)
    {
        return CreateTexture2D(device, D3D11_BIND_SHADER_RESOURCE, format, width, height, sample_count, false);
    }

    ID3D11Texture2D* D3D11ContextManager::CreateSharedTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width,
        int height, int sample_count)
    {
        return CreateTexture2D(device, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, format, width, height, sample_count, true);
    }

    bool D3D11ContextManager::DeleteTexture2D(ID3D11Texture2D*& texture)
    {
        if (texture == nullptr)
        {
            return false;
        }

        texture->Release();
        texture = nullptr;
        return true;
    }

    void D3D11ContextManager::ClearRenderTarget(ID3D11DeviceContext* context, ID3D11RenderTargetView* render_target,
        float* color)
    {
        context->ClearRenderTargetView(render_target, color);
    }

    bool D3D11ContextManager::IsDeviceDebugAvailable()
    {
        const auto hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_NULL,
            0,
            D3D11_CREATE_DEVICE_DEBUG,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            nullptr,
            nullptr,
            nullptr
        );

        return SUCCEEDED(hr);
    }

    bool D3D11ContextManager::OpenSharedTexture(ID3D11Device* device, const void* shared_handle, ID3D11Texture2D*& shared_texture_out)
    {
        const auto hr = device->OpenSharedResource(const_cast<HANDLE>(shared_handle)
            , __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&shared_texture_out));
        return SUCCEEDED(hr);
    }

    bool D3D11ContextManager::CloseSharedTexture(ID3D11Texture2D*& shared_texture)
    {
        if (shared_texture == nullptr)
        {
            return false;
        }

        shared_texture->Release();
        shared_texture = nullptr;
        return true;
    }


    IDXGIKeyedMutex* D3D11ContextManager::AcquireMutex(ID3D11Texture2D* keyed_texture)
    {
        IDXGIKeyedMutex* keyed_mutex = nullptr;
        if (keyed_texture != nullptr)
        {
            if (SUCCEEDED(keyed_texture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&keyed_mutex)))
            {
                if (keyed_mutex->AcquireSync(0, 10) != S_OK)
                {
                    keyed_mutex->Release();
                    keyed_mutex = nullptr;
                }
            }
        }

        return keyed_mutex;
    }

    bool D3D11ContextManager::ReleaseMutex(IDXGIKeyedMutex* keyed_mutex)
    {
        bool res = false;
        if (keyed_mutex)
        {
            res = SUCCEEDED(keyed_mutex->ReleaseSync(0));
            keyed_mutex->Release();
        }

        return res;
    }

}
