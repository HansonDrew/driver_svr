#pragma once
#include <string>
#include <mutex>
class ConfigReader
{
public:
	ConfigReader();
	void ReadConfig(std::string configPath);
	int GetControllerType() { return mControllerType; };
	int GetTriggerTest() { return mTestTrigger; };
	int GetSuperModel() { return mSuperModel; };
	int GetAudioCapturePort() {return mAudioCapturePort;}
	int GetMtu() { return mMtu; }
	int GetAudioPicoModel() { return mPicoAudioModel; }
	int GetAudioDelayTime() { return mAudioDelayTime; }
	int GetPortL() { return mPortL; };
	int GetPortR() { return mPortR; };
	int GetPortH() { return mPortH; };
	int GetAverageBitRate() { return mAverageBitRate; };
	int GetMaxBitRateValue() { return mMaxBitRate; };
	int GetCutx() { return mCutx; };
	int GetCuty() { return mCuty; };
	int GetEveWidth() { return mWidth; };
	int GetEveHeight() { return mHeight; };
	float GetFov() { return fov; };
	float GetInterPupilDistance() { return interPupilDistance; };
	int GetFps() { return fps; };
	int GetEncodeFps();
	std::string GetDstIp() {
		return mdstip;
	};
	std::wstring wdriverPath;
	std::string GetleftControllerModelNumber() { return mleftControllerModelNumber; }
	std::string GetrightControllerModelNumber() { return mrightControllerModelNumber; }
	std::string GetHmdModelNumber() { return mHmdModelNumber; }
	std::string GetControllerInputProfilePath() { return mControllerInputProfilePath; }
	std::string GetHmdInputProfilePath() { return mHmdInputProfilePath; }
	std::string GetSystemName() { return mSystemName; }
	std::string GetleftControllerModelName(){ return mleftControllerModelName; }
	std::string GetrightControllerModelName() { return mrightControllerModelName; }
	std::string GetManufacturerName() { return mManufacturerName; }
	int GetGopSize() const
	{
		return mGopSize;
	}
	int GetInsertIdrEnable();
	int GetCutFlag() { return mCutFlag; };
	~ConfigReader();
	
	float GetControllerpose();
	float GetHmdpose();

	float GetBright();
	float GetBrightValue() { return bright_; };
	float GetSaturation();
	float GetSaturationValue() { return saturation_; };
	float GetContrast();
	float GetContrastValue() { return contrast_; };
	float GetAlpha();
	float GetAlphaValue() { return alpha_; };
	float GetGamma();
	float GetGammaValue() { return gamma_; };
	float GetSharper();
	float GetSharperValue() { return shaper; };
	float GetLeftPitch();
	float GetLeftPitchValue() { return left_pitch_; };

	float GetLeftYaw();
	float GetLeftYawValue() { return left_yaw_; };

	float GetLeftRoll();
	float GetLeftRollValue() { return left_roll_; };
	float GetLeftAddX();
	float GetLeftAddXValue() { return left_add_x_; };
	float GetLeftAddY();
	float GetLeftAddYValue() { return left_add_y_; };
	float GetLeftAddZ();
	float GetLeftAddZValue() { return left_add_z_; };

	float GetRightPitch();
	float GetRightPitchValue() { return right_pitch_; };
	float GetRightYaw();
	float GetRightYawValue() { return right_yaw_; };

	float GetRightRoll();
	float GetRightRollValue() { return right_roll_; };
	float GetRightAddX();
	float GetRightAddXValue() { return right_add_x_; };
	float GetRightAddY();
	float GetRightAddYValue() { return right_add_y_; };
	float GetRightAddZ();
	float GetRightAddZValue() { return right_add_z_; };
	int GetComPress() { return mComPress; };
	int GetEncoderWidth() { return mEncoderWidth; };
	int GetEncoderHeight() { return mEncoderHeight; };
	int GetFindHistoryPose() { return find_history_pose_; };
	std::string GetHmdType() { return mHmdType; };
	float GetVibrationtime();
	int GetAdjustControllerType_();
	int BigPicture() { return mBigPicture; };
	int GetTcpValue() { return mTcp; };
	int GetTcp();
	int GetControllerAccFlag();
	int GetLog() { return mLog; };
	int GetUsbLog() { return mUsbLog; };
	int GetReversal() { return mReversal; };
	int GetHEVC() { return mHEVC; };
	int GetRtcOrBulkMode_() { return rtc_mode_; };
	int GetRtcOrBulkModeFromFile_();
	int GetLinearResolation() { return mLinearResolation; };
	int GetMicWork_();
	int GetMicWorkValue() { return mic_work_; };
	int GetSplit_() { return split_; };
	int GetDepthComputeValue() { return depth_compute_; };
	int GetDepthCompute();
	int GetSmoothController();
	int GetSmoothControllerValue() { return smooth_controller_; };
	int GetSensorTogetherValue() { return sensor_together_; };
	std::mutex max_sensor_store_lock_;
	int GetMaxSensorStoreValue() { 
		max_sensor_store_lock_.lock();
		int ret = max_sensor_store_;
		max_sensor_store_lock_.unlock();
		return ret;
	};
	int GetMaxSensorStore();
	int SetMaxSensorStore(int max_sensor_store);
	int SetAppRun_(int value);
	int GetAppRunValue() { return app_run_; };
	int GetSingleEncode() { return single_encode; };
	int GetNotEncode();
	int GetNotEncodeValue() { return not_encode_; };
	int GetAADTFlag() { return AADT_Func; };
private:
	int AADT_Func = 0;
	int not_encode_ = 0;
	int app_run_ = 0;
	int single_encode = 0;
	int sensor_together_ = 1;
	int max_sensor_store_ = 3;
	int smooth_controller_ = 0;
	int depth_compute_ = 1;
	int rtc_mode_ = 0;
	int mHEVC=1;
	int mReversal = 0;
	int mLog;
	int mUsbLog;
	int mControllerAccFlag;
	int mBigPicture;
	int mTcp;
	float bright_ = 1;
	float	saturation_ = 1.0;
	float	contrast_ = 1;
	float alpha_ = 1;
	float gamma_ = 1;
	float shaper=0;
	float left_pitch_ = 0.0f;
	float left_yaw_ = 0.0f;
	float left_roll_ = 0.0f;
	float left_add_x_ = 0.0f;
	float left_add_y_ = 0.0f;
	float left_add_z_ = 0.0f;
	int mic_work_ = 0;

	float right_pitch_ = 0.0f;
	float right_yaw_ = 0.0f;
	float right_roll_ = 0.0f;
	float right_add_x_ = 0.0f;
	float right_add_y_ = 0.0f;
	float right_add_z_ = 0.0f;
	int mControllerType ;//0vive ,1oculus ,2pico,3cosmos,4vive(supper authority)
	int mTestTrigger ;
	int mSuperModel;
	int mGopSize;
	std::string  mHmdInputProfilePath;
	std::string mleftControllerModelNumber ;
	std::string mrightControllerModelNumber;
	std::string mHmdModelNumber;
	std::string mControllerInputProfilePath;
	std::string mleftControllerModelName;
	std::string mrightControllerModelName;
	std::string mSystemName;
	std::string mManufacturerName;
	std::string mHmdType;
	int mAudioCapturePort;
	int mMtu;
	int mPicoAudioModel;
	int mAudioDelayTime;
	int mPortL;
	int mPortR;
	int mPortH;
	int mAverageBitRate;
	int mMaxBitRate;
	int mCutx;
	int mCuty;
	int mCutFlag;
	int mLinearResolation;
	int mWidth;
	int mHeight;
	int find_history_pose_;
	int split_;
	float  fov;
	float interPupilDistance;
	float vibrationtime;
	int fps;
	int adjust_controller_type_ = 0;
	std::string mdstip;
	int mComPress = 2; //coefficient of compressibilityÑ¹ËõÏµÊý 480/240=2
	int mEncoderWidth = 0;
	int mEncoderHeight = 0;
};

