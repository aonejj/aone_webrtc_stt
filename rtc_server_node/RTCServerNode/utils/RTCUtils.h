//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

namespace android {

class RTCUtils {
public:
	RTCUtils() = default;
	~RTCUtils() = default;

public:
	static void dumpFile(const char* fname, uint8_t* buf, int32_t len);
	static void removeDumpFile(const char* fname);
	static void removeDumpFiles(const char* fname);
	static void int16_to_float(const int16_t* input, float* output, size_t length);
	static void float_to_int16(const float* input, int16_t* output, size_t length);
};

}