#include "runtime_wireless_mode_frame_depth_calc.h"
#include"D3DHelper.h"
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

        //TODO: The disparity params should be counted.
        sgbm_ = cv::StereoSGBM::create();
        // sgbm_->setP1(8 * 3 * 3 * 3);
        // sgbm_->setP2(32 * 3 * 3 * 3);
        // sgbm_->setUniquenessRatio(5);
        {
			int nmDisparities = ((param_.calculate_picture_width / 8) + 15) & -16;//视差搜索范围
			int pngChannels =4;//获取左视图通道数
			int winSize = 9;

            sgbm_->setPreFilterCap(8);//预处理滤波器截断值
            sgbm_->setBlockSize(winSize);//SAD窗口大小
            sgbm_->setP1(8 * pngChannels * winSize * winSize);//控制视差平滑度第一参数
            sgbm_->setP2(32 * pngChannels * winSize * winSize);//控制视差平滑度第二参数
            sgbm_->setMinDisparity(0);//最小视差
           
            //sgbm_->setNumDisparities(64);//视差搜索范围
            //sgbm_->setUniquenessRatio(8);//视差唯一性百分比
            //sgbm_->setSpeckleWindowSize(100);//检查视差连通区域变化度的窗口大小
            //sgbm_->setSpeckleRange(32);//视差变化阈值
            //sgbm_->setDisp12MaxDiff(0);//

			sgbm_->setNumDisparities(128);//视差搜索范围
			sgbm_->setUniquenessRatio(15);//视差唯一性百分比
			sgbm_->setSpeckleWindowSize(100);//检查视差连通区域变化度的窗口大小
			sgbm_->setSpeckleRange(128);//视差变化阈值
			sgbm_->setDisp12MaxDiff(1);//左右视差图最大容许差异
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

        left_compute_texture_ = D3D11ContextManager::CreateTexture2D(d3d_device_
            , 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
            , param_.calculate_picture_width, param_.calculate_picture_height
            , 1, false, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ);

        right_compute_texture_ = D3D11ContextManager::CreateTexture2D(d3d_device_
            , 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
            , param_.calculate_picture_width, param_.calculate_picture_height
            , 1, false, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ);

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
            ID3D11Texture2D* compute_texture = nullptr;

            switch (type)
            {
            case StereoPictureType::kLeft:
                compute_texture = left_compute_texture_;
                break;
            case StereoPictureType::kRight:
                compute_texture = right_compute_texture_;
                break;
            }

            if (compute_texture != nullptr)
            {
                D3D11_BOX box;
                box.left = param_.source_picture_x;
                box.right = param_.source_picture_x + param_.calculate_picture_width;
				box.top = param_.source_picture_y;
				box.bottom = param_.source_picture_y + param_.calculate_picture_height;
                box.front = 0;
                box.back = 1;
                d3d_device_context_->CopySubresourceRegion(compute_texture,
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
	void RuntimeWirelessModeFrameDepthCalc::disp2Depth(cv::Mat dispMap, cv::Mat& depthMap)
	{
		depthMap.create(dispMap.rows, dispMap.cols, CV_16U);
		dispMap.convertTo(dispMap, CV_8U,16);
		int type = dispMap.type();

        ushort fx = 22;

        ushort baseline = 65; //基线距离65mm

		if (type == CV_8U)
		{ 
			int height = dispMap.rows;
			int width = dispMap.cols;

			uchar* dispData = (uchar*)dispMap.data;
			ushort* depthData = (ushort*)depthMap.data;
   
			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width; j++)
				{
					int id = i * width + j;
					if (!dispData[id])  continue;  //防止0除
					depthData[id] = ushort(fx * baseline / (dispData[id]));
				}
			}
		}
		 
	}
	void disp2Depth2(cv::Mat disp, cv::Mat& depth )
	{
		depth.create(disp.rows, disp.cols, CV_32F);
	 
		for (int i = 0; i < disp.rows; i++)
		{
			for (int j = 0; j < disp.cols; j++)
			{
				if (!disp.ptr<ushort>(i)[j])//防止除0中断
					continue;
                depth.ptr<float>(i)[j] =  1000*22.0f * 65.0f /(float) disp.ptr<ushort>(i)[j];
			}
		}
		
	}
	 
	float RuntimeWirelessModeFrameDepthCalc::Compute2() const
	{
		
		D3D11_MAPPED_SUBRESOURCE left_mapped_subresource;
		ZeroMemory(&left_mapped_subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		d3d_device_context_->Map(left_compute_texture_, 0, D3D11_MAP_READ, 0, &left_mapped_subresource);

		char* resource_data = (char*)left_mapped_subresource.pData;
        unsigned char* pData = reinterpret_cast<unsigned char*>(left_mapped_subresource.pData);
		for (int i = 0; i < param_.calculate_picture_height; i++)
		{
			memcpy_s(&left_texture[i * param_.calculate_picture_width * 4], param_.calculate_picture_width * 4, pData, param_.calculate_picture_width * 4);
			pData += left_mapped_subresource.RowPitch;
		}
        d3d_device_context_->Unmap(left_compute_texture_, 0);


		D3D11_MAPPED_SUBRESOURCE right_mapped_subresource;
		ZeroMemory(&right_mapped_subresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		d3d_device_context_->Map(right_compute_texture_, 0, D3D11_MAP_READ, 0, &right_mapped_subresource);
        
		resource_data = (char*)right_mapped_subresource.pData;
		pData = reinterpret_cast<unsigned char*>(right_mapped_subresource.pData);
		for (int i = 0; i < param_.calculate_picture_height; i++)
		{
			memcpy_s(&right_texture[i * param_.calculate_picture_width * 4], param_.calculate_picture_width * 4, pData, param_.calculate_picture_width * 4);
			pData += right_mapped_subresource.RowPitch;
		}
		
        d3d_device_context_->Unmap(right_compute_texture_, 0);
        if ((0x8000 & GetAsyncKeyState('D')) != 0 && (0x8000 & GetAsyncKeyState('S')) != 0)
        {
            WriteBitmap888ToFile("D://left_1fff.bmp", param_.calculate_picture_width, param_.calculate_picture_height, (unsigned char*)left_texture, param_.calculate_picture_width * param_.calculate_picture_height * 3);

        }
        cv::Mat output;
		const cv::Mat left_input(param_.calculate_picture_width, param_.calculate_picture_height, CV_8UC4,left_texture);
		const cv::Mat right_input(param_.calculate_picture_width, param_.calculate_picture_height, CV_8UC4, right_texture);
	
		

		sgbm_->compute(left_input, right_input, output);
        cv::Mat depth;
        disp2Depth2(output, depth);
       
        float all_depth = 0;
        int depth_count = 0;
        float maxdepth = 0;
        float mindepth = 22.0f * 65.0f * 1000;
        int max_count = 0;
        int min_count = 0;
        for (int i = 0; i < depth.rows ; i++)
        {
            for (int j = 0; j < depth.cols; j++)
            {           
                if (depth.ptr<float>(i)[j] > 0&& depth.ptr<float>(i)[j]< 22.0f * 65.0f*1000)
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
                }
               
            }

        }
		if ((0x8000 & GetAsyncKeyState('D')) != 0 && (0x8000 & GetAsyncKeyState('F')) != 0)
		{
			WCHAR strBuffer[256];
			swprintf_s(strBuffer, 256, L"max depth = %f  min depth= %f count =%d maxcount=%d mincount=%d\n", maxdepth,mindepth, depth_count,max_count,min_count);
			OutputDebugStringW(strBuffer);
          
		}
        if (abs(maxdepth-mindepth)<1&&depth_count<param_.calculate_picture_height*param_.calculate_picture_width/3)
        {//没算出视差
            return 10;

        }

		if (  depth_count < param_.calculate_picture_height * param_.calculate_picture_width / 3)
		{

			return (maxdepth+mindepth)/200.00f;

		}
		return  all_depth / depth_count/1000.f ;
	}


    bool RuntimeWirelessModeFrameDepthCalc::Shutdown()
    {
        ClearSharedTextures();
        D3D11ContextManager::DeleteTexture2D(left_compute_texture_);
        D3D11ContextManager::DeleteTexture2D(right_compute_texture_);
        sgbm_.release();

        return true;
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
