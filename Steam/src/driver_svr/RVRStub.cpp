/*----------------------------------------------------------------------------*\
**          Copyright (c) 2017-2018 Qualcomm Technologies, Inc.               **
**                       All Rights Reserved.                                 **
**     Confidential and Proprietary - Qualcomm Technologies, Inc.             **
\*----------------------------------------------------------------------------*/

#pragma once
#include "SensorSocket.h"
#include "TcpSensorSocket.h"
#include "RVRStub.h"
//#include <RVRUnityInterface.h>
#include <RVRUtils.h>

#include "config_reader.h"
#include "driverlog.h"
#include "TimeTool.h"
extern ConfigReader gConfigReader;
RVRStub RVRStub::mInstance;

RVRStub::RVRStub() : bInit(false)
{
}

RVRStub::~RVRStub()
{
	
}

RVRStub* RVRStub::Instance()
{
	return &mInstance;
}

bool RVRStub::Init(ID3D11Device* device)
{
	//HRESULT hr;
	if (!bInit)
	{
		
		m_device = device;
		
		m_pose = new RVR::RVRPoseHmdData;
		if (!m_pose)
			return false;
		else
		{
			bInit = true;
			return true;
		}
		
	}
	else
		return true;
}

void RVRStub::ShutDown()
{	
	SensorSocket::GetInstance(-1)->ShutDown();
	//SensorSocket::GetInstance(0)->ShutDown();
	//SensorSocket::GetInstance(1)->ShutDown();
	delete m_pose;
}

void RVRStub::StartPoseRecv()
{
	SensorSocket::GetInstance();
	SensorSocket::GetInstance()->SetCallBack(RVRStub::SetPose);
	SensorSocket::GetInstance()->InitSocket("127.0.0.1", gConfigReader.GetPortH());
	
	TcpSensorSocket::GetInstance();
	 
	TcpSensorSocket::GetInstance()->InitSocket("127.0.0.1", 29724);
	//SensorSocket::GetInstance(0);	 
    //SensorSocket::GetInstance(0)->InitSocket("127.0.0.1", gConfigReader.GetPortL()+30 );

	//SensorSocket::GetInstance(1); 
	//SensorSocket::GetInstance(1)->InitSocket("127.0.0.1", gConfigReader.GetPortR()+30 );
	
}

RVR::RVRPoseHmdData* RVRStub::GetPose()
{	
	//WireLessType::TransPoseData pose;
	//SensorSocket::GetInstance()->GetPose(pose);
	//m_pose->rotation.w = pose.rotation.w;
	//m_pose->rotation.x = pose.rotation.x;
	//m_pose->rotation.y = pose.rotation.y;
	//m_pose->rotation.z = pose.rotation.z;

	//m_pose->position.x = pose.position.x;
	//m_pose->position.y = pose.position.y;
	//m_pose->position.z = pose.position.z;

	//m_pose->poseRecvTime = pose.poseRecvTime;
	//m_pose->poseTimeStamp = pose.poseTimeStamp;
	//m_pose->predictedTimeMs = pose.predictedTimeMs;
	 
	/*DriverLog("rotation: w=%lf,x=%lf,y=%lf,z=%lf    Position:X=%lf, Y=%lf, Z=%lf\n",
		pose.rotation.w, pose.rotation.x, pose.rotation.y, pose.rotation.z,
		pose.position.x, pose.position.y, pose.position.z);*/
	return m_pose;
}

void RVRStub::SetPose(void * data)
{
	WireLessType::TransPoseData *pose = (WireLessType::TransPoseData*)data;
	
	RVRStub::Instance()->m_pose->rotation.w = pose->rotation.w;
	RVRStub::Instance()->m_pose->rotation.x = pose->rotation.x;
	RVRStub::Instance()->m_pose->rotation.y = pose->rotation.y;
	RVRStub::Instance()->m_pose->rotation.z = pose->rotation.z;

	RVRStub::Instance()->m_pose->position.x = pose->position.x;
	RVRStub::Instance()->m_pose->position.y = pose->position.y;
	RVRStub::Instance()->m_pose->position.z = pose->position.z;

	RVRStub::Instance()->m_pose->poseRecvTime = pose->poseRecvTime;
	RVRStub::Instance()->m_pose->poseTimeStamp = pose->poseTimeStamp;
	RVRStub::Instance()->m_pose->predictedTimeMs = pose->predictedTimeMs;
	
}

void RVRStub::GetControllerPose(uint32_t index, RVR::RVRControllerData* controllerPose)
{
	WireLessType::TransControllerData pose;
	((ControllerSensorSocket*)SensorSocket::GetInstance(index))->GetPose(pose);
	memmove(&controllerPose->analog1D, &pose.analog1D, sizeof(float) * 8);
	memmove(&controllerPose->analog2D, &pose.analog2D, sizeof(RVR::RVRVector2) * 4);
	controllerPose->buttonState = pose.buttonState;
	controllerPose->connectionState = (RVR::RVRControllerConnectionState)pose.connectionState;
	controllerPose->isTouching = pose.isTouching;
	controllerPose->position.x = pose.position.x;
	controllerPose->position.y = pose.position.y;
	controllerPose->position.z = pose.position.z;
	controllerPose->rotation.x = pose.rotation.x;
	controllerPose->rotation.y = pose.rotation.y;
	controllerPose->rotation.z = pose.rotation.z;
	controllerPose->rotation.w = pose.rotation.w;
	controllerPose->timestamp = pose.timestamp;
	controllerPose->vecAcceleration.x = pose.vecAcceleration.x;
	controllerPose->vecAcceleration.y = pose.vecAcceleration.y;
	controllerPose->vecAcceleration.z = pose.vecAcceleration.z;
	controllerPose->vecAngularAcceleration.x = pose.vecAngularAcceleration.x;
	controllerPose->vecAngularAcceleration.y = pose.vecAngularAcceleration.y;
	controllerPose->vecAngularAcceleration.z = pose.vecAngularAcceleration.z;
	controllerPose->vecAngularVelocity.x = pose.vecAngularVelocity.x;
	controllerPose->vecAngularVelocity.y = pose.vecAngularVelocity.y;
	controllerPose->vecAngularVelocity.z = pose.vecAngularVelocity.z;
	controllerPose->vecVelocity.x = pose.vecVelocity.x;
	controllerPose->vecVelocity.y = pose.vecVelocity.y;
	controllerPose->vecVelocity.z = pose.vecVelocity.z;
}
