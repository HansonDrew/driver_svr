#pragma once


#include <d3d11.h>

struct VideoEncoderConfig
{
    // Instance identifier; cookie used differentiate encoder instances
    int instanceId;

    // D3D11 device pointer
    ID3D11Device* D3D11Device;

    void* reserved; //Currently it is the pointer of RVR::VEncConfig::IEncOutputSink* encOutputSink

    // Codec level
    // 40 = Level_4, 41 = Level_4.1, 42 = Level_4.2,
    // 50 = Level_5, 51 = Level_5.1, 52 = Level_5.2,
    // 60 = Level_6, 61 = Level_6.1, 62 = Level_6.2,
    int level;

    // Codec profile
    // 0 = H264.Base, 1 = H264.Main, 2 = H264.High, 3 = H264.Lossless
    // 0 = H265.Main, 1 = H265.MAIN10, 2 = H265.FREXT
    int profile;

    // Entropy coding mode (0 - CAVLC, 1 - CABAC)
    int entropyCodingMode;

    // Image width
    int width;

    // Image height
    int height;

    // Frames per second
    int fps;

    // Format: (0 = BGRA32, 1 = NV12, 2 = P010, 3 = P016)
    int format;

    // I frame interval
    int GOPSize;

    // Enables on demand intra refresh mode
    bool enableForceIntraRefresh;

    // Number of IntraRefresh duration
    // It should be less than intraRefreshPeriod
    int intraRefreshLength;

    // Period between successive IntraRefresh (in number of frames)
    int intraRefreshPeriod;

    // Codec specific data (SPS/PPS) repeation period in frames
    int CSDPeriod;

    // Max reference frames in cc (Decoded picture buffering)
    int maxDPBSize;

    // Video avg bitrate
    int avgBitRate;

    // Video max bitrate
    int maxBitRate;

    // VBV buffer size (if -1, it lets encoder use default value)
    int bufferSizeVBV;

    // Rate control mode ( 0 = CBR, 1 = VBR)
    int rcMode;

    // Codec to be used (0 = h264)
    int codec;

    // Min quantization parameter (for I and P frames)
    int minIQP;
    int minPQP;

    // Max quantization parameter (for I and P frames)
    int maxIQP;
    int maxPQP;

    // Slice mode
    // 0 - slice size is specified in macroblocks/CTUs per slice
    // 1 - slice size is specified in bytes per slice
    // 2 - slice size is specified in units of macroblock/CTU rows per slice
    int sliceMode;

    // Slice size value is going to be in different units
    // based on sliceMode
    int sliceSize;

    // Each frame needs to be encoded independent of other slices in the frame
    bool independentSliceEncode;

    // Flags can be bitwise OR of one or more VEncFlags
    int flags;

    // Preferred encoder output mode
    // 0 - slice based output
    // 1 - frame based output
    int encoderOutputMode;

    //low latency mode, 0 disabled, 1 enabled
    bool lowLatencyMode;

    bool operator==(const VideoEncoderConfig& c) const;

};
