#pragma once

#include <mmdeviceapi.h>
#include <audioclient.h>

#include <memory>
#include <string>
#include <functional>

#include "PolicyConfig.h"
#include "microphone_device_interface.h"

namespace pxr_base
{
    class MicrophoneDevice:public MicrophoneDeviceInterface
    {
    public:
        MicrophoneDevice(MicrophoneDeviceCallback* callback);
        ~MicrophoneDevice();
        bool Start( bool wireless_mode=true);
        void Stop();
        bool Write(char* buff, int len);
        bool SetVolum(int level);
    private:
        std::wstring origin_defalut_device_name_=  L"";
        std::wstring origin_defalut_device_id_= L"";
        bool GetDefaultDevice(IMMDevice** device, int device_type);
        bool GetDefaultDevice(std::wstring& device_id, std::wstring& device_name, int device_type);
        bool SetDefaultDeviceById(std::wstring& device_id );
        bool SetDefaultDeviceByName(std::wstring& device_name, int device_type);
        bool SetDeviceEnable(std::wstring pwszID);
        bool CheckX64System();
      
        bool InitVirtualAudioDevice();
        bool OpenVirtualAudioCable();
        IMMDevice* FindPicoAudioDevice();
        bool InitAudioClient();
        bool CheckPropVariantValid(IMMDevice* device);
        bool CheckAndSetDeviceVisibility(IMMDevice* device);
        bool TryOpenPolicyConfig(IPolicyConfig*& config);
        void TraceLog(const char* func, int line, const std::string& msg);
        void Reset();
    private:
        template<class T>
        static void AutoReleaseFunc(T* val)
        {
            if (val)
            {
                val->Release();
            }
        }

        template<class T>
        static void AutoMemFreeFunc(T* val)
        {
            if (val)
            {
                CoTaskMemFree(val);
            }
        }

        template<class T>
        class AutoReleaser
        {
        public:
            typedef std::function<void(T*)> Deleter;
            AutoReleaser() = default;
            AutoReleaser(Deleter del) :del_(del) {}
            ~AutoReleaser()
            {
                release();
            }

            operator bool()
            {
                return val_ != nullptr;
            }

            T* operator->() const noexcept
            {
                return val_;
            }
            //WARNING(by Alan Duan): Return reference of T pointer!!!
            T*& get() { return val_; }
            void set(T* val) { val_ = val; }
            void reset() { val_ = nullptr; }
            void release() 
            {
                if (val_)
                {
                    del_(val_);
                    val_ = nullptr;
                }
            }
        private:
            int release_type_ = 0;
            T* val_ = nullptr;
            Deleter del_ = AutoReleaseFunc<T>;
        };
    private:
        HANDLE vac_handle_ = INVALID_HANDLE_VALUE;
        bool start_flag_ = false;
        MicrophoneDeviceCallback* callback_ = nullptr;
        AutoReleaser<IMMDevice> device_;
        AutoReleaser<IAudioClient> audio_client_;
        AutoReleaser<IAudioCaptureClient> capture_client_;
    };
}