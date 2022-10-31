//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once

#include <openvr_driver.h>
#include "RVRStub.h"
#include "driver_define.h"
inline bool ButtonMask(short button_state, short state_mask)
{
	if ((button_state & state_mask) != 0)
	{
		return true;
	}
	return false;
}

inline bool ButtonMaskInt(int button_state, int state_mask)
{
	if ((button_state & state_mask) != 0)
	{
		return true;
	}
	return false;
}
class ControllerDriver : public vr::ITrackedDeviceServerDriver
{
    uint32_t mControllerIndex;
    RVRStub* mStubInstance;
    vr::DriverPose_t driver_pose_;
    vr::TrackedDeviceIndex_t m_unObjectId;
    vr::PropertyContainerHandle_t m_ulPropertyContainer;
	//modiefy by dzhuang
	vr::VRInputComponentHandle_t handles[24];
	//end
    uint32_t lastTS[2] = { 0 , 0 };
	//add by dzhuang
	float mGripValueMax = 0;
	float mGripValue = 0;
    bool is_actived_ = false;
    bool is_added_ = false;
    std::string m_sSerialNumber;
	//end 
public:
   
    vr::TrackedDeviceIndex_t GetDriverObjId() { return m_unObjectId; };
	
    RVR::RVRControllerData last_data;
    void* GetControllerIndexPtr() { return (void*)&mControllerIndex; };
    void SetIcons();
    std::string GetSerialNumber() const { return m_sSerialNumber; }
    bool GetIsActived_() { return is_actived_; };
    void LimitPose(RVR::RVRControllerData* pose);
    void SetIsActived_(bool is_actived) { is_actived_ = is_actived; }
	bool GetIsAdded_() { return is_added_; };
	void SetIsAdded_(bool is_added) { is_added_ = is_added; }
    ControllerDriver(RVRStub* stub, int index);
    virtual ~ControllerDriver();
    virtual vr::EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId);
    virtual void Deactivate();
    virtual void EnterStandby();
    void *GetComponent(const char *pchComponentNameAndVersion);
    virtual void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize);
    void ChangeRotation(RVR::RVRControllerData& pose, RVR::RVRQuaternion increment);
    void GetDriverPose(vr::DriverPose_t* pose, RVR::RVRControllerData* data);
    virtual vr::DriverPose_t GetPose();
    void ReSetPose();
    void RunFrame();
    void UpdatePose(RVR::RVRControllerData* pose = nullptr,bool dp=false);
    void UpdateButtonState(RVR::RVRControllerData data);
    void UpdateButtonState(HidType::ButtonStateGather button_state);
	void UpdateSkeleton(RVR::RVRControllerData data);
    uint64_t GetHapticsHandle();
	uint64_t track_add_last = 0;
	uint64_t track_add = 0;
    uint64_t last_timestamp = 0;
    uint64_t last_hmd_timestamp = 0;
    void AdjustControllerPose(RVR::RVRControllerData& pose);
    void BaseAdjustControllerPose(RVR::RVRControllerData& pose,bool dp, std::string hmd_type);
    void ToOculusAdjustControllerPose(RVR::RVRControllerData& pose, bool dp, std::string hmd_type);
    void ToHtcAdjustControllerPose(RVR::RVRControllerData& pose, bool dp, std::string hmd_type);


	void GetBoneTransform(bool withController,
		bool isLeftHand,
		float thumbAnimationProgress,
		float indexAnimationProgress,
		uint64_t lastPoseTouch,
		RVR::RVRControllerData data,
		vr::VRBoneTransform_t outBoneTransform[]);

private:
	static const int SKELETON_BONE_COUNT = 31;
	static const int ANIMATION_FRAME_COUNT = 15;

	float* m_poseTimeOffset;

	 
	vr::VRInputComponentHandle_t m_compSkeleton = vr::k_ulInvalidInputComponentHandle;
	enum HandSkeletonBone : size_t {
		HSB_Root = 0,
		HSB_Wrist,
		HSB_Thumb0,
		HSB_Thumb1,
		HSB_Thumb2,
		HSB_Thumb3,
		HSB_IndexFinger0,
		HSB_IndexFinger1,
		HSB_IndexFinger2,
		HSB_IndexFinger3,
		HSB_IndexFinger4,
		HSB_MiddleFinger0,
		HSB_MiddleFinger1,
		HSB_MiddleFinger2,
		HSB_MiddleFinger3,
		HSB_MiddleFinger4,
		HSB_RingFinger0,
		HSB_RingFinger1,
		HSB_RingFinger2,
		HSB_RingFinger3,
		HSB_RingFinger4,
		HSB_PinkyFinger0,
		HSB_PinkyFinger1,
		HSB_PinkyFinger2,
		HSB_PinkyFinger3,
		HSB_PinkyFinger4,
		HSB_Aux_Thumb,        // Not used yet
		HSB_Aux_IndexFinger,  // Not used yet
		HSB_Aux_MiddleFinger, // Not used yet
		HSB_Aux_RingFinger,   // Not used yet
		HSB_Aux_PinkyFinger,  // Not used yet
		HSB_Count
	};
	vr::VRBoneTransform_t m_boneTransform[HSB_Count];
	float m_thumbAnimationProgress = 0;
	float m_indexAnimationProgress = 0;
	uint64_t m_lastThumbTouch = 0;
	uint64_t m_lastIndexTouch = 0;
};
