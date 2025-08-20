//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <cstring>
#include <cmath>

#include "RTCUtils.h"
#include "../rtc_voice_activity/RTCVadInterface.h"

namespace android {

void RTCUtils::dumpFile(const char* fname, uint8_t* buf, int32_t len) {
		FILE* fp = fopen(fname, "ab");
	if (!fp) {
		perror("fopen");
		return;
	}

	size_t written = fwrite(buf, sizeof(uint8_t), len, fp);
	if (written != (size_t)len) {
		fprintf(stderr, "Warning: fwrite wrote %zu samples, expected %d\n", written, len);
	}

	fclose(fp);
}

void RTCUtils::removeDumpFile(const char* fname) {
	remove(fname);
}

void RTCUtils::removeDumpFiles(const char* fname) {
	std::vector<std::filesystem::path> to_remove;

	for (const auto& entry : std::filesystem::directory_iterator(".")) {
		if (entry.is_regular_file()) {
			std::string filename = entry.path().filename().string();
			if (filename.rfind(fname, 0) == 0) {
				to_remove.push_back(entry.path());
			}
		}
	}

	for (const auto& path : to_remove) {
		std::filesystem::remove(path);
	}
}

void RTCUtils::int16_to_float(const int16_t* input, float* output, size_t length) {
	for (size_t i = 0; i < length; ++i) {
		output[i] = static_cast<float>(input[i]) * kScale;
	}
}

void RTCUtils::float_to_int16(const float* input, int16_t* output, size_t length) {
	for (size_t i = 0; i < length; ++i) {
		float sample = input[i];

		if (sample > 1.0f) sample = 1.0f;
		if (sample < -1.0f) sample = -1.0f;

		sample = std::round(sample * 32768.0f);

		if (sample > 32767.0f) sample = 32767.0f;
		if (sample < -32768.0f) sample = -32768.0f;

		output[i] = static_cast<int16_t>(sample);
	}
}

}