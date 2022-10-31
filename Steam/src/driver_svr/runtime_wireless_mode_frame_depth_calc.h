#pragma once

#include "opencv2/calib3d/calib3d.hpp"

#include "d3d11_context_manager.h"
#include <unordered_map>
#include"DepthOptFlowFB.h"
namespace pxr
{
    class RuntimeWirelessModeFrameDepthCalc
    {
    /// <summary>
    /// // 1 copy sub  from sharedtexture to dsttexture  size :1024  1312 
    ///    2 dsttexture render to  transformtexture(resize »Ò¶È)
    /// copy   transformtexture  to  calculate picture (cpu_read can't render to)
    ///    
    /// </summary>
    public:

        struct DepthParam
        {
            int calculate_picture_width = 0;
            int calculate_picture_height = 0;
            int copy_dst_picture_width = 0;
            int	copy_dst_picture_height = 0;
            int source_picture_x = 0;
            int source_picture_y = 0;
          
        };
		
        enum class StereoPictureType
        {
            kLeft,
            kRight
        };
		DepthOptFlowFB *optfb_;

	
        int pngChannels;
        bool Startup(const DepthParam& param);
        bool Submit(StereoPictureType type, const void* shared_handle);
        void RenderToCalculateTex();
        float Compute() const;
        std::unordered_map<HANDLE, ID3D11ShaderResourceView*> srvMap;
        ID3D11ShaderResourceView* AsShaderResource(ID3D11Device* device, ID3D11Texture2D* texture);
        float Compute2(int &count, float base_div = 9.5) const;
        void ResizeRenderToDst();
        bool Shutdown();
        ID3D11Texture2D* GetRenderDstTexture(StereoPictureType type);
        ID3D11RenderTargetView* resultRenderTarget[2];
        D3D11_VIEWPORT resultViewport;
        RuntimeWirelessModeFrameDepthCalc();
        ~RuntimeWirelessModeFrameDepthCalc();
		uchar* left_texture = NULL;
		uchar* right_texture = NULL;
		uchar* left_save_texture = NULL;
		uchar* right_save_texture = NULL;
        ID3D11Texture2D* AddSharedTexture(const void* handle);
    private:
        void ClearSharedTextures();
        static float FindValidDepthValue(const cv::Mat& mat);
        static float CalculateDepthDistance(const float depth_value);
        static void disp2Depth(cv::Mat disp, cv::Mat& depth);

    private:
        DepthParam param_;
        cv::Ptr<cv::StereoSGBM> sgbm_;
        ID3D11Device* d3d_device_;
        ID3D11DeviceContext* d3d_device_context_;
		ID3D11Texture2D* left_copy_dst_texture_;
		ID3D11Texture2D* right_copy_dst_texture_;
        ID3D11Texture2D* left_compute_texture_;
        ID3D11Texture2D* right_compute_texture_;
		ID3D11Texture2D* left_render_dst_texture_;
		ID3D11Texture2D* right_render_dst_texture_;
        std::unordered_map<const void*, ID3D11Texture2D*> shared_textures_;
    };
}

