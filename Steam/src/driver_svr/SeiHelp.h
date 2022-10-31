#include <inttypes.h>
#include<vector>
#include "driverlog.h"
#include "RVRPluginDefinitions.h"
namespace pico_streaming
{
	const static uint32_t UUID_LENGTH = 16;

	const static uint8_t uuid_pico[UUID_LENGTH] = { 0x6c, 0x32, 0xf0, 0x32,
																									0x03, 0x69, 0x4c, 0xcc,
																									0xa8, 0x61, 0x83, 0x37,
																									0x31, 0xa1, 0xbb, 0xf5 };

	const static uint32_t NALU_HEADER_LENGTH = 4;

	const static uint8_t nalu_header_bytes[NALU_HEADER_LENGTH] = { 0, 0, 0, 1 };

	const static uint32_t PADDING_SIZE = 16;


	static std::vector<uint8_t> SerializeData(uint64_t data) {
		std::vector<uint8_t> result;
		while (data >= 0xff) {
			data -= 0xff;
			result.emplace_back(0xff);
		}
		result.emplace_back(data);
		return result;
	}

	static uint64_t DeserializeData(const std::vector<uint8_t>& data) {
		uint64_t result = 0;
		std::vector<uint8_t>::const_iterator& begin = data.cbegin();
		std::vector<uint8_t>::const_iterator& end = data.cend();
		while ((begin < end) && (*begin == 0xff)) {
			result += 0xff;
			++begin;
		}
		if (begin < end) {
			result += (*begin++);
		}
		return result;
	}

	static void EscapeData(uint8_t* start, uint8_t* end, std::vector<uint8_t>& result) {
		result.clear();
		if (start < end) {
			result.push_back(*start++);
		}
		if (start < end) {
			result.push_back(*start++);
		}
		while (start < end) {
			if (start[0] <= 0x03 && !result.end()[-2] && !result.end()[-1]) {
				result.push_back(0x03);
			}
			result.push_back(*start++);
		}
	}

	static void DeescapeData(uint8_t* start, uint8_t* end, std::vector<uint8_t>& result) {
		
		result.clear();
		size_t length = end - start;
		result.reserve(length);

		for (size_t i = 0; i < length;) {
			if (length - i >= 3 && !start[i] && !start[i + 1] && start[i + 2] == 3) {
				result.push_back(start[i++]);
				result.push_back(start[i++]);
				i++;
			}
			else {
				result.push_back(start[i++]);
			}
		}
	}

	static bool EncodedFramePutSei(
		int32_t codec_type,
		uint8_t* input,
		int32_t input_len,
		uint8_t* sei_payload,
		int32_t sei_payload_size,
		uint8_t* output,
		int32_t& output_len) {
		if (input == nullptr || sei_payload == nullptr || output == nullptr) {
			return false;
		}
		RVR::RVRPoseHmdData out_sensor;
		memmove(&out_sensor, sei_payload, sei_payload_size);
		/*DriverLog("EncodedFramePutSei index=%llu r=%f,%f,%f,%f p=%f,%f,%f,",
			out_sensor.poseTimeStamp, out_sensor.rotation.w, out_sensor.rotation.x,
			out_sensor.rotation.y, out_sensor.rotation.z,
			out_sensor.position.x, out_sensor.position.y, out_sensor.position.z);*/

		std::vector<uint8_t> sei_payload_size_data = SerializeData(sei_payload_size + UUID_LENGTH);

		std::vector<uint8_t> sei_payload_escape_data;
		EscapeData(sei_payload, sei_payload + sei_payload_size, sei_payload_escape_data);


		if (codec_type == 0) {
			int32_t expeceted_len = input_len + NALU_HEADER_LENGTH + sei_payload_escape_data.size() + UUID_LENGTH + sei_payload_size_data.size() + 3;
			if (expeceted_len % PADDING_SIZE != 0) {
				expeceted_len += (PADDING_SIZE - expeceted_len % PADDING_SIZE);
			}
			if (output_len < expeceted_len) {
				return false;
			}
		}
		else if (codec_type == 1) {
			int32_t expeceted_len = input_len + NALU_HEADER_LENGTH + sei_payload_size + UUID_LENGTH + sei_payload_size_data.size() + 4;
			if (expeceted_len % PADDING_SIZE != 0) {
				expeceted_len += (PADDING_SIZE - expeceted_len % PADDING_SIZE);
			}
			if (output_len < expeceted_len) {
				return false;
			}
		}
		else {
			return false;
		}

		int32_t current_len = 0;
		memcpy(&output[current_len], nalu_header_bytes, NALU_HEADER_LENGTH);
		current_len += NALU_HEADER_LENGTH;
		if (codec_type == 0) {
			output[current_len++] = 0x06;
		}
		else if (codec_type == 1) {
			output[current_len++] = 0x50;
			output[current_len++] = 0x01;
		}
		else {
			return false;
		}
		output[current_len++] = 0x05;
		memcpy(&output[current_len], sei_payload_size_data.data(), sei_payload_size_data.size());
		current_len += sei_payload_size_data.size();
		memcpy(&output[current_len], uuid_pico, UUID_LENGTH);
		current_len += UUID_LENGTH;
		memcpy(&output[current_len], sei_payload_escape_data.data(), sei_payload_escape_data.size());
		current_len += sei_payload_escape_data.size();
		output[current_len++] = 0x80;
		memcpy(&output[current_len], input, input_len);
		current_len += input_len;
		if (current_len % PADDING_SIZE != 0) {
			int32_t padding_size = PADDING_SIZE - current_len % PADDING_SIZE;
			memset(&output[current_len], 0, padding_size);
			current_len += padding_size;
		}
		output_len = current_len;
		return true;
	}

	static bool EncodedFrameTakeSei(
		int32_t codec_type,
		uint8_t* input,
		int32_t input_len,
		uint8_t* sei_payload,
		int32_t& sei_payload_size,
		uint8_t* output,
		int32_t& output_len) {
		if (input == nullptr || sei_payload == nullptr || output == nullptr) {
			return false;
		}
		int32_t current_len = 0;
		if (memcmp(input, nalu_header_bytes, NALU_HEADER_LENGTH) != 0) {
			return false;
		}
		current_len += NALU_HEADER_LENGTH;
		if (codec_type == 0) {
			if (input[current_len++] != 0x06) {
				return false;
			}
		}
		else if (codec_type == 1) {
			if (input[current_len++] != 0x50) {
				return false;
			}
			if (input[current_len++] != 0x01) {
				return false;
			}
		}
		else {
			return false;
		}
		if (input[current_len++] != 0x05) {
			return false;
		}
		std::vector<uint8_t> sei_payload_size_data;
		while (input[current_len] == 0xff) {
			sei_payload_size_data.push_back(input[current_len++]);
		}
		sei_payload_size_data.push_back(input[current_len++]);
		uint64_t sei_payload_size_now = DeserializeData(sei_payload_size_data);
		if (sei_payload_size < sei_payload_size_now - UUID_LENGTH) {
			return false;
		}
		if (memcmp(&input[current_len], uuid_pico, UUID_LENGTH) != 0) {
			return false;
		}
		current_len += UUID_LENGTH;

		uint8_t* start = &input[current_len];

		sei_payload_size = sei_payload_size_now - UUID_LENGTH;
		current_len += sei_payload_size;
		while (current_len <= input_len - 4) {
			if (memcmp(&input[current_len], nalu_header_bytes, NALU_HEADER_LENGTH) == 0) {
				break;
			}
			else {
				current_len++;
			}
		}
		if (current_len > input_len - 4) {
			current_len = input_len;
		}

		uint8_t* end = &input[current_len];

		std::vector<uint8_t> sei_payload_deescape_data;
		DeescapeData(start, end, sei_payload_deescape_data);
		if (sei_payload_deescape_data.size() < sei_payload_size) {
			return false;
		}
		memcpy(sei_payload, sei_payload_deescape_data.data(), sei_payload_size);


		if (output_len < input_len - current_len) {
			return false;
		}
		output_len = input_len - current_len;
		memcpy(output, &input[current_len], input_len - current_len);
		return true;
	}
}