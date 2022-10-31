#include <stdlib.h>
#include <stdio.h>
#include"AudioDeviceManger.h"
void main() 
{
    AudioDeviceManger audio_device_manger;
    audio_device_manger.StartUp();
    char key = 'K';
    getchar();
    audio_device_manger.ClearUp();

}