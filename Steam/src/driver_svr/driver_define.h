#pragma once
 
#define STEAMVRVERSIONDOOR 1218

#define ChaperOneMsg WM_USER+201
#define MSG_DEPTH WM_USER + 202
#define OUTTHREADMSG WM_USER+206

#define DisplayEdid  0x3241
#define TOGETHERSENSORBUFLEN 443///107+168*2 


#define TcpSensorDataLen 440
#define TcpSensorBufLen 445   
#define TcpSensorWithAddDataLen 456
#define TcpSensorWithAddBufLen 461
enum Eye {
	kLeft = 0,
	kRight = 1
};

enum class ButtonStateValue
{
	kXAClickState        =0x01,
	kYBClickState        =0x02,
	kXATouchState        =0x04,
	kYBTouchState        =0x08,
	KJoystickTouchState  =0x10,
	kTriggerTouchState   =0x20,
	kGripTouchState      =0x40,
	kBlankTouch          = 0x80,
	kPicoHome            =0x100,
	kPicoMenuState       =0x200,
	kGripClickState      =0x1000,
	kTriggerClickState   =0x2000,
	kJoystickClickState = 0x8000
};
 
enum Hmd_TYPE
{
	PRO = 0,
	EYE = 1
};

enum Hmd_Active_Type
{
	None=-1,
	DP = 0,
	Streaming = 1
};

enum class DriverModuleType
{
	kLeftController = 0,
	kRightController = 1,
	kHmd
};
enum class ControllerIndex
{
	kLeftController = 0,
	kRightController = 1,
	kUnknow=2
};


enum Button_Handle
{
	kApplicationMenu        = 0,
	kTriggerClick           = 1,
	kTrackpadOrJoystickClick = 2,
	kTrackpadTouch          = 3,
	kTriggerValue           = 4,
	kTrackpadx              = 5,
	kTrackpady              = 6,
	kGripClick              = 7,
	kSystemClick            = 8,
	kHaptic                 = 9,
	kJoystickTouch         = 10,
	kJoystickx             = 11,
	kJoysticky             = 12,
	kGripValue              = 13,
	kGripTouch              = 14,
	kXClick                 = 15,
	kXTouch                 = 16,
	kYClick                 = 17,
	kYTouch                 = 18,
	kAClick                 = 19,
	kATouch                 = 20,
	kBClick                 = 21,
	kBTouch                 = 22,
	kTriggerTouch           = 23,
	kSystemTouch            = 24
};

#include<stdint.h>
#define HidDataLen 63
#define HidErrorSleepTime 50
#define HidBufLen 64
#define HidVid 0x2d40
#define HidPid 0x0016
namespace HidType
{
	enum class MessageType
	{
		kHmdData   = 0,
		kLeftControllerData  = 1,
		kRightControllerData = 2,
		kDistortionOffset    = 3,
		kChaperOne=4
	};

 
	struct TransQuaternion
	{
		//uint16_t x, y, z, w;
		char data[16];
	};//��Ԫ��

	struct TransPosition
	{
		//uint16_t x, y, z;
		char data[12];
	};//��ά�����
	struct Velocity
	{
		char data[6];  //short[3] ��λ m/s  �������� ��λС�� ��ԭʼ����*1000��ȡ�������� ������������ֵӦ�� -32768~32767 ֮�䣬 ���������Χ�� ȡ�߽����ֵ���ձ��������� ���ټ�¼ 163km/h �� 45 m/s ,��Ͷ 25m/s ��ְҵ�˶�Ա�ż�)
	};
	struct Acceleration
	{
		char data[6];  // short[3] ��λ  m/s^2 (���� 2 λС�� ) ԭʼ���� *100��ȡ��,�������ͬ�� ����־�￴������ܻӵ� 100m/^2
	};
	struct AngularVelocity
	{
		char data[6];  // short[3]   ��λradians/second  ��(���� 2λС�� ) ԭʼ����*100��ȡ��,�������ͬ�ϡ� ��ൽ  40  radians/second
	};
	struct TransVector2
	{
		uint8_t x : 8;
		uint8_t y : 8;
	};//��ά�����
	enum class ChaperOneRequestType
	{
		kGetStart = 0x0000,
		kGetEnd = 0x0001

	};
	enum class ControllerButtonState
	{

		kNone = 0x0000,
		kXAClick = 0x0001,
		kXATouch = 0x0002,
		kYBClick = 0x0004,
		kYBTouch = 0x0008,
		kJoystickClick = 0x0010,
		kJoystickTouch = 0x0020,
		kTriggerTouch = 0x0040,
		//kTriggerTouch = 0x0080,
		kGripTouch = 0x0100,
		//kGripTouch = 0x0200,
		kSystemButtonClick = 0x1000,
		kMenuButtonClick = 0x2000,
	};

	enum class TransControllerConnectionState
	{
		kNotInitialized = 0,
		kDisconnected = 1,
		kConnected = 2,
		kConnecting = 3,
		kError = 4
	};//�ֱ�״̬

	struct HapticVibration
	{
		uint8_t data_type;
		float duration_seconds; //�𶯳���ʱ�䣨�룩
		float frequency; //��Ƶ��
		float amplitude; //��ǿ�� ��0-1��
		uint8_t type;
	};

	struct DeviceState
	{
		uint8_t battery : 8;
		///
		uint8_t connection_state : 5; //����״̬
		uint8_t  type : 3;  // 0 - head, 1 - left, 2 - right 3 offset 4 chaperone

	};
	struct TransData
	{
		TransQuaternion rotation;
		TransPosition position;
		Velocity  velocity;//���ٶ�
		Acceleration acceleration;//�߼��ٶ�
		AngularVelocity vecAngularVelocity;//�Ƕȵ�

		char button_state[2];//����״̬����Ӧ enum class ControllerButtonState ��
		TransVector2 analog_2d;//analog2D[0]��ֵ��analog2D[1]���ã���ʱ����ֵ����ҡ��λ�ã�xΪ�����꣬����Ϊ����yΪ�����꣬����Ϊ������ֵ��Χ��-1.0 - 1.0��
		char analog_1d[2];//analog1D[0]Ϊ trigger ������ֵ��analog1D[1]Ϊ grip ������ֵ��Ӳ����֧���򲻸�ֵ������ֵ��Χ��0 - 1.0��
		char timestamp[8];
		uint8_t ipd;
	};

	struct ButtonStateGather
	{
		short button_state;
		TransVector2 joystick;
		uint8_t analog_trigger_grip[2];//a
	};
	struct TransDataAxisOffset
	{
		float data[12];
	};
	// report to hid
	typedef struct HidStreamingData
	{
		DeviceState devState;
		TransData transData;


	} HidStreamingData;
	typedef struct HidAxsiOffsetData
	{
		DeviceState devState;
		// union
		// {
			// TransData transData;
		TransDataAxisOffset axisOffset;
		// };
	} HidAxsiOffsetData;


	typedef struct HidRequestData
	{
		uint8_t data_type;           // �̶�ֵΪ0�������޸�
		uint8_t request_type;       // ���������ݵ����ͣ�0��Ӧ����ͨ�õ���Ӧ���ݣ�1��Ӧ�����ֱ�������
		unsigned char data[61];
	} HidRequestData;

	typedef struct HidResponseData
	{
		DeviceState devState;
		unsigned char data[61];
	} HidResponseData, HidCommon;

};

namespace WireLessType 
{
	struct AddachedMsg
	{
		int net_cost;
		int hmd_index;
		int standby[2];
	};
	struct EncodeParam 
	{
		int render_width;
		int render_height;
		int encode_width;
		int encode_heigth;	
		int cut_x;
		int cut_y;
		int compress;
	};
	struct TransQuaternion
	{
		float x, y, z, w;
	};//��Ԫ��
	struct TransVector3
	{
		float x, y, z;
	};//��ά�����
	struct TransVector2
	{
		float x, y;
	};//��ά�����

	//******************************************************************************
	struct TransPoseData
	{
		TransQuaternion rotation;
		TransVector3 position;

		uint64_t poseTimeStamp;//�����ݵ�ʱ���
		float predictedTimeMs;
		uint64_t poseRecvTime;
		TransVector3 linearVelocity;
		TransVector3 linearAcceleration;
		TransVector3 angularVelocity;
		TransVector3 angularAcceleration;

	};// HMD λ����Ϣ
	enum class TransControllerConnectionState {
		kNotInitialized = 0,
		kDisconnected = 1,
		kConnected = 2,
		kConnecting = 3,
		kError = 4
	};//�ֱ�״̬


	struct TransControllerData
	{
		TransQuaternion rotation;
		TransVector3 position;
		TransVector3 vecAngularVelocity;
		TransVector3 vecAcceleration;
		int64_t timestamp;
		uint32_t buttonState;
		TransVector2 analog2D[4];
		float analog1D[8];
		uint32_t isTouching;
		TransControllerConnectionState connectionState;
		TransVector3 vecVelocity;
		TransVector3 vecAngularAcceleration;
	};

	struct AddQuaternion
	{
		float x, y, z, w;
	};
	struct AddVector3
	{
		float x, y, z;
	};


	struct AddPoseData
	{
		AddQuaternion rotation;
		AddVector3 position;

		uint64_t poseTimeStamp;
		float predictedTimeMs;
		uint64_t hmdPoseTimeTs;//0825
		int gameRenderCost;
		int encodeCost;
		int autoRateFlag;
		int encodeRate;

	};

	

}

//namespace UsbBulkType 
//{
//	struct EncodeParam 
//	{
//		int render_width;
//		int render_height;
//		int encode_width;
//		int encode_heigth;	
//		int cut_x;
//		int cut_y;
//		int compress;
//	};
//	struct TransQuaternion
//	{
//		float x, y, z, w;
//	};//��Ԫ��
//	struct TransVector3
//	{
//		float x, y, z;
//	};//��ά�����
//	struct TransVector2
//	{
//		float x, y;
//	};//��ά�����
//
//	//******************************************************************************
//	struct TransPoseData
//	{
//		TransQuaternion rotation;
//		TransVector3 position;
//
//		uint64_t poseTimeStamp;//�����ݵ�ʱ���
//		float predictedTimeMs;
//		uint64_t poseRecvTime;
//		TransVector3 linearVelocity;
//		TransVector3 linearAcceleration;
//		TransVector3 angularVelocity;
//		TransVector3 angularAcceleration;
//
//	};// HMD λ����Ϣ
//	enum class TransControllerConnectionState {
//		kNotInitialized = 0,
//		kDisconnected = 1,
//		kConnected = 2,
//		kConnecting = 3,
//		kError = 4
//	};//�ֱ�״̬
//
//
//	struct TransControllerData
//	{
//		TransQuaternion rotation;
//		TransVector3 position;
//		TransVector3 vecAngularVelocity;
//		TransVector3 vecAcceleration;
//		int64_t timestamp;
//		uint32_t buttonState;
//		TransVector2 analog2D[4];
//		float analog1D[8];
//		uint32_t isTouching;
//		TransControllerConnectionState connectionState;
//		TransVector3 vecVelocity;
//		TransVector3 vecAngularAcceleration;
//	};
//
//	struct AddQuaternion
//	{
//		float x, y, z, w;
//	};
//	struct AddVector3
//	{
//		float x, y, z;
//	};
//
//
//	struct AddPoseData
//	{
//		AddQuaternion rotation;
//		AddVector3 position;
//
//		uint64_t poseTimeStamp;
//		float predictedTimeMs;
//		uint64_t poseRecvTimeUs;
//
//	};
//
//}
