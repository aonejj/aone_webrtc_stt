//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_AONE_LEGACY_VAD_H__
#define __RTC_AONE_LEGACY_VAD_H__

// using webrtc legacy vad

#include <memory>

namespace aone {
namespace voice {

class RTCAOneLegacyVad {
public:
	RTCAOneLegacyVad();
	~RTCAOneLegacyVad();

public:
	enum VadActivity {
		kPassive = 0, 
		kActive = 1, 
		kError = -1,
	};

public:
	int32_t CreateVad();
	VadActivity VoiceActivity(const int16_t* audio,
		size_t num_samples,
		int sample_rate_hz);
	void Reset();

private:
	class Impl;
	std::unique_ptr<Impl> impl_;

private:
	RTCAOneLegacyVad(const RTCAOneLegacyVad&) = delete;
	RTCAOneLegacyVad& operator=(const RTCAOneLegacyVad&) = delete;
};

}
}

#endif //__RTC_AONE_LEGACY_VAD_H__