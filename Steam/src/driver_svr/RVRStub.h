/*----------------------------------------------------------------------------*\
**          Copyright (c) 2017-2018 Qualcomm Technologies, Inc.               **
**                       All Rights Reserved.                                 **
**     Confidential and Proprietary - Qualcomm Technologies, Inc.             **
\*----------------------------------------------------------------------------*/

#pragma once

#include "RVRPluginDefinitions.h"
//#include "RVRUnityInterface.h"

struct ID3D11Device;
struct ID3D11Texture2D;

class RVRStub
{
public:

	static RVRStub* Instance();
	bool Init(ID3D11Device* device);
	void ShutDown();
	void StartPoseRecv();
	 

	RVR::RVRPoseHmdData* GetPose();
	static void SetPose(void *data);
	void GetControllerPose(uint32_t index, RVR::RVRControllerData*);
	 
	
private:
	RVRStub();
	~RVRStub();
	ID3D11Device* m_device;	 
	RVR::RVRPoseHmdData* m_pose;
	bool bInit;

	static RVRStub mInstance;
};
