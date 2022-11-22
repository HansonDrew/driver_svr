#ifndef NO_USBBULK
#include "UsbBulkModule.h"

#include "microphone_device.h"
#include <string> 
#include "RVRLogger.h"
#include "RVRPluginDefinitions.h"
#include "SensorManger.h"
#include "config_reader.h"
#include "SeiHelp.h"
#include "driver_define.h"
#include "SensorManger.h"
#include "driver_pico.h"
#include "TimeTool.h"
#include "pxr_base/logger.h"
#include "pxr_base/mutex_lock.h"
#include "base_hmd.h"
#include "sensor_passer.h"
#include "filetool.h"
#include "stringtool.h"
#include<fstream>
#include <iostream>
#include <functional>

using namespace pxr_base;
using namespace RVR;

extern ConfigReader gConfigReader;
extern PicoVRDriver g_svrDriver;

namespace pico_streaming
{
    std::string ToString(const RVR::RVRVector3& val)
    {
        std::string str;
        str += "(x,y,z):( ";
        str += std::to_string(val.x) + " , ";
        str += std::to_string(val.y) + " , ";
        str += std::to_string(val.z) + "  ) ";
        return str;
    }

    std::string ToString(const RVR::RVRQuaternion& val)
    {
        std::stringstream ss;
        ss << "(w,x,y,z): ( ";
        ss << val.w << " , ";
        ss << val.x << " , ";
        ss << val.y << " , ";
        ss << val.z << "  ) ";
        return ss.str();
    }

    std::string ToString(const RVR::RVRControllerData& val)
    {
        std::stringstream ss;
        ss << "rotation" << ToString(val.rotation) << " , ";
        ss << "position" << ToString(val.position) << " , ";
        ss << "velocity" << ToString(val.vecVelocity);
        return ss.str();
    }

    std::string ToString(const RVR::RVRPoseHmdData& val)
    {
        std::stringstream ss;
        ss << "rotation" << ToString(val.rotation) << " , ";
        ss << "position" << ToString(val.position) << " )";
        return ss.str();
    }

    UsbBulkModule* UsbBulkModule::GetInstance()
    {
        static UsbBulkModule* sUsbBulkModuleInstance = nullptr;
        static std::mutex sMutex;
        if (sUsbBulkModuleInstance == NULL)
        {
            PXR_LOCK_MTX(lk, sMutex);
            if (sUsbBulkModuleInstance == NULL)
            {
                sUsbBulkModuleInstance = new UsbBulkModule();
            }
        }
        return sUsbBulkModuleInstance;
    }

    UsbBulkModule::UsbBulkModule()
        :engine_startup_(false), connected_(false)
    {   
        LogWriter::SetLogFilePrefix("pico_driver");
        if (gConfigReader.GetUsbLog() == 1)
        {
			LogWriter::SetOuputLogLevel(LogLevel::kLogLevelDebug);
            PXR_DEBUG_THIS("set log debug level");
        }
        else
        {
			LogWriter::SetOuputLogLevel(LogLevel::kLogLevelInfo);
			PXR_INFO_THIS("set log info level");
        }
        
        video_send_buffer_.channel() = 0;
        audio_send_buffer_.channel() = 1;
        msg_send_buffer_.channel() = 2;
        DriverLog("usb mode: init success");
    }

    UsbBulkModule::~UsbBulkModule()
    {
        Shutdown();
        DriverLog("usb mode: deinit");
    }

    void UsbBulkModule::Start()
    {
        PXR_CHECK_RET(engine_startup_, "has been started", );
        ///PXR_INFO_THIS("start usb mode");
        engine_startup_ = true;
        server_ = TransportServer::Create(TransportType::kTcp);
        TransportServer::Config config;
        config.tcp_host.SetHost_("0.0.0.0");
        config.tcp_host.SetPort_(29000);
        config.callback = this;
        PXR_INFO_THIS("callback= " << PXR_LOG_PTR(config.callback));
        if (!server_->Start(config))
        {
            PXR_ERROR_THIS("start tcp server failed");
            return;
        }
        check_sensor_thread_.reset(new std::thread(
            std::bind(&UsbBulkModule::CheckSensorReceiveThread, this)));
    }

    void  UsbBulkModule::Shutdown()
    {
        engine_startup_ = false;
        connected_ = false;
        if (session_)
            session_.reset();

        if (server_)
            server_.reset();

        if (check_sensor_thread_ && check_sensor_thread_->joinable())
        {
            check_sensor_thread_->join();
        }
        PXR_INFO_THIS("completed!");
    }

    bool UsbBulkModule::SendVideoFrame(uint8_t* data, int length, int picture_type,
        int eye_inedex, int64_t gen_timestamp, EncodeOutWithInfo encode_info)
    {
        PXR_CHECK_RET(!CheckUsbMode(), "check usb mode failed", false);
        PXR_CHECK_RET(!engine_startup_, "usb mode not started", false);
        PXR_CHECK_RET(!connected_, "not connected", false);

        PXR_LOCK_REC(lk, video_mutex_);
        auto control_mode = gConfigReader.BigPicture() == 1 \
            ? PictureControlType::KBigPicture : PictureControlType::KLittlePicture;
        RVR::RVRPoseHmdData sensor;
        SensorManger::GetInstance()->SetIndex(encode_info.index);
        SensorManger::GetInstance()->GetSensor(control_mode, eye_inedex, sensor);

        PXR_DEBUG_THIS("eye= " << eye_inedex
            << ", len= " << length
            << ", time= " << gen_timestamp
            << ", valid= " << sensor.valid
            << " , poseTime= " << sensor.poseTimeStamp
            << ", poserecvTime= " << sensor.poseRecvTime
            << ", encode_info index= " << encode_info.index
            << ", rotation" << ToString(sensor.rotation)
            << ", position" << ToString(sensor.position));
        PXR_CHECK_RET(!sensor.valid, "invalid sensor", false);

        WireLessType::AddPoseData add_data;
        WireLessType::AddQuaternion add_quaternion;
        WireLessType::AddVector3 add_vector3;
        add_quaternion.w = -sensor.rotation.w;
        add_quaternion.x = -sensor.rotation.x;
        add_quaternion.y = -sensor.rotation.y;
        add_quaternion.z = -sensor.rotation.z;
        add_data.rotation = add_quaternion;

        add_vector3.x = sensor.position.x;
        add_vector3.y = sensor.position.y;
        add_vector3.z = sensor.position.z;
        add_data.position = add_vector3;
        add_data.hmdPoseTimeTs = sensor.hmdTimeStamp;//0825
        add_data.poseTimeStamp = sensor.poseTimeStamp;
        add_data.predictedTimeMs = 0;

        add_data.gameRenderCost = sensor.endGameRenderStamp - sensor.beginGameRenderStamp;
        add_data.encodeCost = encode_info.encodEnd - sensor.beginEncodeStamp;
        add_data.autoRateFlag = encode_info.autoRateFlag;
        add_data.encodeRate = encode_info.bitRate;

        auto& buffer = video_send_buffer_.data();
        int sendLen = length + 6 + sizeof(WireLessType::AddPoseData);
        buffer.SetSize_(sendLen);
        auto mSendBuf = buffer.Data_();
        uint8_t  pload = encode_info.PloadTpye;//gRtpPloadTpye;//hevc
        memcpy(mSendBuf, &length, sizeof(int));
        memcpy(mSendBuf + 4, &pload, sizeof(uint8_t));
        memcpy(mSendBuf + 5, &eye_inedex, sizeof(uint8_t));
        memcpy(mSendBuf + 6, &add_data, sizeof(WireLessType::AddPoseData));
        memcpy(mSendBuf + 6 + sizeof(WireLessType::AddPoseData), data, length);

        //TestFuncSaveVideoFrame(data, length, picture_type, eye_inedex, gen_timestamp);

        if (!session_->SendData(video_send_buffer_))
        {
            PXR_ERROR_THIS("send data failed");
            return false;
        }
        return true;
    }

    bool UsbBulkModule::SendAudioFrame(uint8_t* data, int len, int samples)
    {
        PXR_CHECK_RET(!CheckUsbMode(), "check usb mode failed", false);
        PXR_CHECK_RET(!engine_startup_, "usb mode not started", false);
        PXR_CHECK_RET(!connected_, "not connected", false);
        PXR_CHECK_RET(!session_, "invalid session", false);

        int audio_len = len + 8;
        auto& buffer = audio_send_buffer_.data();
        buffer.SetSize_(audio_len);
        uint8_t* ptr = buffer.Data_();
        memcpy(ptr, &len, sizeof(int));
        memcpy(ptr + 4, &samples, sizeof(int));
        memcpy(ptr + 8, data, len);

        if (!session_->SendData(audio_send_buffer_))
        {
            PXR_ERROR_THIS("send data failed");
            return false;
        }
        return true;
    }

    bool UsbBulkModule::SendMsg(char* buf, int len, int channel)
    {
        PXR_CHECK_RET(!CheckUsbMode(), "check usb mode failed", false);
        PXR_CHECK_RET(!engine_startup_, "usb mode not started", false);
        PXR_CHECK_RET(!connected_, "not connected", false);
        PXR_CHECK_RET(!session_, "invalid session", false);

        PXR_CHECK_RET(!buf || !len, "invalid data or length", false);

        auto& buffer = msg_send_buffer_.data();
        buffer.SetSize_(len);
        memcpy(buffer.Data_(), buf, len);

        PXR_DEBUG_THIS("send message len= " << len);
        msg_send_buffer_.channel() = channel;
        if (!session_->SendData(msg_send_buffer_))
        {
            PXR_ERROR_THIS("send data failed");
            return false;
        }
        return true;
    }

    void UsbBulkModule::NotifyClientConnected(TransportSession::Ptr_t session)
    {
        PXR_CHECK_RET(!CheckUsbMode(), "check usb mode failed", );
        PXR_CHECK_RET(!engine_startup_, "usb not started", );
        PXR_CHECK_RET(!session, "invalid session", );

        if (connected_)
        {
            PXR_WARN_THIS("driver has been connected, refuse new connection");
            if (!session_)
            {
                PXR_ERROR_THIS("invalid session");
                connected_ = false;
            }
            return;
        }
        else if (session_)
        {
            //Todo(by Alan Duan): We should reset the session in NotifySessionStopped,
            //					  fix the crash bug in pxr_base.
            session_.reset();
        }

        PXR_INFO_THIS("connect to assistant");
        TransportSession::Config config;
        config.callback = this;
        config.enable_channel = true;
        config.enable_packet = true;
        if (!session->Start(config))
        {
            PXR_ERROR_THIS("start session failed");
            return;
        }
        session_ = session;
        connected_ = true;
        DriverLog("usb mode: connect to assistant");
    }

    void UsbBulkModule::NotifySessionStopped(int res)
    {
        PXR_INFO_THIS("res= " << res);
        DriverLog("usb mode: assistant disconnected");
        connected_ = false;

		if (gConfigReader.GetRtcOrBulkMode_() == 2)
		{
			PXR_INFO_THIS(" hmd & controller disconnected");

			RVR::RVRPoseHmdData hmd_pose = { 0 };
			memmove(&hmd_pose, &hmd_pose_, sizeof(RVR::RVRPoseHmdData));
			hmd_pose.valid = FALSE;
			g_svrDriver.AddHmdPose(&hmd_pose);

			RVR::RVRControllerData controller_data = { 0 };
			memmove(&controller_data, &left_pose_, sizeof(RVR::RVRPoseHmdData));
			controller_data.connectionState = RVR::RVRControllerConnectionState::kDisconnected;
			g_svrDriver.AddControllerPose(0, &controller_data);

			controller_data = { 0 };
			memmove(&controller_data, &right_pose_, sizeof(RVR::RVRPoseHmdData));
			controller_data.connectionState = RVR::RVRControllerConnectionState::kDisconnected;
			g_svrDriver.AddControllerPose(1, &controller_data);
		}

    }

    void UsbBulkModule::NotifyServerStopped(int res)
    {
        PXR_ERROR_THIS("tcp server stopped, res= " << res);
        connected_ = false;
    }

    void UsbBulkModule::NotifyReceiveData(const TransportData& data)
    {
        PXR_CHECK_RET(!CheckUsbMode(), "check usb mode failed", );

        switch (data.channel())
        {
        case 0:
        {
            HandleSensorData(data);
            break;
        }
        case 1:
        {
            HandleAudioData(data);
            break;
        }
        case 4: 
        {
            HandleMsgData(data);
            break;
        }
        default:
            PXR_ERROR_THIS("unexpected channel= " << data.channel());
            break;
        }
    }

    void UsbBulkModule::HandleAudioData(const TransportData& data)
    {
        auto& buffer = data.data();
        //audio_recv_count_++;
        //if (audio_recv_count_ % 2)
        //{
        //    memcpy(audio_recv_buffer_, buffer.Data_(), buffer.Size_());
        //}
        //else
        //{
        //    memcpy(audio_recv_buffer_ + AUDIOLEN / 2, buffer.Data_(), buffer.Size_());
        //    audio_recv_count_ = 0;
        //    //TestFuncSaveAudioFrame((uint8_t *)audio_recv_buffer_, AUDIOLEN);
        //    g_svrDriver.GetStreamingHmdDriver()->SaveMicDate(audio_recv_buffer_, AUDIOLEN);
        //}
        memcpy(audio_recv_buffer_, buffer.Data_(), buffer.Size_());
        g_svrDriver.GetStreamingHmdDriver()->SaveMicDate(audio_recv_buffer_, AUDIOLEN);
    }

    void UsbBulkModule::HandleSensorData(const TransportData& data)
    {
        static const size_t kSensorDataDefaultSize = 443;
        static const size_t kSensorDataWithachedSize = 459;
        const auto& buffer = data.data();
		bool msg_len_err = false;
		if ((buffer.Size_() != kSensorDataDefaultSize) && (buffer.Size_() != kSensorDataWithachedSize))
		{
			msg_len_err = true;
		}
		PXR_CHECK_RET(msg_len_err, "check sensor size failed", );

        if (buffer[0] != 0x13 || buffer[1] != 0x05)
        {
            PXR_ERROR_THIS("check sensor header failed");
            return;
        }

        if (buffer[buffer.Size_() - 1] != 0xEE)
        {
            PXR_ERROR_THIS("check sensor end failed");
            return;
        }

        static const size_t kSensorMsgHeaderLen = 2;
        static const size_t kSensorMsgHmdPoseLen = sizeof(WireLessType::TransPoseData);
        static const size_t kSensorMsgControllerPoseLen = sizeof(WireLessType::TransControllerData);

        uint8_t* ptr = const_cast<uint8_t*>(buffer.Data_());
        ptr += kSensorMsgHeaderLen;

        RVR::RVRPoseHmdData hmd_pose;
        SensorManger::GetInstance()->ConvertToHmdSensor(ptr, kSensorMsgHmdPoseLen, hmd_pose);
        ptr += kSensorMsgHmdPoseLen;
        last_hmd_recv_timestamp_ = GetTimestampUs();

        RVR::RVRControllerData left_pose;
        auto len_l_controller = sizeof(WireLessType::TransPoseData);
        SensorManger::GetInstance()->ConvertToControllerSensor(ptr,
            kSensorMsgControllerPoseLen, 0, left_pose);
        ptr += kSensorMsgControllerPoseLen;
        last_left_controller_recv_timestamp_ = GetTimestampUs();

        RVR::RVRControllerData right_pose;
        SensorManger::GetInstance()->ConvertToControllerSensor(ptr,
            kSensorMsgControllerPoseLen, 1, right_pose);
        ptr += kSensorMsgControllerPoseLen;
        last_right_controller_recv_timestamp_ = GetTimestampUs();
		if (buffer.Size_() == kSensorDataWithachedSize)
		{
			memmove(&hmd_pose.net_cost, ptr, sizeof(int));
		}
		else
		{
			hmd_pose.net_cost = 0;
		}
        
        SensorPasser::GetInstance()->SetAllSensor(hmd_pose, left_pose, right_pose);
		


        if (g_svrDriver.GetStreamingHmdDriver()->GetIsAdded_() == false)
        {
            PXR_DEBUG_THIS("GetIsAdded_ ==false");
            g_svrDriver.AddHmdPose(&hmd_pose);
            g_svrDriver.AddControllerPose(0, &left_pose);
            g_svrDriver.AddControllerPose(1, &right_pose);
            hmd_pose_ = hmd_pose;
            left_pose_ = left_pose;
            right_pose_ = right_pose;
        }

        PXR_DEBUG_THIS("hmd pose= " << ToString(hmd_pose));
        PXR_DEBUG_THIS("left pose= " << ToString(left_pose));
        PXR_DEBUG_THIS("right pose= " << ToString(right_pose));
    }
    void UsbBulkModule::HandleMsgData(const TransportData& data) 
    {
        const auto& buffer = data.data();
        uint8_t* ptr = const_cast<uint8_t*>(buffer.Data_());
        char msg_buf[256] = { 0 };
        memmove(msg_buf, ptr, buffer.Size_());
        DriverLog("get usb msg %s", msg_buf);
        if (strcmp(msg_buf,"get_encode_param")==0)
        {
			WireLessType::EncodeParam  encode_param = { 0 };
			encode_param.render_width = gConfigReader.GetEveWidth();
			encode_param.render_height = gConfigReader.GetEveHeight();
			encode_param.encode_width = gConfigReader.GetEncoderWidth() / 2;
			encode_param.encode_heigth = gConfigReader.GetEncoderHeight();
			encode_param.cut_x = gConfigReader.GetCutx();
			encode_param.cut_y = gConfigReader.GetCuty();
			encode_param.compress = gConfigReader.GetComPress();
            bool ret=SendMsg((char*)&encode_param, sizeof(encode_param),4);
			DriverLog("encode param %d %d %d %d %d %d %d %d\n",
				encode_param.render_width, encode_param.render_height,
				encode_param.encode_width, encode_param.encode_heigth,
				encode_param.cut_x, encode_param.cut_y, encode_param.compress,ret);
        }
    }
    void UsbBulkModule::CheckSensorReceiveThread()
    {
        PXR_INFO_THIS("begin to check sensor timeout");
        while (engine_startup_)
        {
            Sleep(2000);
            if (!connected_)
                continue;
            static const uint64_t kDefaultSensorTimeout = 5000000;
            uint64_t timestamp = GetTimestampUs();

            if (gConfigReader.GetRtcOrBulkMode_() == 2)
            {
                if (last_hmd_recv_timestamp_ > 0
                    && (timestamp - last_hmd_recv_timestamp_) > kDefaultSensorTimeout)
                {
                    PXR_ERROR_THIS("hmd sensor timeout");
                    RVR::RVRPoseHmdData hmd_pose = { 0 };
					memmove(&hmd_pose, &hmd_pose_, sizeof(RVR::RVRPoseHmdData));
					hmd_pose.valid = FALSE;
                    g_svrDriver.AddHmdPose(&hmd_pose);
                }

                if (last_left_controller_recv_timestamp_ > 0 &&
                    (timestamp - last_left_controller_recv_timestamp_) > kDefaultSensorTimeout)
                {
                    PXR_ERROR_THIS("left controller timeout");
                    RVR::RVRControllerData controller_data = { 0 };
					memmove(&controller_data, &left_pose_, sizeof(RVR::RVRPoseHmdData));
					 
                    controller_data.connectionState = RVR::RVRControllerConnectionState::kDisconnected;
                    g_svrDriver.AddControllerPose(0, &controller_data);
                }

                if (last_right_controller_recv_timestamp_ > 0 &&
                    (timestamp - last_right_controller_recv_timestamp_) > kDefaultSensorTimeout)
                {
                    PXR_ERROR_THIS("right controller timeout");
                    RVR::RVRControllerData controller_data = { 0 };
                    memmove(&controller_data, &right_pose_, sizeof(RVR::RVRPoseHmdData));
                    controller_data.connectionState = RVR::RVRControllerConnectionState::kDisconnected;
                    g_svrDriver.AddControllerPose(1, &controller_data);
                }
            }
        }
        PXR_INFO_THIS("check sensor stopped");
    }

    bool UsbBulkModule::CheckUsbMode()
    {
        return gConfigReader.GetRtcOrBulkMode_() == 2;
    }

    void UsbBulkModule::TestFuncSaveVideoFrame(uint8_t* data, int length,
        int picture_type, int eye_inedex, int64_t gen_timestamp)
    {
        static FILE* addseifile = fopen("leftUsbBulk.h265", "wb+");
        static FILE* addseifile2 = fopen("rightUsbBulk.h265", "wb+");

        if (eye_inedex == 0 && addseifile != NULL)
        {
            fwrite(data, length, sizeof(char), addseifile);
        }

        if (eye_inedex == 1 && addseifile2 != NULL)
        {
            fwrite(data, length, sizeof(char), addseifile2);
        }
        PXR_DEBUG_THIS("save video frame, eye_index= " << eye_inedex
            << ", size = " << length << ", timestamp= " << gen_timestamp);
    }

    void UsbBulkModule::TestFuncSaveAudioFrame(uint8_t* data, int length)
    {
        static FILE* audioSavedfile = fopen("audioUsbBulk.pcm", "wb+");

        fwrite(data, length, sizeof(char), audioSavedfile);
        PXR_DEBUG("save audio frame  size = " << length );
    }
}
#endif
