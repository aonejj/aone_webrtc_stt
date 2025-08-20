//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_VAD_INTERFACE_H__
#define __RTC_VAD_INTERFACE_H__

#include <utils/RefBase.h>

namespace android {

const int32_t kMaxPcmBuffer = 2000;
const int32_t kSampling_16khz = 16000;
const int32_t kSampling_16khz_10ms = kSampling_16khz / 100;
const int32_t kMaxPcmPendingSize = kSampling_16khz_10ms * sizeof(int16_t) * 6;
const int32_t kNonSpeechThreshold = 200;

const float kScale = 1.0f / 32768.0f;

const int32_t kMonoChannel = 1;
const int32_t kStereoChannel = 2;

class RTCPeerInterface;

class RTCVadInterface : public RefBase {
public:
	RTCVadInterface() = default;
	~RTCVadInterface() = default;

public:
	static sp<RTCVadInterface> CreateVad(RTCPeerInterface* peer, const bool isUsingLegacy, const bool is_send_stt = true);

public:
	virtual void setConfig(int32_t inSamplingRate, int32_t inChannels) = 0;
	virtual void pushPCM(int16_t* pcm, int32_t samples) = 0;

protected:
	typedef enum {
		VOICE_SPEECHING_STATE_START,
		VOICE_SPEECHING_STATE,
		VOICE_SPEECHING_STATE_END,
	} VadSpeechingState;
};

}

#endif	//__RTC_VAD_INTERFACE_H__