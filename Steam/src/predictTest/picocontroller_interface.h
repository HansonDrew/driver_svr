/**********************************************************************/ /**
  *------------------------------------------------------------------------
  * 数据流要求：
    * 1. 相对于4Cam数据，IMU数据采集先开始后结束。
    * 2. 开始采集时，第一帧图像timestamp之前的IMU数据在满足实时要求的条件下越多越好(目前133个IMU数据最好，再多不会有提高)。
    * 3. 4CAM 编号
    *    cam0 (up-left)    ---    cam1 (up-right)
    *      |                        |
    *    cam3 (down-left)  ---    cam2 (down-right)
    *   按照固定顺序cam0-cam1-cam2-cam3(头戴视角)传递4张图像, 4张图像的时间戳的差值在50微秒以内，无效或丢帧的图像则赋值为空指针 
******************************************************************************/ 
#ifndef __PICOCONTROLLER_INTERFACE_H__
#define __PICOCONTROLLER_INTERFACE_H__

 


// pico slam results
typedef struct six_dof
{
    double timestamp; // nanoseconds
    double x;  // positon X
    double y;  // position Y
    double z;  // position Z
    double rw; // rotation W
    double rx; // rotation X
    double ry; // rotation Y
    double rz; // rotation Z
} six_dof_t;

typedef struct 
{
   six_dof_t pose;
   double vx,vy,vz;
   double ax,ay,az;
   double wx,wy,wz;
   double w_ax,w_ay,w_az;   

} algo_result_t;



#ifdef __cplusplus
extern "C"
{
#endif

    int PredictMotion(algo_result_t input_algo_result, double predictTime, algo_result_t& new_algo_result);
   
#ifdef __cplusplus
} // extern "C"
#endif

#endif
