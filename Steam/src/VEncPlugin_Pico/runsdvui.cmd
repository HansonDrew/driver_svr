cd /d "D:\working\steamvrdriver\picomodel\RVRSDK_2.3.8_Source\RVRRenderer\Steam\src\VEncPlugin_Pico" &msbuild "VEncPlugin_Pico.vcxproj" /t:sdvViewer /p:configuration="Release" /p:platform=x64
exit %errorlevel% 