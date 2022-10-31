#include "VideoEncoderConfig.h"

bool VideoEncoderConfig::operator==(const VideoEncoderConfig& c) const
{
	if (instanceId == c.instanceId
		&& D3D11Device == c.D3D11Device
		&& level == c.level
		&& profile == c.profile
		&& entropyCodingMode == c.entropyCodingMode
		&& width == c.width
		&& height == c.height
		&& fps == c.fps
		&& format == c.format
		&& GOPSize == c.GOPSize
		&& enableForceIntraRefresh == c.enableForceIntraRefresh
		&& intraRefreshLength == c.intraRefreshLength
		&& intraRefreshPeriod == c.intraRefreshPeriod
		&& CSDPeriod == c.CSDPeriod
		&& maxDPBSize == c.maxDPBSize
		&& avgBitRate == c.avgBitRate
		&& maxBitRate == c.maxBitRate
		&& bufferSizeVBV == c.bufferSizeVBV
		&& rcMode == c.rcMode
		&& codec == c.codec
		&& minIQP == c.minIQP
		&& minPQP == c.minPQP
		&& maxIQP == c.maxIQP
		&& maxPQP == c.maxPQP
		&& sliceMode == c.sliceMode
		&& sliceSize == c.sliceSize
		&& independentSliceEncode == c.independentSliceEncode
		&& flags == c.flags
		&& encoderOutputMode == c.encoderOutputMode
		&& lowLatencyMode == c.lowLatencyMode
		)
	{
		return true;
	}

	return false;
}
