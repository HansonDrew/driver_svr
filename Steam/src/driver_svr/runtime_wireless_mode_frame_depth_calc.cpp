#include "runtime_wireless_mode_frame_depth_calc.h"
#include"D3DHelper.h"
#include "DepthResize.h"
#include "RVRLogger.h"
#include "RVRUtils.h"
#include "opencv2/opencv.hpp"
#include"DepthOptFlowFB.h"
extern bool gIsDebug;
 
namespace pxr
{

    constexpr int kPicoNeo3ScreenWidth = 1920;
    constexpr int kPicoNeo3ScreenHeight = 1920;
    constexpr float kPicoNeo3InterpupillaryDistance = 0.06f;
    constexpr int kPicoNeo3Fov = 101;
    constexpr float kCotPicoNeo3Fov = 0.824f;
    constexpr int kPicoNeo3FocalDistance = kPicoNeo3ScreenWidth / 2 * kCotPicoNeo3Fov;


    RuntimeWirelessModeFrameDepthCalc::RuntimeWirelessModeFrameDepthCalc()
        : sgbm_(nullptr)
        , d3d_device_(nullptr)
        , d3d_device_context_(nullptr)
    {
    }

    RuntimeWirelessModeFrameDepthCalc::~RuntimeWirelessModeFrameDepthCalc()
        = default;

    bool RuntimeWirelessModeFrameDepthCalc::Startup(const DepthParam& param)
    {
		
        param_ = param;
        optfb_ = new DepthOptFlowFB();
        //TODO: The disparity params should be counted.
        sgbm_ = cv::StereoSGBM::create();
        // sgbm_->setP1(8 * 3 * 3 * 3);
        // sgbm_->setP2(32 * 3 * 3 * 3);
        // sgbm_->setUniquenessRatio(5);
        {
			int nmDisparities = ((param_.calculate_picture_width / 8) + 15) & -16;//视差搜索范围
            pngChannels =1;//获取左视图通道数
			int winSize = 3;

            sgbm_->setPreFilterCap(32);//预处理滤波器截断值
            sgbm_->setBlockSize(winSize);//SAD窗口大小
            sgbm_->setP1(8 * pngChannels * winSize * winSize);//控制视差平滑度第一参数
            sgbm_->setP2(32 * pngChannels * winSize * winSize);//控制视差平滑度第二参数
            sgbm_->setMinDisparity(0);//最小视差
            int numberOfDisparities = ((param_.calculate_picture_width / 8) + 15) & -16;
            sgbm_->setNumDisparities(numberOfDisparities);//视差搜索范围
            sgbm_->setUniquenessRatio(5);//视差唯一性百分比
            sgbm_->setSpeckleWindowSize(16);//检查视差连通区域变化度的窗口大小
            sgbm_->setSpeckleRange(12);//视差变化阈值
            sgbm_->setDisp12MaxDiff(0);//
            /*////
			sgbm_->setNumDisparities(32);//视差搜索范围
			sgbm_->setUniquenessRatio(15);//视差唯一性百分比
			sgbm_->setSpeckleWindowSize(64);//检查视差连通区域变化度的窗口大小
			sgbm_->setSpeckleRange(20);//视差变化阈值
			sgbm_->setDisp12MaxDiff(1);//左右视差图最大容许差异
            */
			
   //         sgbm_->setMode(cv::StereoSGBM::MODE_SGBM);//采用全尺寸双通道动态编程算法
        }

        IDXGIAdapter1* adapter1 = nullptr;
        if (!D3D11ContextManager::GetHardwareAdapter(adapter1))
        {
            return false;
        }

        if (!D3D11ContextManager::CreateDevice(d3d_device_, d3d_device_context_, adapter1, true, false))
        {
            return false;
        }
        DepthResize::CreateDepthResizeResources(d3d_device_);
 

        //copy_dst render to render_dst(resize )
		// render_dst copy to calculate(same size )
		left_copy_dst_texture_ = D3D11ContextManager::CreateTexture2D(d3d_device_
			, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM
			, param_.copy_dst_picture_width, param_.copy_dst_picture_height
			, 1, false, D3D11_USAGE_DEFAULT, 0);
		right_copy_dst_texture_ = D3D11ContextManager::CreateTexture2D(d3d_device_
			, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM
			, param_.copy_dst_picture_width, param_.copy_dst_picture_height
			, 1, false, D3D11_USAGE_DEFAULT, 0);
       
        DXGI_FORMAT render_to_format = DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_R8_UNORM??
       
        left_compute_texture_ = D3D11ContextManager::CreateTexture2D(d3d_device_
            , 0, render_to_format
            , param_.calculate_picture_width, param_.calculate_picture_height
            , 1, false, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ);

        right_compute_texture_ = D3D11ContextManager::CreateTexture2D(d3d_device_
            , 0, render_to_format
            , param_.calculate_picture_width, param_.calculate_picture_height
            , 1, false, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ);
        left_render_dst_texture_ = D3D11ContextManager::CreateTexture2D(d3d_device_
			, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, render_to_format
			, param_.calculate_picture_width, param_.calculate_picture_height
			, 1, false, D3D11_USAGE_DEFAULT, 0);

		right_render_dst_texture_ = D3D11ContextManager::CreateTexture2D(d3d_device_
			, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, render_to_format
			, param_.calculate_picture_width, param_.calculate_picture_height
			, 1, false, D3D11_USAGE_DEFAULT, 0);
        d3d_device_->CreateRenderTargetView(left_render_dst_texture_, NULL, &resultRenderTarget[0]);
        d3d_device_->CreateRenderTargetView(right_render_dst_texture_, NULL, &resultRenderTarget[1]);

		::ZeroMemory(&resultViewport, sizeof(resultViewport));
		D3DHelper::GetTextureInfo(left_render_dst_texture_, (DXGI_FORMAT*)nullptr, &resultViewport.Width, &resultViewport.Height);
		resultViewport.MinDepth = 0.0f;
		resultViewport.MaxDepth = 1.0f;
		resultViewport.TopLeftX = 0;
		resultViewport.TopLeftY = 0;

		left_texture = new uchar[param.calculate_picture_width * param.calculate_picture_height * pngChannels];
		right_texture = new uchar[param.calculate_picture_width * param.calculate_picture_height * pngChannels];
        left_save_texture= new uchar[param.calculate_picture_width * param.calculate_picture_height * 3];
        right_save_texture = new uchar[param.calculate_picture_width * param.calculate_picture_height * 3];
        return sgbm_ != nullptr && left_compute_texture_ != nullptr && right_compute_texture_ != nullptr;

    }
	static BOOL WriteBitmap888ToFile(const char* filename, int width, int height, unsigned char* bitmapData, int dateSize)
	{
		//填充BITMAPINFOHEADER  
		BITMAPINFOHEADER bitmapInfoHeader;
		memset(&bitmapInfoHeader, 0, sizeof(BITMAPINFOHEADER));
		bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfoHeader.biWidth = width;
		bitmapInfoHeader.biHeight = height;
		bitmapInfoHeader.biPlanes = 1;
		bitmapInfoHeader.biBitCount = 24;
		bitmapInfoHeader.biCompression = BI_RGB;
		bitmapInfoHeader.biSizeImage = dateSize;
		//填充BITMAPFILEHEADER  
		BITMAPFILEHEADER bitmapFileHeader;
		memset(&bitmapFileHeader, 0, sizeof(BITMAPFILEHEADER));
		bitmapFileHeader.bfType = 0x4d42;   //BM固定为这个
		bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		bitmapFileHeader.bfSize = bitmapFileHeader.bfOffBits + dateSize;
		bitmapFileHeader.bfReserved1 = 0;
		bitmapFileHeader.bfReserved2 = 0;
		FILE* filePtr = 0;



		fopen_s(&filePtr, filename, "wb");

		if (NULL == filePtr)

		{

			return FALSE;

		}



		fwrite(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);



		fwrite(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);



		fwrite(bitmapData, bitmapInfoHeader.biSizeImage, 1, filePtr);

		fclose(filePtr);

		return TRUE;

	}
    bool RuntimeWirelessModeFrameDepthCalc::Submit(StereoPictureType type, const void* shared_handle)
    {

        auto* shared_texture = AddSharedTexture(shared_handle);
        auto* const mutex = D3D11ContextManager::AcquireMutex(shared_texture);
        if (d3d_device_context_ != nullptr)
        {
            ID3D11Texture2D* copy_dst_texture = nullptr;

            switch (type)
            {
            case StereoPictureType::kLeft:
                copy_dst_texture = left_copy_dst_texture_;
                break;
            case StereoPictureType::kRight:
                copy_dst_texture = right_copy_dst_texture_;
                break;
            }

            if (copy_dst_texture != nullptr)
            {
                D3D11_BOX box;
                box.left = param_.source_picture_x;
                box.right = param_.source_picture_x + param_.copy_dst_picture_width;
				box.top = param_.source_picture_y;
				box.bottom = param_.source_picture_y + param_.copy_dst_picture_height;
                box.front = 0;
                box.back = 1;
                d3d_device_context_->CopySubresourceRegion(copy_dst_texture,
                    0, 0, 0, 0
                    , shared_texture, 0, &box
                );

            }
			

        }
        D3D11ContextManager::ReleaseMutex(mutex);
        
        return true;
    }

    float RuntimeWirelessModeFrameDepthCalc::Compute() const
    {

        D3D11_MAPPED_SUBRESOURCE left_mapped_subresource;
        ZeroMemory(&left_mapped_subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
        d3d_device_context_->Map(left_compute_texture_, 0, D3D11_MAP_READ, 0, &left_mapped_subresource);

        D3D11_MAPPED_SUBRESOURCE right_mapped_subresource;
        ZeroMemory(&right_mapped_subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
        d3d_device_context_->Map(right_compute_texture_, 0, D3D11_MAP_READ, 0, &right_mapped_subresource);

        cv::Mat output;
        const cv::Mat left_input(param_.calculate_picture_width, param_.calculate_picture_height, CV_8UC4, left_mapped_subresource.pData);
        const cv::Mat right_input(param_.calculate_picture_width, param_.calculate_picture_height, CV_8UC4, right_mapped_subresource.pData);

        sgbm_->compute(left_input, right_input, output);

        d3d_device_context_->Unmap(left_compute_texture_, 0);
        d3d_device_context_->Unmap(right_compute_texture_, 0);

        output.convertTo(output, CV_32F, 1.0 / 16);

        const auto depth_value = FindValidDepthValue(output);
        const auto depth_distance = CalculateDepthDistance(depth_value);
       
        return depth_distance;
    }
	void RuntimeWirelessModeFrameDepthCalc::disp2Depth(cv::Mat disp, cv::Mat& depth)
	{
		depth.create(disp.rows, disp.cols, CV_32F);

		for (int i = 0; i < disp.rows; i++)
		{
			for (int j = 0; j < disp.cols; j++)
			{
				if (!disp.ptr<ushort>(i)[j])//防止除0中断             
                   {
                    depth.ptr<float>(i)[j] = -1000;
                    continue;
                   }
					
				depth.ptr<float>(i)[j] = 1000  / (float)disp.ptr<ushort>(i)[j];
			}
		}
	}
	 
	//-----------------------------------------------------------------------------
	ID3D11ShaderResourceView* RuntimeWirelessModeFrameDepthCalc::AsShaderResource(ID3D11Device* device, ID3D11Texture2D* texture)
		//-----------------------------------------------------------------------------
	{
		ID3D11ShaderResourceView* srv;
		if (srvMap.find(texture) != srvMap.end())
		{
			srv = srvMap[(HANDLE)texture];
		}
		else {
			D3D11_SHADER_RESOURCE_VIEW_DESC sRVDesc;
			::ZeroMemory(&sRVDesc, sizeof(sRVDesc));
			D3DHelper::GetTextureInfo(texture, &sRVDesc.Format, nullptr, nullptr);
			sRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			sRVDesc.Texture2D.MipLevels = 1;
			if (!SUCCEEDED(device->CreateShaderResourceView(texture, &sRVDesc, &srv)))
			{
                OutputDebugStringW(L"Failed to Map SRV!!!!!!!!!!!!!!!!!!!!!!!!!!");
			}
			else {
				srvMap[texture] = srv;
			}
		}

		return srv;
	}
    void  RuntimeWirelessModeFrameDepthCalc::RenderToCalculateTex() 
    {
        DepthResize::DepthResizeFun(resultRenderTarget[0], &resultViewport, AsShaderResource(d3d_device_, left_copy_dst_texture_));
        DepthResize::DepthResizeFun(resultRenderTarget[1], &resultViewport, AsShaderResource(d3d_device_, right_copy_dst_texture_));
        if (gIsDebug)
        {
			if ((0x8000 & GetAsyncKeyState('D')) != 0 && (0x8000 & GetAsyncKeyState('S')) != 0)
			{
				D3DHelper::SaveTextureToFile(d3d_device_, left_copy_dst_texture_, L"left_c.jpg");
				D3DHelper::SaveTextureToFile(d3d_device_, left_render_dst_texture_, L"left_r.jpg");

			}
        }
		
        d3d_device_context_->CopyResource(left_compute_texture_, left_render_dst_texture_);
        d3d_device_context_->CopyResource(right_compute_texture_, right_render_dst_texture_);
       
    }
 //   uchar left_test[128 * 128 * 3];

	
	float RuntimeWirelessModeFrameDepthCalc::Compute2(int& count,float base_div ) const
	{
		
		uint64_t begint = RVR::nowInUs();
		D3D11_MAPPED_SUBRESOURCE left_mapped_subresource;
		ZeroMemory(&left_mapped_subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		d3d_device_context_->Map(left_compute_texture_, 0, D3D11_MAP_READ, 0, &left_mapped_subresource);

		 uchar* resource_data = (uchar*)left_mapped_subresource.pData;
		for (int i = 0; i < param_.calculate_picture_height; i++)
		{
			for (int j = 0; j < param_.calculate_picture_width; j++)
			{
                left_texture[i * param_.calculate_picture_width  + j] = resource_data[i * left_mapped_subresource.RowPitch + j * 4];
                //left_texture[i * param_.calculate_picture_width + j] = resource_data[i * left_mapped_subresource.RowPitch + j];
                /*left_texture[i * param_.calculate_picture_width * 3 + j * 3] = resource_data[i * left_mapped_subresource.RowPitch + j * 4];
				left_texture[i * param_.calculate_picture_width * 3 + j * 3 + 1] = resource_data[i * left_mapped_subresource.RowPitch + j * 4 + 1];
				left_texture[i * param_.calculate_picture_width * 3 + j * 3 + 2] = resource_data[i * left_mapped_subresource.RowPitch + j * 4 + 2];*/
			}
		}
        d3d_device_context_->Unmap(left_compute_texture_, 0);

        
		D3D11_MAPPED_SUBRESOURCE right_mapped_subresource;
		ZeroMemory(&right_mapped_subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		d3d_device_context_->Map(right_compute_texture_, 0, D3D11_MAP_READ, 0, &right_mapped_subresource);
        
		resource_data = (uchar*)right_mapped_subresource.pData;
       
		for (int i = 0; i < param_.calculate_picture_height; i++)
		{
			for (int j = 0; j < param_.calculate_picture_width; j++)
			{
                right_texture[i * param_.calculate_picture_width + j ] = resource_data[i * right_mapped_subresource.RowPitch + j * 4];
                //right_texture[i * param_.calculate_picture_width + j] = resource_data[i * right_mapped_subresource.RowPitch + j ];
				/*right_texture[i * param_.calculate_picture_width * 3 + j * 3] = resource_data[i * right_mapped_subresource.RowPitch + j * 4];
				right_texture[i * param_.calculate_picture_width * 3 + j * 3 + 1] = resource_data[i * right_mapped_subresource.RowPitch + j * 4 + 1];
				right_texture[i * param_.calculate_picture_width * 3 + j * 3 + 2] = resource_data[i * right_mapped_subresource.RowPitch + j * 4 + 2];*/
			}
		}
       
        d3d_device_context_->Unmap(right_compute_texture_, 0);
     
        cv::Mat output;
		const cv::Mat left_input(param_.calculate_picture_width, param_.calculate_picture_height, CV_8U,left_texture);
		const cv::Mat right_input(param_.calculate_picture_width, param_.calculate_picture_height, CV_8U, right_texture);
		uint64_t end = RVR::nowInUs();
		WCHAR strBuffer[256];
		swprintf_s(strBuffer, 256, L"deput picture copy cost = %f\n", (end- begint)/1000.f);
		//OutputDebugStringW(strBuffer);

		int type=left_input.type();
        if (gIsDebug)
        {
			if ((0x8000 & GetAsyncKeyState('D')) != 0 && (0x8000 & GetAsyncKeyState('S')) != 0)
			{

				 
				D3DHelper::SaveTextureToFile(d3d_device_, left_compute_texture_, L"left_ca.jpg");

				for (int i = 0; i < param_.calculate_picture_height; i++)
				{
					for (int j = 0; j < param_.calculate_picture_width; j++)
					{
						left_save_texture[i * param_.calculate_picture_width * 3 + j * 3] = 0;
						left_save_texture[i * param_.calculate_picture_width * 3 + j * 3 + 1] = 0;
						left_save_texture[i * param_.calculate_picture_width * 3 + j * 3 + 2] = left_texture[(param_.calculate_picture_height - 1 - i) * param_.calculate_picture_width + j];

						right_save_texture[i * param_.calculate_picture_width * 3 + j * 3] = 0;
						right_save_texture[i * param_.calculate_picture_width * 3 + j * 3 + 1] = 0;
						right_save_texture[i * param_.calculate_picture_width * 3 + j * 3 + 2] = right_texture[(param_.calculate_picture_height - 1 - i) * param_.calculate_picture_width + j];

					}
				}
				WriteBitmap888ToFile("D://left_1fff.bmp", param_.calculate_picture_width, param_.calculate_picture_height, (unsigned char*)left_save_texture, param_.calculate_picture_width * param_.calculate_picture_height * 3);
				WriteBitmap888ToFile("D://right_1fff.bmp", param_.calculate_picture_width, param_.calculate_picture_height, (unsigned char*)right_save_texture, param_.calculate_picture_width * param_.calculate_picture_height * 3);

				cv::imwrite("D://leftcvgray.bmp", left_input);
				cv::imwrite("D://rightcvgray.bmp", right_input);
			}

        }
      
       // Mat fbflow;
        //optfb_->compute(left_input, right_input, fbflow);

		//count = 0;
        //float depth = optfb_->GetDepth(fbflow,count,2);
        //return depth;
		//return 1;
      
	    sgbm_->compute(left_input, right_input, output);
		cv::Mat depth;
		disp2Depth(output, depth);

		float all_depth = 0;
		int depth_count = 0;
		float maxdepth = 0;
		float mindepth =  1000;
		int max_count = 0;
		int min_count = 0;

		float  half_depth = 0;
		int  half_depth_count = 0;
		int half_count_block = 4;
		for (int i = 0; i < depth.rows ; i++)
		{
			for (int j = 0; j < depth.cols; j++)
			{
				if (depth.ptr<float>(i)[j] > 0&& depth.ptr<float>(i)[j]<  1000)
				{
					all_depth = all_depth + (float)depth.ptr<float>(i)[j] ;
					depth_count++;
					if ((float)depth.ptr<float>(i)[j] > maxdepth)
					{

						maxdepth = (float)depth.ptr<float>(i)[j] ;
						max_count = 1;
					}
					else if (abs((float)depth.ptr<float>(i)[j]-maxdepth)<1)
					{
						max_count++;
					}
					if ((float)depth.ptr<float>(i)[j]< mindepth)
					{

						mindepth = (float)depth.ptr<float>(i)[j];
						min_count = 1;
					}
					else if (abs((float)depth.ptr<float>(i)[j] - mindepth) < 1)
					{
						min_count++;
					}

					if ((i > (depth.rows / 2 - half_count_block)) && (i < (depth.rows / 2 + half_count_block - 1))&&
						(j > (depth.cols / 2 - half_count_block)) && (j < (depth.cols / 2 + half_count_block - 1)))
					{
						half_depth = half_depth + (float)depth.ptr<float>(i)[j];
						half_depth_count++;
					}
				}
				else
				{
					WCHAR strBuffer[256];
					swprintf_s(strBuffer, 256, L"track error depth=%f\n", depth.ptr<float>(i)[j]);
					//OutputDebugStringW(strBuffer);
					if ((i > (depth.rows / 2 - half_count_block)) && (i < (depth.rows / 2 + half_count_block - 1)) &&
						(j > (depth.cols / 2 - half_count_block)) && (j < (depth.cols / 2 + half_count_block - 1)))
					{
						half_depth = half_depth + (float)depth.ptr<float>(i)[j];
						half_depth_count++;
					}
				}

			}

		}
		half_depth = half_depth / half_depth_count/9.5f;
		if ((0x8000 & GetAsyncKeyState('D')) != 0 && (0x8000 & GetAsyncKeyState('F')) != 0)
		{
			WCHAR strBuffer[256];
			swprintf_s(strBuffer, 256, L"half_depth=%f  max depth = %f  min depth= %f count =%d maxcount=%d mincount=%d\n",half_depth, maxdepth,mindepth, depth_count,max_count,min_count);
			OutputDebugStringW(strBuffer);

		}
		if (abs(maxdepth-mindepth)<1&&depth_count<param_.calculate_picture_height*param_.calculate_picture_width/3)
		{//没算出视差
			return 18;

		}

		if (  depth_count < param_.calculate_picture_height * param_.calculate_picture_width / 3)
		{

			return (maxdepth+mindepth)/7.00f;

		}
		else if ((param_.calculate_picture_height * param_.calculate_picture_width / 3 <depth_count)&& (depth_count < param_.calculate_picture_height * param_.calculate_picture_width / 2))
		{
			return  all_depth / depth_count / 5.0f;
		}

		//return  all_depth / depth_count/4.800f ;//256
		return  all_depth / depth_count / base_div;
      
		
	}

    void  RuntimeWirelessModeFrameDepthCalc::ResizeRenderToDst() 
    {

    }
    bool RuntimeWirelessModeFrameDepthCalc::Shutdown()
    {
        ClearSharedTextures();
        D3D11ContextManager::DeleteTexture2D(left_compute_texture_);
        D3D11ContextManager::DeleteTexture2D(right_compute_texture_);
        sgbm_.release();

        return true;
    }
    ID3D11Texture2D* RuntimeWirelessModeFrameDepthCalc::GetRenderDstTexture(StereoPictureType type)
    {
        ID3D11Texture2D* render_dst_texture=NULL;
		switch (type)
		{
		case StereoPictureType::kLeft:
            render_dst_texture =left_render_dst_texture_;
			break;
		case StereoPictureType::kRight:
            render_dst_texture = right_render_dst_texture_;
			break;
		}
        return render_dst_texture;
    }
    ID3D11Texture2D* RuntimeWirelessModeFrameDepthCalc::AddSharedTexture(const void* handle)
    {
        if (handle == nullptr)
        {
            return nullptr;
        }

        ID3D11Texture2D* texture = nullptr;
        if (shared_textures_.find(handle) != shared_textures_.end())
        {
            texture = shared_textures_[handle];
        }
        else
        {
            if (!D3D11ContextManager::OpenSharedTexture(d3d_device_, handle, texture))
            {
                texture = nullptr;
            }
            shared_textures_[handle] = texture;
        }

        return texture;
    }

    void RuntimeWirelessModeFrameDepthCalc::ClearSharedTextures()
    {
        for (auto t : shared_textures_)
        {
            D3D11ContextManager::CloseSharedTexture(t.second);
        }
    }

    float RuntimeWirelessModeFrameDepthCalc::FindValidDepthValue(const cv::Mat& mat)
    {
        //a calculated depth cv::Mat might be:
        // 0, 0, 0, 0, 0, 0
        // 0, 0, 0, 8, 5, 0
        // 0, 0, 0, 0, 6, 0
        // 0, 0, 0, 0, 0, 0
        //
        //We need to find the first valid( bigger than 0 from left-top) value,
        //From this sample, we got 8.


        auto value_found = 0.f;
        const auto channels = mat.channels();
        const auto rows = mat.rows;
        const auto cols = mat.cols * channels;

        for (auto i = 0; i < rows; ++i)
        {
            const auto* p = mat.ptr<float>(i);
            for (auto j = 0; j < cols; ++j)
            {
                if (p[j] > 0)
                {
                    value_found = p[j];
                    break;
                }
            }

            if (value_found > 0)
            {
                break;
            }
        }

        return value_found;
    }

    float RuntimeWirelessModeFrameDepthCalc::CalculateDepthDistance(const float depth_value)
    {
        if (depth_value <= 0.f)
        {
            return 0.f;
        }

        return kPicoNeo3FocalDistance * kPicoNeo3InterpupillaryDistance / depth_value;
    }
}
