//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#include "controller_driver.h"
#include "Util.h"
#include "RVRLogger.h"
#include "openvrtool.h"
#include "config_reader.h"
#include "driverlog.h"
#include "TimeTool.h"
#include "driver_define.h"
extern ConfigReader gConfigReader;
using namespace RVR;
using namespace vr;
extern bool gLogOutControllerPose  ;
extern bool gIsDebug;
extern bool gLogOutHmdPose;
extern float gControllerPose  ;
extern float gHmdTimeOffset  ;
extern int gControllerAcc;
extern uint32_t gBasePid ;
extern uint32_t gPid ;
extern  bool gDashboardActivated;
extern bool gOverlayShow;
extern bool gAppConnected ;
extern bool g_test_sensor;
extern int g_test_sensor_mode;
//-----------------------------------------------------------------------------
ControllerDriver::ControllerDriver(RVRStub* stub, int index)
//-----------------------------------------------------------------------------
{
	 
    this->mControllerIndex = index;
    mStubInstance = stub;
	if (index==0)
	{
		m_sSerialNumber = "Pico Left";
		if (gConfigReader.GetHmdType().compare("neo2") == 0)
		{
			m_sSerialNumber.assign("Pico Neo2 Controller left");			 
		}
		else if (gConfigReader.GetHmdType().compare("neo3") == 0)
		{
			m_sSerialNumber.assign("Pico Neo3 Controller left");			
		}	
		else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
		{
			m_sSerialNumber.assign("PICO 4 Controller left");
		}
		if (gConfigReader.GetControllerType() == 1)
		{
			m_sSerialNumber.assign("WMHD316Q512X12_Controller_Left");
		}
	} 
	else
	{
		m_sSerialNumber = "Pico Right";
		if (gConfigReader.GetHmdType().compare("neo2") == 0)
		{
			m_sSerialNumber.assign("Pico Neo2 Controller right");		 
		}
		else if (gConfigReader.GetHmdType().compare("neo3") == 0)
		{
			m_sSerialNumber.assign("Pico Neo3 Controller right");			
		}
		else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
		{
			m_sSerialNumber.assign("PICO 4 Controller right");
		}
		if (gConfigReader.GetControllerType() == 1)
		{
			m_sSerialNumber.assign("WMHD316Q512X12_Controller_Right");
		}
	}
	
}

//-----------------------------------------------------------------------------
ControllerDriver::~ControllerDriver()
//-----------------------------------------------------------------------------
{

}
void ControllerDriver::SetIcons()
{
	if (gConfigReader.GetHmdType().compare("neo2") == 0)
	{
		if (mControllerIndex==0)
		{
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons_neo2/controller_off.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons_neo2/controller_searching.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons_neo2/controller_searching_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{pico}/icons_neo2/controller_ready.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons_neo2/controller_ready_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons_neo2/controller_error.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons_neo2/controller_standby.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons_neo2/ontroller_ready_low.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons_neo2/controller_ready_alert.png");
		} 
		else
		{
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons_neo2/controller_off.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons_neo2/controller_searching.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons_neo2/controller_searching_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{pico}/icons_neo2/controller_ready.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons_neo2/controller_ready_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons_neo2/controller_error.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons_neo2/controller_standby.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons_neo2/controller_ready_low.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons_neo2/controller_ready_alert.png");
		}
		
	}else if (gConfigReader.GetHmdType().compare("neo3")==0)
	{
		if (mControllerIndex == 0)
		{
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_LeftHand);
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons_neo3/left_controller_off.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons_neo3/left_controller_searching.gif");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons_neo3/left_controller_searching_alert.gif");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{pico}/icons_neo3/left_controller_ready.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons_neo3/left_controller_ready_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons_neo3/left_controller_error.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons_neo3/left_controller_standby.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons_neo3/left_controller_ready_low.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons_neo3/left_controller_ready_alert.png");
		}
		else
		{
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_RightHand);
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons_neo3/right_controller_off.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons_neo3/right_controller_searching.gif");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons_neo3/right_controller_searching_alert.gif");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{pico}/icons_neo3/right_controller_ready.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons_neo3/right_controller_ready_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons_neo3/right_controller_error.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons_neo3/right_controller_standby.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons_neo3/right_controller_ready_low.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons_neo3/right_controller_ready_alert.png");
		}
	}
	else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
	{
		if (mControllerIndex == 0)
		{
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_LeftHand);
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons_phoenix/left_controller_off.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons_phoenix/left_controller_searching.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons_phoenix/left_controller_searching_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{pico}/icons_phoenix/left_controller_ready.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons_phoenix/left_controller_ready_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons_phoenix/left_controller_error.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons_phoenix/left_controller_standby.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons_phoenix/left_controller_ready_low.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons_phoenix/left_controller_ready_alert.png");
		} 
		else
		{
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_RightHand);
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons_phoenix/right_controller_off.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons_phoenix/right_controller_searching.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons_phoenix/right_controller_searching_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{pico}/icons_phoenix/right_controller_ready.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons_phoenix/right_controller_ready_alert.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons_phoenix/right_controller_error.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons_phoenix/right_controller_standby.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons_phoenix/right_controller_ready_low.png");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons_phoenix/right_controller_ready_alert.png");
		}
	}
}
//-----------------------------------------------------------------------------
vr::EVRInitError ControllerDriver::Activate(vr::TrackedDeviceIndex_t unObjectId)
//-----------------------------------------------------------------------------
{
	m_unObjectId = unObjectId;
	m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);
	//add by dzhuang
	uint64_t supportedButtons = 0xFFFFFFFFFFFFFFFFULL;
	vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, vr::Prop_SupportedButtons_Uint64, supportedButtons);
	if (gConfigReader.GetControllerType()==2|| gConfigReader.GetControllerType() == 4)
	{
		vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);
	
	}
	else
	{
		vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_Joystick);
		
	}

	//driver may not have authority,transfer this functionality to PC assistant
	//ChangeRenderModelFile(gConfigReader.GetControllerType()); 
	
	if (gConfigReader.GetControllerType() == 0)//pico
	{	
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "pico");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "oculus_touch");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "PICO");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::ETrackedDeviceProperty::Prop_InputProfilePath_String, "{oculus}/input/touch_profile.json");
		string register_type = "oculus/+";
		register_type = register_type + m_sSerialNumber;
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, register_type.c_str());
		if (mControllerIndex == 0)
		{
			
			if (gConfigReader.GetHmdType().compare("neo2")==0)
			{							
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Neo2  Controller left");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_neo2_leftcontroller");
			} 
			else if(gConfigReader.GetHmdType().compare("neo3") == 0)
			{				
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Neo3  Controller left");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_neo3_leftcontroller");
			}
			else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "PICO  4  Controller left");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_phoeinx_leftcontroller");
			}
			
		}
		else
		{
			
			if (gConfigReader.GetHmdType().compare("neo2") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/Pico  Neo2  Controller right");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Neo2  Controller right");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_neo2_rightcontroller");
			}
			else if (gConfigReader.GetHmdType().compare("neo3") == 0) 
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/Pico  Neo3  Controller right");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Neo3  Controller right");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_neo3_rightcontroller");
			}
			else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "PICO  4  Controller right");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_phoeinx_rightcontroller");
			}
			
		}

	}
	else if (gConfigReader.GetControllerType() == 1)//Oculus
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "oculus");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "oculus_touch");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "Oculus");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::ETrackedDeviceProperty::Prop_InputProfilePath_String, "{oculus}/input/touch_profile.json");
		if (mControllerIndex == 0)
		{
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/WMHD316Q512X13");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Oculus Rift CV1 (Left Controller)");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "oculus_cv1_controller_left");
		}
		else
		{
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/WMHD316Q512X13");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Oculus Rift CV1 (Right Controller)");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "oculus_cv1_controller_right");
		}
		/*if (mControllerIndex == 0)
		{
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/WMHD316Q512X13");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Oculus Quest (Left Controller)");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "oculus_rifts_controller_left");		
		}
		else
		{
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/WMHD316Q512X13");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Oculus Quest (Right Controller)");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "oculus_rifts_controller_right");
		}*/
	}
	else if (gConfigReader.GetControllerType() == 2)//htc
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "lighthouse");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "vive_controller");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "HTC");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::ETrackedDeviceProperty::Prop_InputProfilePath_String, "{htc}/input/vive_controller_profile.json");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "VIVE Controller Pro MV");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "htc/vive_controllerLHR-5BC03915"); 
	}
	else if (gConfigReader.GetControllerType() == 3|| gConfigReader.GetControllerType() == 4)//really pico
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "Pico");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "pico");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "pico_controller");
		if (gConfigReader.GetControllerType() == 3)
		{

			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_Joystick);
			if (gConfigReader.GetHmdType().compare("neo3")==0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::ETrackedDeviceProperty::Prop_InputProfilePath_String, "{pico}/input/pico_neo3_controller_profile.json");
			} 
			else if(gConfigReader.GetHmdType().compare("phoenix") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::ETrackedDeviceProperty::Prop_InputProfilePath_String, "{pico}/input/pico_phoenix_controller_profile.json");
			}
		
		}
		else if (gConfigReader.GetControllerType() == 4)
		{

			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);
			if (gConfigReader.GetHmdType().compare("neo3") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::ETrackedDeviceProperty::Prop_InputProfilePath_String, "{pico}/input/pico_neo3_controller2_profile.json");
			}
			else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::ETrackedDeviceProperty::Prop_InputProfilePath_String, "{pico}/input/pico_phoenix_controller2_profile.json");
			}
		}

		if (mControllerIndex ==(int) ControllerIndex::kLeftController)
		{
			if (gConfigReader.GetHmdType().compare("neo2") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Neo2  Controller left");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_neo2_leftcontroller");
			}
			else if (gConfigReader.GetHmdType().compare("neo3") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Neo3  Controller left");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_neo3_leftcontroller");
			}
			else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Phoenix  Controller left");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_phoeinx_leftcontroller");
			}
		}
		else
		{
			if (gConfigReader.GetHmdType().compare("neo2") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Neo2 Controller right");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_neo2_rightcontroller");
			}
			else if (gConfigReader.GetHmdType().compare("neo3") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Neo3  Controller right");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_neo3_rightcontroller");
			}
			else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
			{
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Pico  Phoenix  Controller right");
				vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{pico}/rendermodels/pico_phoeinx_rightcontroller");
			}
		}
		string register_type = "pico/";
		register_type = register_type + m_sSerialNumber;
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, register_type.c_str());
	}

	vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
	
	if (mControllerIndex == 0)
	{
		
		vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_LeftHand);
	/*	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{pico}/icons/controller_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{pico}/icons/controller_off.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{pico}/icons/controller_standby.png");*/
	}
	else
	{
		vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_RightHand);
		/*vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{pico}/icons/controller_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{pico}/icons/controller_off.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{pico}/icons/controller_standby.png");*/
	}
	//end 



	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trigger/click", &handles[Button_Handle::kTriggerClick]);

	vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trigger/value", &handles[Button_Handle::kTriggerValue], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);

	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/grip/click", &handles[Button_Handle::kGripClick]);
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/system/click", &handles[Button_Handle::kSystemClick]);
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/system/touch", &handles[Button_Handle::kSystemTouch]);
	if (gConfigReader.GetControllerType() == 2 || gConfigReader.GetControllerType() == 4)
	{
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/application_menu/click", &handles[Button_Handle::kApplicationMenu]);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trackpad/click", &handles[Button_Handle::kTrackpadOrJoystickClick]);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trackpad/touch", &handles[Button_Handle::kTrackpadTouch]);
		vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trackpad/x", &handles[Button_Handle::kTrackpadx], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
		vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trackpad/y", &handles[Button_Handle::kTrackpady], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);

	}
	else if (gConfigReader.GetControllerType() == 0||gConfigReader.GetControllerType() == 1 || gConfigReader.GetControllerType() == 3)
	{
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/joystick/click", &handles[Button_Handle::kTrackpadOrJoystickClick]);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/joystick/touch", &handles[Button_Handle::kJoystickTouch]);
		vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/joystick/x", &handles[Button_Handle::kJoystickx], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
		vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/joystick/y", &handles[Button_Handle::kJoysticky], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
		vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/grip/value", &handles[Button_Handle::kGripValue], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/grip/touch", &handles[Button_Handle::kGripTouch]);

		if (mControllerIndex == 0)
		{
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/x/click", &handles[Button_Handle::kXClick]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/x/touch", &handles[Button_Handle::kXTouch]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/y/click", &handles[Button_Handle::kYClick]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/y/touch", &handles[Button_Handle::kYTouch]);

		}
		else
		{
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/a/click", &handles[Button_Handle::kAClick]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/a/touch", &handles[Button_Handle::kATouch]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/b/click", &handles[Button_Handle::kBClick]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/b/touch", &handles[Button_Handle::kBTouch]);
		}
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trigger/touch", &handles[Button_Handle::kTriggerTouch]);
	}

	/*if (mControllerIndex==0)
	{
		vr::VRDriverInput()->CreateSkeletonComponent(
			this->m_ulPropertyContainer,
			"/input/skeleton/left",
			"/skeleton/hand/left",
			"/pose/raw",
			vr::EVRSkeletalTrackingLevel::VRSkeletalTracking_Partial,
			nullptr,
			SKELETON_BONE_COUNT,
			&m_compSkeleton);
	} 
	else
	{
		vr::VRDriverInput()->CreateSkeletonComponent(
			this->m_ulPropertyContainer,
			"/input/skeleton/right",
			"/skeleton/hand/right",
			"/pose/raw",
			vr::EVRSkeletalTrackingLevel::VRSkeletalTracking_Partial,
			nullptr,
			SKELETON_BONE_COUNT,
			&m_compSkeleton);
	}*/
	//vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DoNotApplyPrediction_Bool, true);
	vr::EVRInputError input_error = vr::VRDriverInput()->CreateHapticComponent(m_ulPropertyContainer, "/output/haptic", &handles[Button_Handle::kHaptic]);
	if (input_error != vr::VRInputError_None)
	{
		handles[9] = 0;
	}
	SetIcons();
	is_actived_ = true;
	DriverLog("controller  %d  actived",mControllerIndex);
	return vr::VRInitError_None;
}

//-----------------------------------------------------------------------------
void ControllerDriver::Deactivate()
//-----------------------------------------------------------------------------
{
    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
}

//-----------------------------------------------------------------------------
void ControllerDriver::EnterStandby()
//-----------------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------------
void ControllerDriver::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize)
//-----------------------------------------------------------------------------
{

}

//-----------------------------------------------------------------------------
void* ControllerDriver::GetComponent(const char *pchComponentNameAndVersion)
//-----------------------------------------------------------------------------
{
    return NULL;
}

//-----------------------------------------------------------------------------
void ControllerDriver::GetDriverPose(vr::DriverPose_t* pose, RVR::RVRControllerData* data)
//-----------------------------------------------------------------------------
{
    pose->poseIsValid = true;
    pose->result = vr::TrackingResult_Running_OK;
    pose->deviceIsConnected = (data->connectionState == RVR::RVRControllerConnectionState::kConnected) ? true : false;
    pose->qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
    pose->qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

	pose->qRotation = HmdQuaternion_Init(1, 0, 0, 0);
	pose->vecPosition[0] = pose->vecPosition[1] = pose->vecPosition[2] = 0;
    pose->vecPosition[0] = data->position.x;
    pose->vecPosition[1] = data->position.y;
    pose->vecPosition[2] = data->position.z;

    pose->qRotation.w = data->rotation.w;
    pose->qRotation.x = data->rotation.x;
    pose->qRotation.y = data->rotation.y;
	pose->qRotation.z = data->rotation.z;
	

    pose->vecAcceleration[0] = data->vecAcceleration.x;
    pose->vecAcceleration[1] = data->vecAcceleration.y;
    pose->vecAcceleration[2] = data->vecAcceleration.z;

    pose->vecAngularVelocity[0] = data->vecAngularVelocity.x ;
    pose->vecAngularVelocity[1] = data->vecAngularVelocity.y  ;
    pose->vecAngularVelocity[2] = data->vecAngularVelocity.z  ;
	if (gConfigReader.GetHmdType().compare("neo2")==0)
	{
		pose->vecAngularVelocity[0] = pose->vecAngularVelocity[0]/5;
		pose->vecAngularVelocity[1] = pose->vecAngularVelocity[1]/5;
		pose->vecAngularVelocity[2] = pose->vecAngularVelocity[2]/5;
	}
	
    pose->vecVelocity[0] = data->vecVelocity.x;
    pose->vecVelocity[1] = data->vecVelocity.y;
    pose->vecVelocity[2] = data->vecVelocity.z;
	pose->vecAngularAcceleration[0] = pose->vecAngularAcceleration[1] = pose->vecAngularAcceleration[2] = 0;
    /*pose->vecAngularAcceleration[0] = data->vecAngularAcceleration.x;
    pose->vecAngularAcceleration[1] = data->vecAngularAcceleration.y;
    pose->vecAngularAcceleration[2] = data->vecAngularAcceleration.z;*/

	pose->poseTimeOffset = 0;// 0.035f; 正数 越大，预测越大。负数没有作用
}

//-----------------------------------------------------------------------------
vr::DriverPose_t ControllerDriver::GetPose()
//-----------------------------------------------------------------------------
{
    return driver_pose_;
}
void ControllerDriver::ReSetPose()
{
	driver_pose_.poseTimeOffset = 0;

	for (int i = 0; i < 3; i++)
	{
		driver_pose_.vecWorldFromDriverTranslation[i] = 0.0;
		driver_pose_.vecDriverFromHeadTranslation[i] = 0.0;
	}

	driver_pose_.qRotation.w = 1;
	driver_pose_.qRotation.x = 0;
	driver_pose_.qRotation.y = 0;
	driver_pose_.qRotation.z = 0;

	driver_pose_.qWorldFromDriverRotation.w = 1;
	driver_pose_.qWorldFromDriverRotation.x = 0;
	driver_pose_.qWorldFromDriverRotation.y = 0;
	driver_pose_.qWorldFromDriverRotation.z = 0;

	driver_pose_.qDriverFromHeadRotation.w = 1;
	driver_pose_.qDriverFromHeadRotation.x = 0;
	driver_pose_.qDriverFromHeadRotation.y = 0;
	driver_pose_.qDriverFromHeadRotation.z = 0;

	// some things are always true
	driver_pose_.shouldApplyHeadModel = false;// true;
	driver_pose_.willDriftInYaw = false;

	// we don't do position, so these are easy
	for (int i = 0; i < 3; i++)
	{
		driver_pose_.vecPosition[i] = 0.0;
		driver_pose_.vecVelocity[i] = 0.0;
		driver_pose_.vecAcceleration[i] = 0.0;

		// we also don't know the angular velocity or acceleration
		driver_pose_.vecAngularVelocity[i] = 0.0;
		driver_pose_.vecAngularAcceleration[i] = 0.0;

	}
}
//-----------------------------------------------------------------------------
void ControllerDriver::RunFrame()
//-----------------------------------------------------------------------------
{
  //  UpdatePose();
}
void  ControllerDriver::ChangeRotation(RVR::RVRControllerData& pose, RVR::RVRQuaternion increment)
{
	RVR::RVRQuaternion out_rotation = { 0 };
	out_rotation.x = pose.rotation.w * increment.x + pose.rotation.x * increment.w + pose.rotation.y * increment.z - pose.rotation.z * increment.y;
	out_rotation.y = pose.rotation.w * increment.y + pose.rotation.y * increment.w + pose.rotation.z * increment.x - pose.rotation.x * increment.z;
	out_rotation.z = pose.rotation.w * increment.z + pose.rotation.z * increment.w + pose.rotation.x * increment.y - pose.rotation.y * increment.x;
	out_rotation.w = pose.rotation.w * increment.w - pose.rotation.x * increment.x - pose.rotation.y * increment.y - pose.rotation.z * increment.z;
	pose.rotation = out_rotation;
}

void ControllerDriver::LimitPose(RVR::RVRControllerData* pose) 
{
	if (abs(pose->position.x) > 1000)
	{
		if (pose->position.x < 0)
		{
			pose->position.x = -1000;
		}
		else if (pose->position.x > 0)
		{
			pose->position.x = 1000;
		}
	}
	if (abs(pose->position.y) > 1000)
	{
		if (pose->position.y < 0)
		{
			pose->position.y = -1000;
		}
		else if (pose->position.y > 0)
		{
			pose->position.y = 1000;
		}
	}
	if (abs(pose->position.z) > 1000)
	{
		if (pose->position.z < 0)
		{
			pose->position.z = -1000;
		}
		else if (pose->position.z > 0)
		{
			pose->position.z = 1000;
		}
	}
}
void ControllerDriver::UpdatePose(RVR::RVRControllerData* pose,bool dp)
{
	if (g_test_sensor)
	{
		if (g_test_sensor_mode<4)
		{
			return;
		}
		
	}
	if (gConfigReader.GetRtcOrBulkMode_() != 2)
	{
		ReSetPose();
	}
    if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid)
    {
        RVR::RVRControllerData data;
		
        data = *pose;
      
        {
         //Update Pose
			if ((gConfigReader.GetAppRunValue()==0)||gOverlayShow||gDashboardActivated)
			{
			if (gConfigReader.GetHmdType().compare("neo3")==0)
				{
					BaseAdjustControllerPose(data, dp,"neo3");
				}
				else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
				{
					BaseAdjustControllerPose(data, dp, "phoenix");
				}
				if (gLogOutControllerPose && gIsDebug)
				{
					DriverLog("apprun 0");
				}
				
			
			}
			else 
			{
				if (gLogOutControllerPose && gIsDebug)
				{
					DriverLog("apprun 1");
				}
				if (gConfigReader.GetControllerType()==0|| gConfigReader.GetControllerType() == 1|| gConfigReader.GetControllerType() == 3)
				{
					ToOculusAdjustControllerPose(data, dp, gConfigReader.GetHmdType());
				}
				else
				{
					ToHtcAdjustControllerPose(data, dp, gConfigReader.GetHmdType());
				}
			}
			last_data = data;
			AdjustControllerPose(data);
			 			
            GetDriverPose(&driver_pose_, &data);
		
			/*if (gControllerAcc == 0|| gConfigReader.GetAppRunValue() == 2||gOverlayShow || gDashboardActivated || gConfigReader.GetAppRunValue() == 0)
			{
				driver_pose_.vecAcceleration[0] = driver_pose_.vecAcceleration[1] = driver_pose_.vecAcceleration[2] = 0;
				driver_pose_.vecVelocity[0] = driver_pose_.vecVelocity[1] = driver_pose_.vecVelocity[2] = 0;
				driver_pose_.vecAngularVelocity[0] = driver_pose_.vecAngularVelocity[1] = driver_pose_.vecAngularVelocity[2] = 0;
				driver_pose_.vecAngularAcceleration[0] = driver_pose_.vecAngularAcceleration[1] = driver_pose_.vecAngularAcceleration[2] = 0;
			}*/
			if (g_test_sensor && (g_test_sensor_mode > 0))
			{
				return;
			}
            vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, driver_pose_, sizeof(vr::DriverPose_t));
            lastTS[mControllerIndex] = data.timestamp;
			
			if (gLogOutControllerPose&& gIsDebug)
			{
				char msg[2048] = {0};
				uint64_t timestamp = GetTimestampUs();
				/*sprintf_s(msg, "Controller index=%d rotation:%lf, %lf, %lf, %lf , pose:%lf, %lf, %lf ,connect=%d,vel:%lf, %lf, %lf,acc:%lf, %lf, %lf,angvel:%lf, %lf, %lf\n",
					mControllerIndex, driver_pose_.qRotation.w, driver_pose_.qRotation.x, driver_pose_.qRotation.y, driver_pose_.qRotation.z,
					driver_pose_.vecPosition[0], driver_pose_.vecPosition[1], driver_pose_.vecPosition[2], driver_pose_.deviceIsConnected,
					driver_pose_.vecVelocity[0], driver_pose_.vecVelocity[1], driver_pose_.vecVelocity[2],
					driver_pose_.vecAcceleration[0], driver_pose_.vecAcceleration[1], driver_pose_.vecAcceleration[2],
					driver_pose_.vecAngularVelocity[0], driver_pose_.vecAngularVelocity[1], driver_pose_.vecAngularVelocity[2]);*/
				sprintf_s(msg, "Controller index=%d rotation:%lf, %lf, %lf, %lf , pose:%lf, %lf, %lf ,connect=%d,vel:%lf, %lf, %lf,acc:%lf, %lf, %lf,angvel:%lf, %lf, %lf, timestamp=%llu,hmd_time_stamp=%llu,sub_location=%llu,sub_remote=%llu\n",
					mControllerIndex, data.rotation.w, data.rotation.x, data.rotation.y, data.rotation.z,
					data.position.x, data.position.y, data.position.z, driver_pose_.deviceIsConnected,
					data.vecVelocity.x, data.vecVelocity.y, data.vecVelocity.z,
					data.vecAcceleration.x, data.vecAcceleration.y, data.vecAcceleration.z,
					data.vecAngularVelocity.x, data.vecAngularVelocity.y, data.vecAngularVelocity.z,
					timestamp,data.timestamp,
					timestamp-last_timestamp,
					data.timestamp-last_hmd_timestamp);
				last_timestamp = timestamp;
				last_hmd_timestamp = data.timestamp;
				
				RVR_LOG_A(msg);
			}
			

        }
		
		//end
    }
}
 
void ControllerDriver::UpdateButtonState(RVR::RVRControllerData data)
{
	
	if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid)
	{
		data.analog1D[0] = data.analog1D[0] / 172 * 255;
		//add by dzhuang
		if (fabs(data.analog2D[0].x) > 0.003 || fabs(data.analog2D[0].y) > 0.003|| ButtonMaskInt(data.buttonState, 0x10))
		{
			data.isTouching = 31;
		}
		else
		{
			data.isTouching = 0;
		}//end

	// modidy by dzhuang  .add oculus action 
		{ //Update Buttons

			//PRIMARY_TRIGGER
			vr::VRDriverInput()->UpdateScalarComponent(handles[kSystemTouch], ButtonMaskInt(data.buttonState, 0x80), 0.0);
			vr::VRDriverInput()->UpdateBooleanComponent(handles[kTriggerClick], ButtonMaskInt(data.buttonState, 0x2000), 0.0);
			vr::VRDriverInput()->UpdateScalarComponent(handles[kTriggerValue], data.analog1D[0], 0.0);
		
			if (gConfigReader.GetControllerType() == 4 || gConfigReader.GetControllerType() == 2)
			{
				//SYSTEM_BUTTON
				
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kSystemClick], ButtonMaskInt(data.buttonState, 0x01), 0.0);
				
				//MENU/START				

				vr::VRDriverInput()->UpdateBooleanComponent(handles[kApplicationMenu], ButtonMaskInt(data.buttonState, 0x02),0.0);
	

				//PRIMARY_GRIP
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kGripClick], ButtonMaskInt(data.buttonState, 0x1000), 0.0);

				//  TOUCHPAD joystick/click
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kTrackpadOrJoystickClick], ButtonMaskInt(data.buttonState, 0x8000), 0.0);

				//PRIMARY_TOUCHPAD
			   //modify by dzhuang
				//delete //vr::VRDriverInput()->UpdateBooleanComponent(handles[3], ButtonMaskInt(data.isTouching, 0x0), 0.0);
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kTrackpadTouch], data.isTouching != 0 ? true : false, 0.0);

				//delete// vr::VRDriverInput()->UpdateScalarComponent(handles[5], data.analog2D[0].x, 0.0);
				//delete //vr::VRDriverInput()->UpdateScalarComponent(handles[6], data.analog2D[0].y, 0.0);
				vr::VRDriverInput()->UpdateScalarComponent(handles[kTrackpadx], -data.analog2D[0].x, 0.0);
				vr::VRDriverInput()->UpdateScalarComponent(handles[kTrackpady], -data.analog2D[0].y, 0.0);
				//end
			}
			else if (gConfigReader.GetControllerType() == 1 || gConfigReader.GetControllerType() == 0 || gConfigReader.GetControllerType() == 3)
			{
				//SYSTEM_BUTTON
			
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kSystemClick], ButtonMaskInt(data.buttonState, 0x200), 0.0);
				
				
				{
					
					vr::VRDriverInput()->UpdateScalarComponent(handles[kGripValue], data.analog1D[1], 0.0);
					vr::VRDriverInput()->UpdateBooleanComponent(handles[kGripClick], abs(data.analog1D[1]) > 0.99 ? 1 : 0, 0.0);
				}


				//PRIMARY_joystick_button  click
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kTrackpadOrJoystickClick], ButtonMaskInt(data.buttonState, 0x8000), 0.0);
				
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kJoystickTouch], data.isTouching != 0 ? true : false, 0.0);

				vr::VRDriverInput()->UpdateScalarComponent(handles[kJoystickx], -data.analog2D[0].x, 0.0);
				vr::VRDriverInput()->UpdateScalarComponent(handles[kJoysticky], -data.analog2D[0].y, 0.0);
				//end
				if (mControllerIndex == 0)
				{
					vr::VRDriverInput()->UpdateBooleanComponent(handles[kXClick], ButtonMaskInt(data.buttonState, 0x01), 0.0);
					vr::VRDriverInput()->UpdateBooleanComponent(handles[kXTouch], ButtonMaskInt(data.buttonState, 0x04), 0.0);

					vr::VRDriverInput()->UpdateBooleanComponent(handles[kYClick], ButtonMaskInt(data.buttonState, 0x02), 0.0);
					vr::VRDriverInput()->UpdateBooleanComponent(handles[kYTouch], ButtonMaskInt(data.buttonState, 0x08), 0.0);
				}
				else
				{
					vr::VRDriverInput()->UpdateBooleanComponent(handles[kAClick], ButtonMaskInt(data.buttonState, 0x01), 0.0);
					vr::VRDriverInput()->UpdateBooleanComponent(handles[kATouch], ButtonMaskInt(data.buttonState, 0x04), 0.0);

					vr::VRDriverInput()->UpdateBooleanComponent(handles[kBClick], ButtonMaskInt(data.buttonState, 0x02), 0.0);
					vr::VRDriverInput()->UpdateBooleanComponent(handles[kBTouch], ButtonMaskInt(data.buttonState, 0x08), 0.0);
				}

				/*	if (data.analog1D[0] > 0.001|| ButtonMaskInt(data.buttonState, 0x020))
					{
						vr::VRDriverInput()->UpdateBooleanComponent(handles[kTriggerTouch], true, 0.0);
					}
					else
					{
						vr::VRDriverInput()->UpdateBooleanComponent(handles[kTriggerTouch], false, 0.0);

					}*/
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kTriggerTouch], ButtonMaskInt(data.buttonState, 0x20), 0.0);
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kGripTouch],( abs(data.analog1D[1]) > 0.0001 ? 1 : 0)|| ButtonMaskInt(data.buttonState, 0x40), 0.0);
			}

		}
		//UpdateSkeleton(data);
	}
	
}

void ControllerDriver::UpdateButtonState(HidType::ButtonStateGather button_state) 
{
	if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid)
	{
		bool joystick_is_touching = false;

		uint32_t trigger_value = button_state.analog_trigger_grip[0] & 0xff;
		uint32_t grip_value = button_state.analog_trigger_grip[1] & 0xff;

		if (button_state.joystick.x != 128 || button_state.joystick.y != 128)
		{
			joystick_is_touching = true;
		}
		int device_joystic_y = button_state.joystick.y > 128 ? button_state.joystick.y + 1 : button_state.joystick.y;
		float joystick_x = (float)(device_joystic_y - 128) / 128.000000;
		int device_joystic_x = button_state.joystick.x > 128 ? button_state.joystick.x + 1 : button_state.joystick.x;
		float joystick_y = (float)(device_joystic_x - 128) / 128.000000;
		//end

		vr::VRDriverInput()->UpdateBooleanComponent(handles[kApplicationMenu], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kMenuButtonClick), 0.0);

		vr::VRDriverInput()->UpdateBooleanComponent(handles[kTriggerTouch], trigger_value > 0 ? 1 : 0, 0.0);
		vr::VRDriverInput()->UpdateScalarComponent(handles[kTriggerValue], (float)trigger_value / 255.000, 0);
		vr::VRDriverInput()->UpdateBooleanComponent(handles[kTriggerClick], trigger_value == 255 ? 1 : 0, 0.0);

		if (ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kTriggerTouch))
		{
			DriverLog("kTriggerTouch\n");
		}

		if (gConfigReader.GetControllerType() == 2|| gConfigReader.GetControllerType() == 4)
		{
			vr::VRDriverInput()->UpdateBooleanComponent(handles[kTrackpadOrJoystickClick], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kJoystickClick), 0.0);
			//vr::VRDriverInput()->UpdateBooleanComponent(handles[kTrackpadTouch], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kJoystickTouch), 0.0);
			vr::VRDriverInput()->UpdateBooleanComponent(handles[kTrackpadTouch], joystick_is_touching, 0.0);
			vr::VRDriverInput()->UpdateScalarComponent(handles[kTrackpadx], joystick_x, 0.0);
			vr::VRDriverInput()->UpdateScalarComponent(handles[kTrackpady], joystick_y, 0.0);
			vr::VRDriverInput()->UpdateBooleanComponent(handles[kGripClick], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kGripTouch), 0.0);

		}
		else if ((gConfigReader.GetControllerType() == 0) || (gConfigReader.GetControllerType() == 1)|| gConfigReader.GetControllerType() == 3)
		{
			vr::VRDriverInput()->UpdateBooleanComponent(handles[kTrackpadOrJoystickClick], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kJoystickClick), 0.0);
			//vr::VRDriverInput()->UpdateBooleanComponent(handles[kTrackpadTouch], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kJoystickTouch), 0.0);
			vr::VRDriverInput()->UpdateBooleanComponent(handles[kJoystickTouch], joystick_is_touching, 0.0);
			vr::VRDriverInput()->UpdateScalarComponent(handles[kJoystickx], joystick_x, 0.0);
			vr::VRDriverInput()->UpdateScalarComponent(handles[kJoysticky], joystick_y, 0.0);

			vr::VRDriverInput()->UpdateBooleanComponent(handles[kGripTouch], grip_value > 0 ? 1 : 0, 0.0);
			vr::VRDriverInput()->UpdateScalarComponent(handles[kGripValue], float(grip_value / 255.0f), 0.0);
			vr::VRDriverInput()->UpdateBooleanComponent(handles[kGripClick], grip_value == 255 ? 1 : 0, 0.0);
			if ((ControllerIndex)mControllerIndex == ControllerIndex::kLeftController)
			{

				vr::VRDriverInput()->UpdateBooleanComponent(handles[kXClick], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kXAClick), 0.0);
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kXTouch], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kXATouch), 0.0);

				vr::VRDriverInput()->UpdateBooleanComponent(handles[kYClick], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kYBClick), 0.0);
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kYTouch], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kYBTouch), 0.0);
			}
			else
			{
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kAClick], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kXAClick), 0.0);
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kATouch], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kXATouch), 0.0);
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kBClick], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kYBClick), 0.0);
				vr::VRDriverInput()->UpdateBooleanComponent(handles[kBTouch], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kYBTouch), 0.0);

			}
		}

		vr::VRDriverInput()->UpdateBooleanComponent(handles[kSystemClick], ButtonMask(button_state.button_state, (short)HidType::ControllerButtonState::kSystemButtonClick), 0.0);
	}
}

void ControllerDriver::UpdateSkeleton(RVR::RVRControllerData data)
{
	uint64_t currentThumbTouch = (data.buttonState & (int)ButtonStateValue::kXATouchState)|
		(data.buttonState & (int)ButtonStateValue::kYBTouchState)|
		(data.buttonState & (int)ButtonStateValue::KJoystickTouchState);
	 
		 
	if (m_lastThumbTouch != currentThumbTouch) {
		m_thumbAnimationProgress += 1.f / ANIMATION_FRAME_COUNT;
		if (m_thumbAnimationProgress > 1.f) {
			m_thumbAnimationProgress = 0;
			m_lastThumbTouch = currentThumbTouch;
		}
	}
	else {
		m_thumbAnimationProgress = 0;
	}

	uint64_t currentIndexTouch = (data.buttonState & (int)ButtonStateValue::kTriggerTouchState);
	if (m_lastIndexTouch != currentIndexTouch) {
		m_indexAnimationProgress += 1.f / ANIMATION_FRAME_COUNT;
		if (m_indexAnimationProgress > 1.f) {
			m_indexAnimationProgress = 0;
			m_lastIndexTouch = currentIndexTouch;
		}
	}
	else {
		m_indexAnimationProgress = 0;
	}

	uint64_t lastPoseTouch = m_lastThumbTouch + m_lastIndexTouch;

	vr::VRBoneTransform_t boneTransforms[SKELETON_BONE_COUNT];

	// Perform whatever logic is necessary to convert your device's input into a skeletal
	// pose, first to create a pose "With Controller", that is as close to the pose of the
	// user's real hand as possible
	GetBoneTransform(true,
		mControllerIndex==0,
		m_thumbAnimationProgress,
		m_indexAnimationProgress,
		lastPoseTouch,
		data,
		boneTransforms);

	// Then update the WithController pose on the component with those transforms
	vr::EVRInputError err = vr::VRDriverInput()->UpdateSkeletonComponent(
		m_compSkeleton,
		vr::VRSkeletalMotionRange_WithController,
		boneTransforms,
		SKELETON_BONE_COUNT);
	if (err != vr::VRInputError_None) {
		// Handle failure case
		DriverLog("UpdateSkeletonComponentfailed.  Error: %i\n", err);
	}

	GetBoneTransform(false,
		mControllerIndex == 0,
		m_thumbAnimationProgress,
		m_indexAnimationProgress,
		lastPoseTouch,
		data,
		boneTransforms);

	// Then update the WithoutController pose on the component
	err = vr::VRDriverInput()->UpdateSkeletonComponent(
		m_compSkeleton,
		vr::VRSkeletalMotionRange_WithoutController,
		boneTransforms,
		SKELETON_BONE_COUNT);
	if (err != vr::VRInputError_None) {
		// Handle failure case
		DriverLog("UpdateSkeletonComponentfailed.  Error: %i\n", err);
	}
}
void ControllerDriver::AdjustControllerPose(RVR::RVRControllerData& pose) 
{
	double pitch = 0; 
	double yaw = 0; 
	double roll = 0; 
	if (mControllerIndex==0)
	{
		pitch = gConfigReader.GetLeftPitchValue();
		yaw = gConfigReader.GetLeftYawValue();
		roll = gConfigReader.GetLeftRollValue();
	} 
	else
	{
		pitch = gConfigReader.GetRightPitchValue();
		yaw = gConfigReader.GetRightYawValue();
		roll = gConfigReader.GetRightRollValue();
	}
	
	RVRQuaternion increment_rotation;
	EularAnglesToRotation(pitch, yaw,roll, increment_rotation);
	ChangeRotation(pose, increment_rotation);
	RVRVector3 increment_pose = { 0 };
	if (mControllerIndex==0)
	{
		increment_pose={ gConfigReader.GetLeftAddXValue(), gConfigReader.GetLeftAddYValue(), gConfigReader.GetLeftAddZValue() };
	} 
	else
	{
		increment_pose = { gConfigReader.GetRightAddXValue(), gConfigReader.GetRightAddYValue(), gConfigReader.GetRightAddZValue() };
	}
	
	ChangePosition(pose.rotation, pose.position, increment_pose);
}


void ControllerDriver::BaseAdjustControllerPose(RVR::RVRControllerData& pose,bool dp, std::string hmd_type)
{
	double pitch = 0;
	double yaw = 0;
	double roll = 0;
	if (dp)
	{
		if (mControllerIndex == 0)
		{
			pitch = 23;
			yaw = 6;
			roll = -5;
		}
		else
		{
			pitch = 23;
			yaw = -6;
			roll = 5;
		}

	}
	else
	{
		if (hmd_type.compare("neo3")==0)
		{
			if (mControllerIndex == 0)
			{
				pitch = 23;
				yaw = 6;
				roll = -5;
			}
			else
			{
				pitch = 23;
				yaw = -6;
				roll = 5;
			}

		} 
		else if(hmd_type.compare("phoenix") == 0)
		{
			if (mControllerIndex == 0)
			{
				pitch = 0;
				yaw = 0;
				roll =0;
			}
			else
			{
				pitch =0;
				yaw = 0;
				roll = 0;
			}

		}
		
	}
	
	RVRQuaternion increment_rotation;
	EularAnglesToRotation(pitch, yaw, roll, increment_rotation);
	ChangeRotation(pose, increment_rotation);
	RVRVector3 increment_pose = { 0 };
	if (dp)
	{
		if (mControllerIndex == 0)
		{
			increment_pose = { -0.003 , -0.03, -0.045 };
		}
		else
		{
			increment_pose = { 0.003, -0.03, -0.045 };
		}

	}
	else
	{
		if (hmd_type.compare("neo3") == 0)
		{
			if (mControllerIndex == 0)
			{
				increment_pose = { 0, -0.02, -0.06 };
			}
			else
			{
				increment_pose = { 0, -0.02, -0.06 };
			}
		}
		else if (hmd_type.compare("phoenix") == 0)
		{
			if (mControllerIndex == 0)
			{
				increment_pose = { 0.008, 0.017, -0.007 };
			}
			else
			{
				increment_pose = { -0.008, 0.017, -0.007 };
			}
		}

	}
	
	ChangePosition(pose.rotation, pose.position, increment_pose);
}



void ControllerDriver::ToOculusAdjustControllerPose(RVR::RVRControllerData& pose, bool dp, std::string hmd_type)
{
	double pitch = 0;
	double yaw = 0;
	double roll = 0;
	if (dp)
	{
		if (mControllerIndex == 0)
		{
			pitch = 24;
			yaw = 1;
			roll = 4;
		}
		else
		{
			pitch = 24;
			yaw = -1;
			roll = -4;
		}

	}
	else
	{
		if (hmd_type.compare("neo3") == 0)
		{
			if (mControllerIndex == 0)
			{
				pitch = 24;
				yaw = 1;
				roll = 4;
			}
			else
			{
				pitch = 24;
				yaw = -1;
				roll = -4;
			}
		}
		else if (hmd_type.compare("phoenix") == 0)
		{
			if (mControllerIndex == 0)
			{
				pitch = 20;
				yaw = 2;
				roll = 2;
			}
			else
			{
				pitch = 20;
				yaw = -2;
				roll = -2;
			}
		}

	}

	RVRQuaternion increment_rotation;
	EularAnglesToRotation(pitch, yaw, roll, increment_rotation);
	ChangeRotation(pose, increment_rotation);
	RVRVector3 increment_pose = { 0 };
	if (dp)
	{
		if (mControllerIndex == 0)
		{
			increment_pose = { 0.002 , -0.02, -0.085 };
		}
		else
		{
			increment_pose = { -0.002, -0.02, -0.085 };
		}

	}
	else
	{
		if (hmd_type.compare("neo3") == 0)
		{
			if (mControllerIndex == 0)
			{
				increment_pose = { 0.005, -0.01, -0.1 };
			}
			else
			{
				increment_pose = { -0.005, -0.01, -0.1 };
			}
		}
		else if (hmd_type.compare("phoenix") == 0)
		{
			if (mControllerIndex == 0)
			{
				increment_pose = { 0, 0.034, -0.06 };
			}
			else
			{
				increment_pose = { 0, 0.034, -0.06 };
			}
		}

	}

	ChangePosition(pose.rotation, pose.position, increment_pose);
}


void ControllerDriver::ToHtcAdjustControllerPose(RVR::RVRControllerData& pose, bool dp, std::string hmd_type)
{
	double pitch = 0;
	double yaw = 0;
	double roll = 0;
	if (dp)
	{
		if (mControllerIndex == 0)
		{
			pitch = 35;
			yaw = 0;
			roll = 0;
		}
		else
		{
			pitch = 35;
			yaw = 0;
			roll =0;
		}

	}
	else
	{
		if (hmd_type.compare("neo3") == 0)
		{
			if (mControllerIndex == 0)
			{
				pitch = 35;
				yaw = 0;
				roll = 0;
			}
			else
			{
				pitch = 35;
				yaw = 0;
				roll = 0;
			}
		}
		else if (hmd_type.compare("phoenix") == 0)
		{
			if (mControllerIndex == 0)
			{
				pitch = 20;
				yaw = 2;
				roll = 2;
			}
			else
			{
				pitch = 20;
				yaw = -2;
				roll = -2;
			}
		}

	}

	RVRQuaternion increment_rotation;
	EularAnglesToRotation(pitch, yaw, roll, increment_rotation);
	ChangeRotation(pose, increment_rotation);
	RVRVector3 increment_pose = { 0 };
	if (dp)
	{
		if (mControllerIndex == 0)
		{
			increment_pose = { 0.002 , 0.01, -0.085 };
		}
		else
		{
			increment_pose = { -0.002, 0.01, -0.085 };
		}

	}
	else
	{
		if (hmd_type.compare("neo3") == 0)
		{
			if (mControllerIndex == 0)
			{
				increment_pose = { 0.005, 0.02, -0.1 };
			}
			else
			{
				increment_pose = { -0.005, 0.02, -0.1 };
			}
		}
		else if (hmd_type.compare("phoenix") == 0)
		{
			if (mControllerIndex == 0)
			{
				increment_pose = { 0, 0.034, -0.08 };
			}
			else
			{
				increment_pose = { 0, 0.034, -0.08 };
			}
		}

	}

	ChangePosition(pose.rotation, pose.position, increment_pose);
}


uint64_t ControllerDriver::GetHapticsHandle()
{
    return handles[kHaptic];
}


void GetThumbBoneTransform(bool withController,
	bool isLeftHand,
	uint32_t buttons,
	vr::VRBoneTransform_t outBoneTransform[]) {
	if (isLeftHand) {
		if (ButtonMaskInt(buttons, (int)ButtonStateValue::kYBTouchState)) {
			// y touch
			if (withController) {
				outBoneTransform[2] = { {-0.017303f, 0.032567f, 0.025281f, 1.f},
									   {0.317609f, 0.528344f, 0.213134f, 0.757991f} };
				outBoneTransform[3] = { {0.040406f, 0.000000f, -0.000000f, 1.f},
									   {0.991742f, 0.085317f, 0.019416f, 0.093765f} };
				outBoneTransform[4] = { {0.032517f, -0.000000f, 0.000000f, 1.f},
									   {0.959385f, -0.012202f, -0.031055f, 0.280120f} };
			}
			else {
				outBoneTransform[2] = { {-0.016426f, 0.030866f, 0.025118f, 1.f},
									   {0.403850f, 0.595704f, 0.082451f, 0.689380f} };
				outBoneTransform[3] = { {0.040406f, 0.000000f, -0.000000f, 1.f},
									   {0.989655f, -0.090426f, 0.028457f, 0.107691f} };
				outBoneTransform[4] = { {0.032517f, 0.000000f, 0.000000f, 1.f},
									   {0.988590f, 0.143978f, 0.041520f, 0.015363f} };
			}
		}
		else if (ButtonMaskInt(buttons, (int)ButtonStateValue::kXATouchState)) {
			// x touch
			if (withController) {
				outBoneTransform[2] = { {-0.017625f, 0.031098f, 0.022755f, 1},
									   {0.388513f, 0.527438f, 0.249444f, 0.713193f} };
				outBoneTransform[3] = { {0.040406f, 0.000000f, -0.000000f, 1},
									   {0.978341f, 0.085924f, 0.037765f, 0.184501f} };
				outBoneTransform[4] = { {0.032517f, -0.000000f, 0.000000f, 1},
									   {0.894037f, -0.043820f, -0.048328f, 0.443217f} };
			}
			else {
				outBoneTransform[2] = { {-0.017288f, 0.027151f, 0.021465f, 1},
									   {0.502777f, 0.569978f, 0.147197f, 0.632988f} };
				outBoneTransform[3] = { {0.040406f, 0.000000f, -0.000000f, 1},
									   {0.970397f, -0.048119f, 0.023261f, 0.235527f} };
				outBoneTransform[4] = { {0.032517f, 0.000000f, 0.000000f, 1},
									   {0.794064f, 0.084451f, -0.037468f, 0.600772f} };
			}
		}
		else if (ButtonMaskInt(buttons, (int)ButtonStateValue::KJoystickTouchState)) {
			// joy touch
			if (withController) {
				outBoneTransform[2] = { {-0.017914f, 0.029178f, 0.025298f, 1},
									   {0.455126f, 0.591760f, 0.168152f, 0.643743f} };
				outBoneTransform[3] = { {0.040406f, 0.000000f, -0.000000f, 1},
									   {0.969878f, 0.084444f, 0.045679f, 0.223873f} };
				outBoneTransform[4] = { {0.032517f, -0.000000f, 0.000000f, 1},
									   {0.991257f, 0.014384f, -0.005602f, 0.131040f} };
			}
			else {
				outBoneTransform[2] = { {-0.017914f, 0.029178f, 0.025298f, 1},
									   {0.455126f, 0.591760f, 0.168152f, 0.643743f} };
				outBoneTransform[3] = { {0.040406f, 0.000000f, -0.000000f, 1},
									   {0.969878f, 0.084444f, 0.045679f, 0.223873f} };
				outBoneTransform[4] = { {0.032517f, -0.000000f, 0.000000f, 1},
									   {0.991257f, 0.014384f, -0.005602f, 0.131040f} };
			}
		}
		else {
			// no touch
			outBoneTransform[2] = { {-0.012083f, 0.028070f, 0.025050f, 1},
								   {0.464112f, 0.567418f, 0.272106f, 0.623374f} };
			outBoneTransform[3] = { {0.040406f, 0.000000f, -0.000000f, 1},
								   {0.994838f, 0.082939f, 0.019454f, 0.055130f} };
			outBoneTransform[4] = { {0.032517f, 0.000000f, 0.000000f, 1},
								   {0.974793f, -0.003213f, 0.021867f, -0.222015f} };
		}

		outBoneTransform[5] = { {0.030464f, -0.000000f, -0.000000f, 1},
							   {1.000000f, -0.000000f, 0.000000f, 0.000000f} };
	}
	else {
		if (ButtonMaskInt(buttons, (int)ButtonStateValue::kYBTouchState)) {
			// b touch
			if (withController) {
				outBoneTransform[2] = { {0.017303f, 0.032567f, 0.025281f, 1},
									   {0.528344f, -0.317609f, 0.757991f, -0.213134f} };
				outBoneTransform[3] = { {-0.040406f, -0.000000f, 0.000000f, 1},
									   {0.991742f, 0.085317f, 0.019416f, 0.093765f} };
				outBoneTransform[4] = { {-0.032517f, 0.000000f, -0.000000f, 1},
									   {0.959385f, -0.012202f, -0.031055f, 0.280120f} };
			}
			else {
				outBoneTransform[2] = { {0.016426f, 0.030866f, 0.025118f, 1},
									   {0.595704f, -0.403850f, 0.689380f, -0.082451f} };
				outBoneTransform[3] = { {-0.040406f, -0.000000f, 0.000000f, 1},
									   {0.989655f, -0.090426f, 0.028457f, 0.107691f} };
				outBoneTransform[4] = { {-0.032517f, -0.000000f, -0.000000f, 1},
									   {0.988590f, 0.143978f, 0.041520f, 0.015363f} };
			}
		}
		else if (ButtonMaskInt(buttons, (int)ButtonStateValue::kXATouchState)) {
			// a touch
			if (withController) {
				outBoneTransform[2] = { {0.017625f, 0.031098f, 0.022755f, 1},
									   {0.527438f, -0.388513f, 0.713193f, -0.249444f} };
				outBoneTransform[3] = { {-0.040406f, -0.000000f, 0.000000f, 1},
									   {0.978341f, 0.085924f, 0.037765f, 0.184501f} };
				outBoneTransform[4] = { {-0.032517f, 0.000000f, -0.000000f, 1},
									   {0.894037f, -0.043820f, -0.048328f, 0.443217f} };
			}
			else {
				outBoneTransform[2] = { {0.017288f, 0.027151f, 0.021465f, 1},
									   {0.569978f, -0.502777f, 0.632988f, -0.147197f} };
				outBoneTransform[3] = { {-0.040406f, -0.000000f, 0.000000f, 1},
									   {0.970397f, -0.048119f, 0.023261f, 0.235527f} };
				outBoneTransform[4] = { {-0.032517f, -0.000000f, -0.000000f, 1},
									   {0.794064f, 0.084451f, -0.037468f, 0.600772f} };
			}
		}
		else if (ButtonMaskInt(buttons, (int)ButtonStateValue::KJoystickTouchState)) {
			// joy touch
			if (withController) {
				outBoneTransform[2] = { {0.017914f, 0.029178f, 0.025298f, 1},
									   {0.591760f, -0.455126f, 0.643743f, -0.168152f} };
				outBoneTransform[3] = { {-0.040406f, -0.000000f, 0.000000f, 1},
									   {0.969878f, 0.084444f, 0.045679f, 0.223873f} };
				outBoneTransform[4] = { {-0.032517f, 0.000000f, -0.000000f, 1},
									   {0.991257f, 0.014384f, -0.005602f, 0.131040f} };
			}
			else {
				outBoneTransform[2] = { {0.017914f, 0.029178f, 0.025298f, 1},
									   {0.591760f, -0.455126f, 0.643743f, -0.168152f} };
				outBoneTransform[3] = { {-0.040406f, -0.000000f, 0.000000f, 1},
									   {0.969878f, 0.084444f, 0.045679f, 0.223873f} };
				outBoneTransform[4] = { {-0.032517f, 0.000000f, -0.000000f, 1},
									   {0.991257f, 0.014384f, -0.005602f, 0.131040f} };
			}
		}
		else {
			// no touch
			outBoneTransform[2] = { {0.012330f, 0.028661f, 0.025049f, 1},
								   {0.571059f, -0.451277f, 0.630056f, -0.270685f} };
			outBoneTransform[3] = { {-0.040406f, -0.000000f, 0.000000f, 1},
								   {0.994565f, 0.078280f, 0.018282f, 0.066177f} };
			outBoneTransform[4] = { {-0.032517f, -0.000000f, -0.000000f, 1},
								   {0.977658f, -0.003039f, 0.020722f, -0.209156f} };
		}

		outBoneTransform[5] = { {-0.030464f, 0.000000f, 0.000000f, 1},
							   {1.000000f, -0.000000f, 0.000000f, 0.000000f} };
	}
}

void GetTriggerBoneTransform(bool withController,
	bool isLeftHand,
	uint32_t buttons,
	vr::VRBoneTransform_t outBoneTransform[]) {
	if (ButtonMaskInt(buttons, (int)ButtonStateValue::kTriggerClickState)) {
		// click
		if (withController) {
			if (isLeftHand) {
				outBoneTransform[6] = { {-0.003925f, 0.027171f, 0.014640f, 1},
									   {0.666448f, 0.430031f, -0.455947f, 0.403772f} };
				outBoneTransform[7] = { {0.076015f, -0.005124f, 0.000239f, 1},
									   {-0.956011f, -0.000025f, 0.158355f, -0.246913f} };
				outBoneTransform[8] = { {0.043930f, -0.000000f, -0.000000f, 1},
									   {-0.944138f, -0.043351f, 0.014947f, -0.326345f} };
				outBoneTransform[9] = { {0.028695f, 0.000000f, 0.000000f, 1},
									   {-0.912149f, 0.003626f, 0.039888f, -0.407898f} };
				outBoneTransform[10] = { {0.022821f, 0.000000f, -0.000000f, 1},
										{1.000000f, -0.000000f, -0.000000f, 0.000000f} };
				outBoneTransform[11] = { {0.002177f, 0.007120f, 0.016319f, 1},
										{0.529359f, 0.540512f, -0.463783f, 0.461011f} };
				outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
										{0.847397f, -0.257141f, -0.139135f, 0.443213f} };
				outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
										{0.874907f, 0.009875f, 0.026584f, 0.483460f} };
				outBoneTransform[14] = { {0.033266f, -0.000000f, 0.000000f, 1},
										{0.894578f, -0.036774f, -0.050597f, 0.442513f} };
				outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
										{0.999195f, -0.000000f, 0.000000f, 0.040126f} };
				outBoneTransform[16] = { {0.000513f, -0.006545f, 0.016348f, 1},
										{0.500244f, 0.530784f, -0.516215f, 0.448939f} };
				outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
										{0.831617f, -0.242931f, -0.139695f, 0.479461f} };
				outBoneTransform[18] = { {0.040697f, 0.000000f, 0.000000f, 1},
										{0.769163f, -0.001746f, 0.001363f, 0.639049f} };
				outBoneTransform[19] = { {0.028747f, -0.000000f, -0.000000f, 1},
										{0.968615f, -0.064538f, -0.046586f, 0.235477f} };
				outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
										{1.000000f, 0.000000f, -0.000000f, -0.000000f} };
				outBoneTransform[21] = { {-0.002478f, -0.018981f, 0.015214f, 1},
										{0.474671f, 0.434670f, -0.653212f, 0.398827f} };
				outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
										{0.798788f, -0.199577f, -0.094418f, 0.559636f} };
				outBoneTransform[23] = { {0.030220f, 0.000002f, -0.000000f, 1},
										{0.853087f, 0.001644f, -0.000913f, 0.521765f} };
				outBoneTransform[24] = { {0.018187f, -0.000002f, 0.000000f, 1},
										{0.974249f, 0.052491f, 0.003591f, 0.219249f} };
				outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
										{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
				outBoneTransform[26] = { {0.006629f, 0.026690f, 0.061870f, 1},
										{0.805084f, -0.018369f, 0.584788f, -0.097597f} };
				outBoneTransform[27] = { {-0.007882f, -0.040478f, 0.039337f, 1},
										{-0.322494f, 0.932092f, 0.121861f, 0.111140f} };
				outBoneTransform[28] = { {0.017136f, -0.032633f, 0.080682f, 1},
										{-0.169466f, 0.800083f, 0.571006f, 0.071415f} };
				outBoneTransform[29] = { {0.011144f, -0.028727f, 0.108366f, 1},
										{-0.076328f, 0.788280f, 0.605097f, 0.081527f} };
				outBoneTransform[30] = { {0.011333f, -0.026044f, 0.128585f, 1},
										{-0.144791f, 0.737451f, 0.656958f, -0.060069f} };
			}
			else {
				outBoneTransform[6] = { {-0.003925f, 0.027171f, 0.014640f, 1},
									   {0.666448f, 0.430031f, -0.455947f, 0.403772f} };
				outBoneTransform[7] = { {0.076015f, -0.005124f, 0.000239f, 1},
									   {-0.956011f, -0.000025f, 0.158355f, -0.246913f} };
				outBoneTransform[8] = { {0.043930f, -0.000000f, -0.000000f, 1},
									   {-0.944138f, -0.043351f, 0.014947f, -0.326345f} };
				outBoneTransform[9] = { {0.028695f, 0.000000f, 0.000000f, 1},
									   {-0.912149f, 0.003626f, 0.039888f, -0.407898f} };
				outBoneTransform[10] = { {0.022821f, 0.000000f, -0.000000f, 1},
										{1.000000f, -0.000000f, -0.000000f, 0.000000f} };
				outBoneTransform[11] = { {0.002177f, 0.007120f, 0.016319f, 1},
										{0.529359f, 0.540512f, -0.463783f, 0.461011f} };
				outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
										{0.847397f, -0.257141f, -0.139135f, 0.443213f} };
				outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
										{0.874907f, 0.009875f, 0.026584f, 0.483460f} };
				outBoneTransform[14] = { {0.033266f, -0.000000f, 0.000000f, 1},
										{0.894578f, -0.036774f, -0.050597f, 0.442513f} };
				outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
										{0.999195f, -0.000000f, 0.000000f, 0.040126f} };
				outBoneTransform[16] = { {0.000513f, -0.006545f, 0.016348f, 1},
										{0.500244f, 0.530784f, -0.516215f, 0.448939f} };
				outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
										{0.831617f, -0.242931f, -0.139695f, 0.479461f} };
				outBoneTransform[18] = { {0.040697f, 0.000000f, 0.000000f, 1},
										{0.769163f, -0.001746f, 0.001363f, 0.639049f} };
				outBoneTransform[19] = { {0.028747f, -0.000000f, -0.000000f, 1},
										{0.968615f, -0.064538f, -0.046586f, 0.235477f} };
				outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
										{1.000000f, 0.000000f, -0.000000f, -0.000000f} };
				outBoneTransform[21] = { {-0.002478f, -0.018981f, 0.015214f, 1},
										{0.474671f, 0.434670f, -0.653212f, 0.398827f} };
				outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
										{0.798788f, -0.199577f, -0.094418f, 0.559636f} };
				outBoneTransform[23] = { {0.030220f, 0.000002f, -0.000000f, 1},
										{0.853087f, 0.001644f, -0.000913f, 0.521765f} };
				outBoneTransform[24] = { {0.018187f, -0.000002f, 0.000000f, 1},
										{0.974249f, 0.052491f, 0.003591f, 0.219249f} };
				outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
										{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
				outBoneTransform[26] = { {0.006629f, 0.026690f, 0.061870f, 1},
										{0.805084f, -0.018369f, 0.584788f, -0.097597f} };
				outBoneTransform[27] = { {-0.007882f, -0.040478f, 0.039337f, 1},
										{-0.322494f, 0.932092f, 0.121861f, 0.111140f} };
				outBoneTransform[28] = { {0.017136f, -0.032633f, 0.080682f, 1},
										{-0.169466f, 0.800083f, 0.571006f, 0.071415f} };
				outBoneTransform[29] = { {0.011144f, -0.028727f, 0.108366f, 1},
										{-0.076328f, 0.788280f, 0.605097f, 0.081527f} };
				outBoneTransform[30] = { {0.011333f, -0.026044f, 0.128585f, 1},
										{-0.144791f, 0.737451f, 0.656958f, -0.060069f} };
			}
		}
		else {
			if (isLeftHand) {
				outBoneTransform[6] = { {0.003802f, 0.021514f, 0.012803f, 1},
									   {0.617314f, 0.395175f, -0.510874f, 0.449185f} };
				outBoneTransform[7] = { {0.074204f, -0.005002f, 0.000234f, 1},
									   {0.737291f, -0.032006f, -0.115013f, 0.664944f} };
				outBoneTransform[8] = { {0.043287f, -0.000000f, -0.000000f, 1},
									   {0.611381f, 0.003287f, 0.003823f, 0.791320f} };
				outBoneTransform[9] = { {0.028275f, 0.000000f, 0.000000f, 1},
									   {0.745389f, -0.000684f, -0.000945f, 0.666629f} };
				outBoneTransform[10] = { {0.022821f, 0.000000f, -0.000000f, 1},
										{1.000000f, 0.000000f, -0.000000f, 0.000000f} };
				outBoneTransform[11] = { {0.004885f, 0.006885f, 0.016480f, 1},
										{0.522678f, 0.527374f, -0.469333f, 0.477923f} };
				outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
										{0.826071f, -0.121321f, 0.017267f, 0.550082f} };
				outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
										{0.956676f, 0.013210f, 0.009330f, 0.290704f} };
				outBoneTransform[14] = { {0.033266f, 0.000000f, 0.000000f, 1},
										{0.979740f, -0.001605f, -0.019412f, 0.199323f} };
				outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
										{0.999195f, 0.000000f, 0.000000f, 0.040126f} };
				outBoneTransform[16] = { {0.001696f, -0.006648f, 0.016418f, 1},
										{0.509620f, 0.540794f, -0.504891f, 0.439220f} };
				outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
										{0.955009f, -0.065344f, -0.063228f, 0.282294f} };
				outBoneTransform[18] = { {0.040577f, 0.000000f, 0.000000f, 1},
										{0.953823f, -0.000972f, 0.000697f, 0.300366f} };
				outBoneTransform[19] = { {0.028698f, -0.000000f, -0.000000f, 1},
										{0.977627f, -0.001163f, -0.011433f, 0.210033f} };
				outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
										{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
				outBoneTransform[21] = { {-0.001792f, -0.019041f, 0.015254f, 1},
										{0.518602f, 0.511152f, -0.596086f, 0.338315f} };
				outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
										{0.978584f, -0.045398f, -0.103083f, 0.172297f} };
				outBoneTransform[23] = { {0.030154f, 0.000000f, 0.000000f, 1},
										{0.970479f, -0.000068f, -0.002025f, 0.241175f} };
				outBoneTransform[24] = { {0.018187f, 0.000000f, 0.000000f, 1},
										{0.997053f, -0.000687f, -0.052009f, -0.056395f} };
				outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
										{1.000000f, -0.000000f, -0.000000f, -0.000000f} };
				outBoneTransform[26] = { {-0.005193f, 0.054191f, 0.060030f, 1},
										{0.747374f, 0.182388f, 0.599615f, 0.220518f} };
				outBoneTransform[27] = { {0.000171f, 0.016473f, 0.096515f, 1},
										{-0.006456f, 0.022747f, -0.932927f, -0.359287f} };
				outBoneTransform[28] = { {-0.038019f, -0.074839f, 0.046941f, 1},
										{-0.199973f, 0.698334f, -0.635627f, -0.261380f} };
				outBoneTransform[29] = { {-0.036836f, -0.089774f, 0.081969f, 1},
										{-0.191006f, 0.756582f, -0.607429f, -0.148761f} };
				outBoneTransform[30] = { {-0.030241f, -0.086049f, 0.119881f, 1},
										{-0.019037f, 0.779368f, -0.612017f, -0.132881f} };
			}
			else {
				outBoneTransform[6] = { {-0.003802f, 0.021514f, 0.012803f, 1},
									   {0.395174f, -0.617314f, 0.449185f, 0.510874f} };
				outBoneTransform[7] = { {-0.074204f, 0.005002f, -0.000234f, 1},
									   {0.737291f, -0.032006f, -0.115013f, 0.664944f} };
				outBoneTransform[8] = { {-0.043287f, 0.000000f, 0.000000f, 1},
									   {0.611381f, 0.003287f, 0.003823f, 0.791320f} };
				outBoneTransform[9] = { {-0.028275f, -0.000000f, -0.000000f, 1},
									   {0.745389f, -0.000684f, -0.000945f, 0.666629f} };
				outBoneTransform[10] = { {-0.022821f, -0.000000f, 0.000000f, 1},
										{1.000000f, 0.000000f, -0.000000f, 0.000000f} };
				outBoneTransform[11] = { {-0.004885f, 0.006885f, 0.016480f, 1},
										{0.527233f, -0.522513f, 0.478085f, 0.469510f} };
				outBoneTransform[12] = { {-0.070953f, -0.000779f, -0.000997f, 1},
										{0.826317f, -0.120120f, 0.019005f, 0.549918f} };
				outBoneTransform[13] = { {-0.043108f, -0.000000f, -0.000000f, 1},
										{0.958363f, 0.013484f, 0.007380f, 0.285138f} };
				outBoneTransform[14] = { {-0.033266f, -0.000000f, -0.000000f, 1},
										{0.977901f, -0.001431f, -0.018078f, 0.208279f} };
				outBoneTransform[15] = { {-0.025892f, 0.000000f, -0.000000f, 1},
										{0.999195f, 0.000000f, 0.000000f, 0.040126f} };
				outBoneTransform[16] = { {-0.001696f, -0.006648f, 0.016418f, 1},
										{0.541481f, -0.508179f, 0.441001f, 0.504054f} };
				outBoneTransform[17] = { {-0.065876f, -0.001786f, -0.000693f, 1},
										{0.953780f, -0.064506f, -0.058812f, 0.287548f} };
				outBoneTransform[18] = { {-0.040577f, -0.000000f, -0.000000f, 1},
										{0.954761f, -0.000983f, 0.000698f, 0.297372f} };
				outBoneTransform[19] = { {-0.028698f, 0.000000f, 0.000000f, 1},
										{0.976924f, -0.001344f, -0.010281f, 0.213335f} };
				outBoneTransform[20] = { {-0.022430f, 0.000000f, -0.000000f, 1},
										{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
				outBoneTransform[21] = { {0.001792f, -0.019041f, 0.015254f, 1},
										{0.510569f, -0.514906f, 0.341115f, 0.598191f} };
				outBoneTransform[22] = { {-0.062878f, -0.002844f, -0.000332f, 1},
										{0.979195f, -0.043879f, -0.095103f, 0.173800f} };
				outBoneTransform[23] = { {-0.030154f, -0.000000f, -0.000000f, 1},
										{0.971387f, -0.000102f, -0.002019f, 0.237494f} };
				outBoneTransform[24] = { {-0.018187f, -0.000000f, -0.000000f, 1},
										{0.997961f, 0.000800f, -0.051911f, -0.037114f} };
				outBoneTransform[25] = { {-0.018018f, -0.000000f, 0.000000f, 1},
										{1.000000f, -0.000000f, -0.000000f, -0.000000f} };
				outBoneTransform[26] = { {0.004392f, 0.055515f, 0.060253f, 1},
										{0.745924f, 0.156756f, -0.597950f, -0.247953f} };
				outBoneTransform[27] = { {-0.000171f, 0.016473f, 0.096515f, 1},
										{-0.006456f, 0.022747f, 0.932927f, 0.359287f} };
				outBoneTransform[28] = { {0.038119f, -0.074730f, 0.046338f, 1},
										{-0.207931f, 0.699835f, 0.632631f, 0.258406f} };
				outBoneTransform[29] = { {0.035492f, -0.089519f, 0.081636f, 1},
										{-0.197555f, 0.760574f, 0.601098f, 0.145535f} };
				outBoneTransform[30] = { {0.029073f, -0.085957f, 0.119561f, 1},
										{-0.031423f, 0.791013f, 0.597190f, 0.129133f} };
			}
		}
	}
	else if (ButtonMaskInt(buttons, (int)ButtonStateValue::kTriggerTouchState)) {
		// touch
		if (withController) {
			if (isLeftHand) {
				outBoneTransform[6] = { {-0.003925f, 0.027171f, 0.014640f, 1},
									   {0.666448f, 0.430031f, -0.455947f, 0.403772f} };
				outBoneTransform[7] = { {0.074204f, -0.005002f, 0.000234f, 1},
									   {-0.951843f, 0.009717f, 0.158611f, -0.262188f} };
				outBoneTransform[8] = { {0.043930f, -0.000000f, -0.000000f, 1},
									   {-0.973045f, -0.044676f, 0.010341f, -0.226012f} };
				outBoneTransform[9] = { {0.028695f, 0.000000f, 0.000000f, 1},
									   {-0.935253f, -0.002881f, 0.023037f, -0.353217f} };
				outBoneTransform[10] = { {0.022821f, 0.000000f, -0.000000f, 1},
										{1.000000f, -0.000000f, -0.000000f, 0.000000f} };
				outBoneTransform[11] = { {0.002177f, 0.007120f, 0.016319f, 1},
										{0.529359f, 0.540512f, -0.463783f, 0.461011f} };
				outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
										{0.847397f, -0.257141f, -0.139135f, 0.443213f} };
				outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
										{0.874907f, 0.009875f, 0.026584f, 0.483460f} };
				outBoneTransform[14] = { {0.033266f, -0.000000f, 0.000000f, 1},
										{0.894578f, -0.036774f, -0.050597f, 0.442513f} };
				outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
										{0.999195f, -0.000000f, 0.000000f, 0.040126f} };
				outBoneTransform[16] = { {0.000513f, -0.006545f, 0.016348f, 1},
										{0.500244f, 0.530784f, -0.516215f, 0.448939f} };
				outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
										{0.831617f, -0.242931f, -0.139695f, 0.479461f} };
				outBoneTransform[18] = { {0.040697f, 0.000000f, 0.000000f, 1},
										{0.769163f, -0.001746f, 0.001363f, 0.639049f} };
				outBoneTransform[19] = { {0.028747f, -0.000000f, -0.000000f, 1},
										{0.968615f, -0.064538f, -0.046586f, 0.235477f} };
				outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
										{1.000000f, 0.000000f, -0.000000f, -0.000000f} };
				outBoneTransform[21] = { {-0.002478f, -0.018981f, 0.015214f, 1},
										{0.474671f, 0.434670f, -0.653212f, 0.398827f} };
				outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
										{0.798788f, -0.199577f, -0.094418f, 0.559636f} };
				outBoneTransform[23] = { {0.030220f, 0.000002f, -0.000000f, 1},
										{0.853087f, 0.001644f, -0.000913f, 0.521765f} };
				outBoneTransform[24] = { {0.018187f, -0.000002f, 0.000000f, 1},
										{0.974249f, 0.052491f, 0.003591f, 0.219249f} };
				outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
										{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
				outBoneTransform[26] = { {0.006629f, 0.026690f, 0.061870f, 1},
										{0.805084f, -0.018369f, 0.584788f, -0.097597f} };
				outBoneTransform[27] = { {-0.009005f, -0.041708f, 0.037992f, 1},
										{-0.338860f, 0.939952f, -0.007564f, 0.040082f} };
				outBoneTransform[28] = { {0.017136f, -0.032633f, 0.080682f, 1},
										{-0.169466f, 0.800083f, 0.571006f, 0.071415f} };
				outBoneTransform[29] = { {0.011144f, -0.028727f, 0.108366f, 1},
										{-0.076328f, 0.788280f, 0.605097f, 0.081527f} };
				outBoneTransform[30] = { {0.011333f, -0.026044f, 0.128585f, 1},
										{-0.144791f, 0.737451f, 0.656958f, -0.060069f} };
			}
			else {
				outBoneTransform[6] = { {-0.003925f, 0.027171f, 0.014640f, 1},
									   {0.666448f, 0.430031f, -0.455947f, 0.403772f} };
				outBoneTransform[7] = { {0.074204f, -0.005002f, 0.000234f, 1},
									   {-0.951843f, 0.009717f, 0.158611f, -0.262188f} };
				outBoneTransform[8] = { {0.043930f, -0.000000f, -0.000000f, 1},
									   {-0.973045f, -0.044676f, 0.010341f, -0.226012f} };
				outBoneTransform[9] = { {0.028695f, 0.000000f, 0.000000f, 1},
									   {-0.935253f, -0.002881f, 0.023037f, -0.353217f} };
				outBoneTransform[10] = { {0.022821f, 0.000000f, -0.000000f, 1},
										{1.000000f, -0.000000f, -0.000000f, 0.000000f} };
				outBoneTransform[11] = { {0.002177f, 0.007120f, 0.016319f, 1},
										{0.529359f, 0.540512f, -0.463783f, 0.461011f} };
				outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
										{0.847397f, -0.257141f, -0.139135f, 0.443213f} };
				outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
										{0.874907f, 0.009875f, 0.026584f, 0.483460f} };
				outBoneTransform[14] = { {0.033266f, -0.000000f, 0.000000f, 1},
										{0.894578f, -0.036774f, -0.050597f, 0.442513f} };
				outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
										{0.999195f, -0.000000f, 0.000000f, 0.040126f} };
				outBoneTransform[16] = { {0.000513f, -0.006545f, 0.016348f, 1},
										{0.500244f, 0.530784f, -0.516215f, 0.448939f} };
				outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
										{0.831617f, -0.242931f, -0.139695f, 0.479461f} };
				outBoneTransform[18] = { {0.040697f, 0.000000f, 0.000000f, 1},
										{0.769163f, -0.001746f, 0.001363f, 0.639049f} };
				outBoneTransform[19] = { {0.028747f, -0.000000f, -0.000000f, 1},
										{0.968615f, -0.064538f, -0.046586f, 0.235477f} };
				outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
										{1.000000f, 0.000000f, -0.000000f, -0.000000f} };
				outBoneTransform[21] = { {-0.002478f, -0.018981f, 0.015214f, 1},
										{0.474671f, 0.434670f, -0.653212f, 0.398827f} };
				outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
										{0.798788f, -0.199577f, -0.094418f, 0.559636f} };
				outBoneTransform[23] = { {0.030220f, 0.000002f, -0.000000f, 1},
										{0.853087f, 0.001644f, -0.000913f, 0.521765f} };
				outBoneTransform[24] = { {0.018187f, -0.000002f, 0.000000f, 1},
										{0.974249f, 0.052491f, 0.003591f, 0.219249f} };
				outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
										{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
				outBoneTransform[26] = { {0.006629f, 0.026690f, 0.061870f, 1},
										{0.805084f, -0.018369f, 0.584788f, -0.097597f} };
				outBoneTransform[27] = { {-0.009005f, -0.041708f, 0.037992f, 1},
										{-0.338860f, 0.939952f, -0.007564f, 0.040082f} };
				outBoneTransform[28] = { {0.017136f, -0.032633f, 0.080682f, 1},
										{-0.169466f, 0.800083f, 0.571006f, 0.071415f} };
				outBoneTransform[29] = { {0.011144f, -0.028727f, 0.108366f, 1},
										{-0.076328f, 0.788280f, 0.605097f, 0.081527f} };
				outBoneTransform[30] = { {0.011333f, -0.026044f, 0.128585f, 1},
										{-0.144791f, 0.737451f, 0.656958f, -0.060069f} };
			}
		}
		else {
			if (isLeftHand) {
				outBoneTransform[6] = { {0.002693f, 0.023387f, 0.013573f, 1},
									   {0.626743f, 0.404630f, -0.499840f, 0.440032f} };
				outBoneTransform[7] = { {0.074204f, -0.005002f, 0.000234f, 1},
									   {0.869067f, -0.019031f, -0.093524f, 0.485400f} };
				outBoneTransform[8] = { {0.043512f, -0.000000f, -0.000000f, 1},
									   {0.834068f, 0.020722f, 0.003930f, 0.551259f} };
				outBoneTransform[9] = { {0.028422f, 0.000000f, 0.000000f, 1},
									   {0.890556f, 0.000289f, -0.009290f, 0.454779f} };
				outBoneTransform[10] = { {0.022821f, 0.000000f, -0.000000f, 1},
										{1.000000f, 0.000000f, -0.000000f, 0.000000f} };
				outBoneTransform[11] = { {0.003937f, 0.006967f, 0.016424f, 1},
										{0.531603f, 0.532690f, -0.459598f, 0.471602f} };
				outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
										{0.906933f, -0.142169f, -0.015445f, 0.396261f} };
				outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
										{0.975787f, 0.014996f, 0.010867f, 0.217936f} };
				outBoneTransform[14] = { {0.033266f, 0.000000f, 0.000000f, 1},
										{0.992777f, -0.002096f, -0.021403f, 0.118029f} };
				outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
										{0.999195f, 0.000000f, 0.000000f, 0.040126f} };
				outBoneTransform[16] = { {0.001282f, -0.006612f, 0.016394f, 1},
										{0.513688f, 0.543325f, -0.502550f, 0.434011f} };
				outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
										{0.971280f, -0.068108f, -0.073480f, 0.215818f} };
				outBoneTransform[18] = { {0.040619f, 0.000000f, 0.000000f, 1},
										{0.976566f, -0.001379f, 0.000441f, 0.215216f} };
				outBoneTransform[19] = { {0.028715f, -0.000000f, -0.000000f, 1},
										{0.987232f, -0.000977f, -0.011919f, 0.158838f} };
				outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
										{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
				outBoneTransform[21] = { {-0.002032f, -0.019020f, 0.015240f, 1},
										{0.521784f, 0.511917f, -0.594340f, 0.335325f} };
				outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
										{0.982925f, -0.053050f, -0.108004f, 0.139206f} };
				outBoneTransform[23] = { {0.030177f, 0.000000f, 0.000000f, 1},
										{0.979798f, 0.000394f, -0.001374f, 0.199982f} };
				outBoneTransform[24] = { {0.018187f, 0.000000f, 0.000000f, 1},
										{0.997410f, -0.000172f, -0.051977f, -0.049724f} };
				outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
										{1.000000f, -0.000000f, -0.000000f, -0.000000f} };
				outBoneTransform[26] = { {-0.004857f, 0.053377f, 0.060017f, 1},
										{0.751040f, 0.174397f, 0.601473f, 0.209178f} };
				outBoneTransform[27] = { {-0.013234f, -0.004327f, 0.069740f, 1},
										{-0.119277f, 0.262590f, -0.888979f, -0.355718f} };
				outBoneTransform[28] = { {-0.037500f, -0.074514f, 0.046899f, 1},
										{-0.204942f, 0.706005f, -0.626220f, -0.259623f} };
				outBoneTransform[29] = { {-0.036251f, -0.089302f, 0.081732f, 1},
										{-0.194045f, 0.764033f, -0.596592f, -0.150590f} };
				outBoneTransform[30] = { {-0.029633f, -0.085595f, 0.119439f, 1},
										{-0.025015f, 0.787219f, -0.601140f, -0.135243f} };
			}
			else {
				outBoneTransform[6] = { {-0.002693f, 0.023387f, 0.013573f, 1},
									   {0.404698f, -0.626951f, 0.439894f, 0.499645f} };
				outBoneTransform[7] = { {-0.074204f, 0.005002f, -0.000234f, 1},
									   {0.870303f, -0.017421f, -0.092515f, 0.483436f} };
				outBoneTransform[8] = { {-0.043512f, 0.000000f, 0.000000f, 1},
									   {0.835972f, 0.018944f, 0.003312f, 0.548436f} };
				outBoneTransform[9] = { {-0.028422f, -0.000000f, -0.000000f, 1},
									   {0.890326f, 0.000173f, -0.008504f, 0.455244f} };
				outBoneTransform[10] = { {-0.022821f, -0.000000f, 0.000000f, 1},
										{1.000000f, 0.000000f, -0.000000f, 0.000000f} };
				outBoneTransform[11] = { {-0.003937f, 0.006967f, 0.016424f, 1},
										{0.532293f, -0.531137f, 0.472074f, 0.460113f} };
				outBoneTransform[12] = { {-0.070953f, -0.000779f, -0.000997f, 1},
										{0.908154f, -0.139967f, -0.013210f, 0.394323f} };
				outBoneTransform[13] = { {-0.043108f, -0.000000f, -0.000000f, 1},
										{0.977887f, 0.015350f, 0.008912f, 0.208378f} };
				outBoneTransform[14] = { {-0.033266f, -0.000000f, -0.000000f, 1},
										{0.992487f, -0.002006f, -0.020888f, 0.120540f} };
				outBoneTransform[15] = { {-0.025892f, 0.000000f, -0.000000f, 1},
										{0.999195f, 0.000000f, 0.000000f, 0.040126f} };
				outBoneTransform[16] = { {-0.001282f, -0.006612f, 0.016394f, 1},
										{0.544460f, -0.511334f, 0.436935f, 0.501187f} };
				outBoneTransform[17] = { {-0.065876f, -0.001786f, -0.000693f, 1},
										{0.971233f, -0.064561f, -0.071188f, 0.217877f} };
				outBoneTransform[18] = { {-0.040619f, -0.000000f, -0.000000f, 1},
										{0.978211f, -0.001419f, 0.000451f, 0.207607f} };
				outBoneTransform[19] = { {-0.028715f, 0.000000f, 0.000000f, 1},
										{0.987488f, -0.001166f, -0.010852f, 0.157314f} };
				outBoneTransform[20] = { {-0.022430f, 0.000000f, -0.000000f, 1},
										{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
				outBoneTransform[21] = { {0.002032f, -0.019020f, 0.015240f, 1},
										{0.513640f, -0.518192f, 0.337332f, 0.594860f} };
				outBoneTransform[22] = { {-0.062878f, -0.002844f, -0.000332f, 1},
										{0.983501f, -0.050059f, -0.104491f, 0.138930f} };
				outBoneTransform[23] = { {-0.030177f, -0.000000f, -0.000000f, 1},
										{0.981170f, 0.000501f, -0.001363f, 0.193138f} };
				outBoneTransform[24] = { {-0.018187f, -0.000000f, -0.000000f, 1},
										{0.997801f, 0.000487f, -0.051933f, -0.041173f} };
				outBoneTransform[25] = { {-0.018018f, -0.000000f, 0.000000f, 1},
										{1.000000f, -0.000000f, -0.000000f, -0.000000f} };
				outBoneTransform[26] = { {0.004574f, 0.055518f, 0.060226f, 1},
										{0.745334f, 0.161961f, -0.597782f, -0.246784f} };
				outBoneTransform[27] = { {0.013831f, -0.004360f, 0.069547f, 1},
										{-0.117443f, 0.257604f, 0.890065f, 0.357255f} };
				outBoneTransform[28] = { {0.038220f, -0.074817f, 0.046428f, 1},
										{-0.205767f, 0.697939f, 0.635107f, 0.259191f} };
				outBoneTransform[29] = { {0.035802f, -0.089658f, 0.081733f, 1},
										{-0.196007f, 0.758396f, 0.604341f, 0.145564f} };
				outBoneTransform[30] = { {0.029364f, -0.086069f, 0.119701f, 1},
										{-0.028444f, 0.787767f, 0.601616f, 0.129123f} };
			}
		}
	}
	else {
		// no touch
		if (isLeftHand) {
			outBoneTransform[6] = { {0.000632f, 0.026866f, 0.015002f, 1},
								   {0.644251f, 0.421979f, -0.478202f, 0.422133f} };
			outBoneTransform[7] = { {0.074204f, -0.005002f, 0.000234f, 1},
								   {0.995332f, 0.007007f, -0.039124f, 0.087949f} };
			outBoneTransform[8] = { {0.043930f, -0.000000f, -0.000000f, 1},
								   {0.997891f, 0.045808f, 0.002142f, -0.045943f} };
			outBoneTransform[9] = { {0.028695f, 0.000000f, 0.000000f, 1},
								   {0.999649f, 0.001850f, -0.022782f, -0.013409f} };
			outBoneTransform[10] = { {0.022821f, 0.000000f, -0.000000f, 1},
									{1.000000f, 0.000000f, -0.000000f, 0.000000f} };
			outBoneTransform[11] = { {0.002177f, 0.007120f, 0.016319f, 1},
									{0.546723f, 0.541277f, -0.442520f, 0.460749f} };
			outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
									{0.980294f, -0.167261f, -0.078959f, 0.069368f} };
			outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
									{0.997947f, 0.018493f, 0.013192f, 0.059886f} };
			outBoneTransform[14] = { {0.033266f, 0.000000f, 0.000000f, 1},
									{0.997394f, -0.003328f, -0.028225f, -0.066315f} };
			outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
									{0.999195f, 0.000000f, 0.000000f, 0.040126f} };
			outBoneTransform[16] = { {0.000513f, -0.006545f, 0.016348f, 1},
									{0.516692f, 0.550144f, -0.495548f, 0.429888f} };
			outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
									{0.990420f, -0.058696f, -0.101820f, 0.072495f} };
			outBoneTransform[18] = { {0.040697f, 0.000000f, 0.000000f, 1},
									{0.999545f, -0.002240f, 0.000004f, 0.030081f} };
			outBoneTransform[19] = { {0.028747f, -0.000000f, -0.000000f, 1},
									{0.999102f, -0.000721f, -0.012693f, 0.040420f} };
			outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
									{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
			outBoneTransform[21] = { {-0.002478f, -0.018981f, 0.015214f, 1},
									{0.526918f, 0.523940f, -0.584025f, 0.326740f} };
			outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
									{0.986609f, -0.059615f, -0.135163f, 0.069132f} };
			outBoneTransform[23] = { {0.030220f, 0.000000f, 0.000000f, 1},
									{0.994317f, 0.001896f, -0.000132f, 0.106446f} };
			outBoneTransform[24] = { {0.018187f, 0.000000f, 0.000000f, 1},
									{0.995931f, -0.002010f, -0.052079f, -0.073526f} };
			outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
									{1.000000f, -0.000000f, -0.000000f, -0.000000f} };
			outBoneTransform[26] = { {-0.006059f, 0.056285f, 0.060064f, 1},
									{0.737238f, 0.202745f, 0.594267f, 0.249441f} };
			outBoneTransform[27] = { {-0.040416f, -0.043018f, 0.019345f, 1},
									{-0.290330f, 0.623527f, -0.663809f, -0.293734f} };
			outBoneTransform[28] = { {-0.039354f, -0.075674f, 0.047048f, 1},
									{-0.187047f, 0.678062f, -0.659285f, -0.265683f} };
			outBoneTransform[29] = { {-0.038340f, -0.090987f, 0.082579f, 1},
									{-0.183037f, 0.736793f, -0.634757f, -0.143936f} };
			outBoneTransform[30] = { {-0.031806f, -0.087214f, 0.121015f, 1},
									{-0.003659f, 0.758407f, -0.639342f, -0.126678f} };
		}
		else {
			outBoneTransform[6] = { {-0.000632f, 0.026866f, 0.015002f, 1},
								   {0.421833f, -0.643793f, 0.422458f, 0.478661f} };
			outBoneTransform[7] = { {-0.074204f, 0.005002f, -0.000234f, 1},
								   {0.994784f, 0.007053f, -0.041286f, 0.093009f} };
			outBoneTransform[8] = { {-0.043930f, 0.000000f, 0.000000f, 1},
								   {0.998404f, 0.045905f, 0.002780f, -0.032767f} };
			outBoneTransform[9] = { {-0.028695f, -0.000000f, -0.000000f, 1},
								   {0.999704f, 0.001955f, -0.022774f, -0.008282f} };
			outBoneTransform[10] = { {-0.022821f, -0.000000f, 0.000000f, 1},
									{1.000000f, 0.000000f, -0.000000f, 0.000000f} };
			outBoneTransform[11] = { {-0.002177f, 0.007120f, 0.016319f, 1},
									{0.541874f, -0.547427f, 0.459996f, 0.441701f} };
			outBoneTransform[12] = { {-0.070953f, -0.000779f, -0.000997f, 1},
									{0.979837f, -0.168061f, -0.075910f, 0.076899f} };
			outBoneTransform[13] = { {-0.043108f, -0.000000f, -0.000000f, 1},
									{0.997271f, 0.018278f, 0.013375f, 0.070266f} };
			outBoneTransform[14] = { {-0.033266f, -0.000000f, -0.000000f, 1},
									{0.998402f, -0.003143f, -0.026423f, -0.049849f} };
			outBoneTransform[15] = { {-0.025892f, 0.000000f, -0.000000f, 1},
									{0.999195f, 0.000000f, 0.000000f, 0.040126f} };
			outBoneTransform[16] = { {-0.000513f, -0.006545f, 0.016348f, 1},
									{0.548983f, -0.519068f, 0.426914f, 0.496920f} };
			outBoneTransform[17] = { {-0.065876f, -0.001786f, -0.000693f, 1},
									{0.989791f, -0.065882f, -0.096417f, 0.081716f} };
			outBoneTransform[18] = { {-0.040697f, -0.000000f, -0.000000f, 1},
									{0.999102f, -0.002168f, -0.000020f, 0.042317f} };
			outBoneTransform[19] = { {-0.028747f, 0.000000f, 0.000000f, 1},
									{0.998584f, -0.000674f, -0.012714f, 0.051653f} };
			outBoneTransform[20] = { {-0.022430f, 0.000000f, -0.000000f, 1},
									{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
			outBoneTransform[21] = { {0.002478f, -0.018981f, 0.015214f, 1},
									{0.518597f, -0.527304f, 0.328264f, 0.587580f} };
			outBoneTransform[22] = { {-0.062878f, -0.002844f, -0.000332f, 1},
									{0.987294f, -0.063356f, -0.125964f, 0.073274f} };
			outBoneTransform[23] = { {-0.030220f, -0.000000f, -0.000000f, 1},
									{0.993413f, 0.001573f, -0.000147f, 0.114578f} };
			outBoneTransform[24] = { {-0.018187f, -0.000000f, -0.000000f, 1},
									{0.997047f, -0.000695f, -0.052009f, -0.056495f} };
			outBoneTransform[25] = { {-0.018018f, -0.000000f, 0.000000f, 1},
									{1.000000f, -0.000000f, -0.000000f, -0.000000f} };
			outBoneTransform[26] = { {0.005198f, 0.054204f, 0.060030f, 1},
									{0.747318f, 0.182508f, -0.599586f, -0.220688f} };
			outBoneTransform[27] = { {0.038779f, -0.042973f, 0.019824f, 1},
									{-0.297445f, 0.639373f, 0.648910f, 0.285734f} };
			outBoneTransform[28] = { {0.038027f, -0.074844f, 0.046941f, 1},
									{-0.199898f, 0.698218f, 0.635767f, 0.261406f} };
			outBoneTransform[29] = { {0.036845f, -0.089781f, 0.081973f, 1},
									{-0.190960f, 0.756469f, 0.607591f, 0.148733f} };
			outBoneTransform[30] = { {0.030251f, -0.086056f, 0.119887f, 1},
									{-0.018948f, 0.779249f, 0.612180f, 0.132846f} };
		}
	}
}

void GetGripClickBoneTransform(bool withController,
	bool isLeftHand,
	vr::VRBoneTransform_t outBoneTransform[]) {
	if (withController) {
		if (isLeftHand) {
			outBoneTransform[11] = { {0.002177f, 0.007120f, 0.016319f, 1},
									{0.529359f, 0.540512f, -0.463783f, 0.461011f} };
			outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
									{-0.831727f, 0.270927f, 0.175647f, -0.451638f} };
			outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
									{-0.854886f, -0.008231f, -0.028107f, -0.517990f} };
			outBoneTransform[14] = { {0.033266f, -0.000000f, 0.000000f, 1},
									{-0.825759f, 0.085208f, 0.086456f, -0.550805f} };
			outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
									{0.999195f, -0.000000f, 0.000000f, 0.040126f} };
			outBoneTransform[16] = { {0.000513f, -0.006545f, 0.016348f, 1},
									{0.500244f, 0.530784f, -0.516215f, 0.448939f} };
			outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
									{0.831617f, -0.242931f, -0.139695f, 0.479461f} };
			outBoneTransform[18] = { {0.040697f, 0.000000f, 0.000000f, 1},
									{0.769163f, -0.001746f, 0.001363f, 0.639049f} };
			outBoneTransform[19] = { {0.028747f, -0.000000f, -0.000000f, 1},
									{0.968615f, -0.064537f, -0.046586f, 0.235477f} };
			outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
									{1.000000f, 0.000000f, -0.000000f, -0.000000f} };
			outBoneTransform[21] = { {-0.002478f, -0.018981f, 0.015214f, 1},
									{0.474671f, 0.434670f, -0.653212f, 0.398827f} };
			outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
									{0.798788f, -0.199577f, -0.094418f, 0.559636f} };
			outBoneTransform[23] = { {0.030220f, 0.000002f, -0.000000f, 1},
									{0.853087f, 0.001644f, -0.000913f, 0.521765f} };
			outBoneTransform[24] = { {0.018187f, -0.000002f, 0.000000f, 1},
									{0.974249f, 0.052491f, 0.003591f, 0.219249f} };
			outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
									{1.000000f, 0.000000f, 0.000000f, 0.000000f} };

			outBoneTransform[28] = { {0.016642f, -0.029992f, 0.083200f, 1},
									{-0.094577f, 0.694550f, 0.702845f, 0.121100f} };
			outBoneTransform[29] = { {0.011144f, -0.028727f, 0.108366f, 1},
									{-0.076328f, 0.788280f, 0.605097f, 0.081527f} };
			outBoneTransform[30] = { {0.011333f, -0.026044f, 0.128585f, 1},
									{-0.144791f, 0.737451f, 0.656958f, -0.060069f} };
		}
		else {
			outBoneTransform[11] = { {0.002177f, 0.007120f, 0.016319f, 1},
									{0.529359f, 0.540512f, -0.463783f, 0.461011f} };
			outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
									{-0.831727f, 0.270927f, 0.175647f, -0.451638f} };
			outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
									{-0.854886f, -0.008231f, -0.028107f, -0.517990f} };
			outBoneTransform[14] = { {0.033266f, -0.000000f, 0.000000f, 1},
									{-0.825759f, 0.085208f, 0.086456f, -0.550805f} };
			outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
									{0.999195f, -0.000000f, 0.000000f, 0.040126f} };
			outBoneTransform[16] = { {0.000513f, -0.006545f, 0.016348f, 1},
									{0.500244f, 0.530784f, -0.516215f, 0.448939f} };
			outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
									{0.831617f, -0.242931f, -0.139695f, 0.479461f} };
			outBoneTransform[18] = { {0.040697f, 0.000000f, 0.000000f, 1},
									{0.769163f, -0.001746f, 0.001363f, 0.639049f} };
			outBoneTransform[19] = { {0.028747f, -0.000000f, -0.000000f, 1},
									{0.968615f, -0.064537f, -0.046586f, 0.235477f} };
			outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
									{1.000000f, 0.000000f, -0.000000f, -0.000000f} };
			outBoneTransform[21] = { {-0.002478f, -0.018981f, 0.015214f, 1},
									{0.474671f, 0.434670f, -0.653212f, 0.398827f} };
			outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
									{0.798788f, -0.199577f, -0.094418f, 0.559636f} };
			outBoneTransform[23] = { {0.030220f, 0.000002f, -0.000000f, 1},
									{0.853087f, 0.001644f, -0.000913f, 0.521765f} };
			outBoneTransform[24] = { {0.018187f, -0.000002f, 0.000000f, 1},
									{0.974249f, 0.052491f, 0.003591f, 0.219249f} };
			outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
									{1.000000f, 0.000000f, 0.000000f, 0.000000f} };

			outBoneTransform[28] = { {0.016642f, -0.029992f, 0.083200f, 1},
									{-0.094577f, 0.694550f, 0.702845f, 0.121100f} };
			outBoneTransform[29] = { {0.011144f, -0.028727f, 0.108366f, 1},
									{-0.076328f, 0.788280f, 0.605097f, 0.081527f} };
			outBoneTransform[30] = { {0.011333f, -0.026044f, 0.128585f, 1},
									{-0.144791f, 0.737451f, 0.656958f, -0.060069f} };
		}

	}
	else {
		if (isLeftHand) {
			outBoneTransform[11] = { {0.005787f, 0.006806f, 0.016534f, 1},
									{0.514203f, 0.522315f, -0.478348f, 0.483700f} };
			outBoneTransform[12] = { {0.070953f, 0.000779f, 0.000997f, 1},
									{0.723653f, -0.097901f, 0.048546f, 0.681458f} };
			outBoneTransform[13] = { {0.043108f, 0.000000f, 0.000000f, 1},
									{0.637464f, -0.002366f, -0.002831f, 0.770472f} };
			outBoneTransform[14] = { {0.033266f, 0.000000f, 0.000000f, 1},
									{0.658008f, 0.002610f, 0.003196f, 0.753000f} };
			outBoneTransform[15] = { {0.025892f, -0.000000f, 0.000000f, 1},
									{0.999195f, 0.000000f, 0.000000f, 0.040126f} };
			outBoneTransform[16] = { {0.004123f, -0.006858f, 0.016563f, 1},
									{0.489609f, 0.523374f, -0.520644f, 0.463997f} };
			outBoneTransform[17] = { {0.065876f, 0.001786f, 0.000693f, 1},
									{0.759970f, -0.055609f, 0.011571f, 0.647471f} };
			outBoneTransform[18] = { {0.040331f, 0.000000f, 0.000000f, 1},
									{0.664315f, 0.001595f, 0.001967f, 0.747449f} };
			outBoneTransform[19] = { {0.028489f, -0.000000f, -0.000000f, 1},
									{0.626957f, -0.002784f, -0.003234f, 0.779042f} };
			outBoneTransform[20] = { {0.022430f, -0.000000f, 0.000000f, 1},
									{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
			outBoneTransform[21] = { {0.001131f, -0.019295f, 0.015429f, 1},
									{0.479766f, 0.477833f, -0.630198f, 0.379934f} };
			outBoneTransform[22] = { {0.062878f, 0.002844f, 0.000332f, 1},
									{0.827001f, 0.034282f, 0.003440f, 0.561144f} };
			outBoneTransform[23] = { {0.029874f, 0.000000f, 0.000000f, 1},
									{0.702185f, -0.006716f, -0.009289f, 0.711903f} };
			outBoneTransform[24] = { {0.017979f, 0.000000f, 0.000000f, 1},
									{0.676853f, 0.007956f, 0.009917f, 0.736009f} };
			outBoneTransform[25] = { {0.018018f, 0.000000f, -0.000000f, 1},
									{1.000000f, -0.000000f, -0.000000f, -0.000000f} };

			outBoneTransform[28] = { {0.000448f, 0.001536f, 0.116543f, 1},
									{-0.039357f, 0.105143f, -0.928833f, -0.353079f} };
			outBoneTransform[29] = { {0.003949f, -0.014869f, 0.130608f, 1},
									{-0.055071f, 0.068695f, -0.944016f, -0.317933f} };
			outBoneTransform[30] = { {0.003263f, -0.034685f, 0.139926f, 1},
									{0.019690f, -0.100741f, -0.957331f, -0.270149f} };
		}
		else {
			outBoneTransform[11] = { {-0.005787f, 0.006806f, 0.016534f, 1},
									{0.522315f, -0.514203f, 0.483700f, 0.478348f} };
			outBoneTransform[12] = { {-0.070953f, -0.000779f, -0.000997f, 1},
									{0.723653f, -0.097901f, 0.048546f, 0.681458f} };
			outBoneTransform[13] = { {-0.043108f, -0.000000f, -0.000000f, 1},
									{0.637464f, -0.002366f, -0.002831f, 0.770472f} };
			outBoneTransform[14] = { {-0.033266f, -0.000000f, -0.000000f, 1},
									{0.658008f, 0.002610f, 0.003196f, 0.753000f} };
			outBoneTransform[15] = { {-0.025892f, 0.000000f, -0.000000f, 1},
									{0.999195f, 0.000000f, 0.000000f, 0.040126f} };
			outBoneTransform[16] = { {-0.004123f, -0.006858f, 0.016563f, 1},
									{0.523374f, -0.489609f, 0.463997f, 0.520644f} };
			outBoneTransform[17] = { {-0.065876f, -0.001786f, -0.000693f, 1},
									{0.759970f, -0.055609f, 0.011571f, 0.647471f} };
			outBoneTransform[18] = { {-0.040331f, -0.000000f, -0.000000f, 1},
									{0.664315f, 0.001595f, 0.001967f, 0.747449f} };
			outBoneTransform[19] = { {-0.028489f, 0.000000f, 0.000000f, 1},
									{0.626957f, -0.002784f, -0.003234f, 0.779042f} };
			outBoneTransform[20] = { {-0.022430f, 0.000000f, -0.000000f, 1},
									{1.000000f, 0.000000f, 0.000000f, 0.000000f} };
			outBoneTransform[21] = { {-0.001131f, -0.019295f, 0.015429f, 1},
									{0.477833f, -0.479766f, 0.379935f, 0.630198f} };
			outBoneTransform[22] = { {-0.062878f, -0.002844f, -0.000332f, 1},
									{0.827001f, 0.034282f, 0.003440f, 0.561144f} };
			outBoneTransform[23] = { {-0.029874f, -0.000000f, -0.000000f, 1},
									{0.702185f, -0.006716f, -0.009289f, 0.711903f} };
			outBoneTransform[24] = { {-0.017979f, -0.000000f, -0.000000f, 1},
									{0.676853f, 0.007956f, 0.009917f, 0.736009f} };
			outBoneTransform[25] = { {-0.018018f, -0.000000f, 0.000000f, 1},
									{1.000000f, -0.000000f, -0.000000f, -0.000000f} };

			outBoneTransform[28] = { {-0.000448f, 0.001536f, 0.116543f, 1},
									{-0.039357f, 0.105143f, 0.928833f, 0.353079f} };
			outBoneTransform[29] = { {-0.003949f, -0.014869f, 0.130608f, 1},
									{-0.055071f, 0.068695f, 0.944016f, 0.317933f} };
			outBoneTransform[30] = { {-0.003263f, -0.034685f, 0.139926f, 1},
									{0.019690f, -0.100741f, 0.957331f, 0.270149f} };
		}
	}
}


inline vr::HmdVector4_t Lerp(vr::HmdVector4_t& v1, vr::HmdVector4_t& v2, double lambda)
{
	vr::HmdVector4_t res;
	res.v[0] = (float)((1 - lambda) * v1.v[0] + lambda * v2.v[0]);
	res.v[1] = (float)((1 - lambda) * v1.v[1] + lambda * v2.v[1]);
	res.v[2] = (float)((1 - lambda) * v1.v[2] + lambda * v2.v[2]);
	res.v[3] = 1;

	return res;
}

inline vr::HmdQuaternionf_t Slerp(vr::HmdQuaternionf_t& q1, vr::HmdQuaternionf_t& q2, double lambda)
{
	if (q1.w != q2.w || q1.x != q2.x || q1.y != q2.y || q1.z != q2.z) {
		float dotproduct = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
		float theta, st, sut, sout, coeff1, coeff2;

		// algorithm adapted from Shoemake's paper

		theta = (float)acos(dotproduct);
		if (theta < 0.0) theta = -theta;

		st = (float)sin(theta);
		sut = (float)sin(lambda * theta);
		sout = (float)sin((1 - lambda) * theta);
		coeff1 = sout / st;
		coeff2 = sut / st;

		vr::HmdQuaternionf_t res;
		res.w = coeff1 * q1.w + coeff2 * q2.w;
		res.x = coeff1 * q1.x + coeff2 * q2.x;
		res.y = coeff1 * q1.y + coeff2 * q2.y;
		res.z = coeff1 * q1.z + coeff2 * q2.z;

		float norm = res.w * res.w + res.x * res.x + res.y * res.y + res.z * res.z;
		res.w /= norm;
		res.x /= norm;
		res.y /= norm;
		res.z /= norm;

		return res;
	}
	else {
		return q1;
	}
}
void ControllerDriver::GetBoneTransform(bool withController,
	bool isLeftHand,
	float thumbAnimationProgress,
	float indexAnimationProgress,
	uint64_t lastPoseButtons,
	RVR::RVRControllerData data,
	vr::VRBoneTransform_t outBoneTransform[]) {

	vr::VRBoneTransform_t boneTransform1[SKELETON_BONE_COUNT];
	vr::VRBoneTransform_t boneTransform2[SKELETON_BONE_COUNT];

	// root and wrist
	outBoneTransform[0] = { {0.000000f, 0.000000f, 0.000000f, 1},
						   {1.000000f, -0.000000f, -0.000000f, 0.000000f} };
	if (isLeftHand) {
		outBoneTransform[1] = { {-0.034038f, 0.036503f, 0.164722f, 1},
							   {-0.055147f, -0.078608f, -0.920279f, 0.379296f} };
	}
	else {
		outBoneTransform[1] = { {0.034038f, 0.036503f, 0.164722f, 1},
							   {-0.055147f, -0.078608f, 0.920279f, -0.379296f} };
	}

	// thumb
	GetThumbBoneTransform(withController, isLeftHand, lastPoseButtons, boneTransform1);
	GetThumbBoneTransform(withController, isLeftHand, data.buttonState, boneTransform2);
	for (int boneIdx = 2; boneIdx < 6; boneIdx++) {
		outBoneTransform[boneIdx].position = Lerp(boneTransform1[boneIdx].position,
			boneTransform2[boneIdx].position,
			thumbAnimationProgress);
		outBoneTransform[boneIdx].orientation = Slerp(boneTransform1[boneIdx].orientation,
			boneTransform2[boneIdx].orientation,
			thumbAnimationProgress);
	}
	data.analog1D[0] = data.analog1D[0] / 172 * 255;
	// trigger (index to pinky)
	if (data.analog1D[0] > 0) {
		GetTriggerBoneTransform(
			withController, isLeftHand, data.buttonState, boneTransform1);
		if (data.analog1D[0]>0.99)
		{
			data.buttonState = data.buttonState |(uint32_t) ButtonStateValue::kTriggerClickState;
		}
		GetTriggerBoneTransform(
			withController, isLeftHand, data.buttonState, boneTransform2);
		for (int boneIdx = 6; boneIdx < SKELETON_BONE_COUNT; boneIdx++) {
			outBoneTransform[boneIdx].position = Lerp(
				boneTransform1[boneIdx].position, boneTransform2[boneIdx].position, data.analog1D[0]);
			outBoneTransform[boneIdx].orientation = Slerp(boneTransform1[boneIdx].orientation,
				boneTransform2[boneIdx].orientation,
				data.analog1D[0]);
		}
	}
	else {
		GetTriggerBoneTransform(withController, isLeftHand, lastPoseButtons, boneTransform1);
		GetTriggerBoneTransform(withController, isLeftHand, data.buttonState, boneTransform2);
		for (int boneIdx = 6; boneIdx < SKELETON_BONE_COUNT; boneIdx++) {
			outBoneTransform[boneIdx].position = Lerp(boneTransform1[boneIdx].position,
				boneTransform2[boneIdx].position,
				indexAnimationProgress);
			outBoneTransform[boneIdx].orientation = Slerp(boneTransform1[boneIdx].orientation,
				boneTransform2[boneIdx].orientation,
				indexAnimationProgress);
		}
	}

	// grip (middle to pinky)
	if (data.analog1D[1] > 0) {
		GetGripClickBoneTransform(withController, isLeftHand, boneTransform2);
		for (int boneIdx = 11; boneIdx < 26; boneIdx++) {
			outBoneTransform[boneIdx].position = Lerp(
				outBoneTransform[boneIdx].position, boneTransform2[boneIdx].position, data.analog1D[1]);
			outBoneTransform[boneIdx].orientation = Slerp(outBoneTransform[boneIdx].orientation,
				boneTransform2[boneIdx].orientation,
				data.analog1D[1]);
		}
		for (int boneIdx = 28; boneIdx < SKELETON_BONE_COUNT; boneIdx++) {
			outBoneTransform[boneIdx].position = Lerp(
				outBoneTransform[boneIdx].position, boneTransform2[boneIdx].position, data.analog1D[1]);
			outBoneTransform[boneIdx].orientation = Slerp(outBoneTransform[boneIdx].orientation,
				boneTransform2[boneIdx].orientation,
				data.analog1D[1]);
		}
	}
}
