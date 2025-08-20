//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_RNNOISE_VAD_H__
#define __RTC_RNNOISE_VAD_H__

#include <utils/RefBase.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>

#include "RTCAOneResampler.h"

#include "RTCVadInterface.h"

#include "rnnoise.h"

namespace android {

class RTCPeerInterface;
class RTCRNNoiseVad : public RTCVadInterface {
public:
	RTCRNNoiseVad(RTCPeerInterface* peer, bool is_send_stt = true);
	~RTCRNNoiseVad();

public:
	void setConfig(int32_t inSamplingRate, int32_t inChannels) override;
	void pushPCM(int16_t* pcm, int32_t samples) override;

private:
	void _send_stt_pcm(int16_t *pcm, int32_t samples, VadSpeechingState speech_enum);

private:
	int32_t _in_sampling_rate;
	int32_t _in_channels;

	RTCPeerInterface* _peer;
	bool _is_send_stt;

	aone::voice::RTCAOneResampler _resampler;

	sp<ABuffer> _pending_buffer = nullptr;
	DenoiseState* _ds_ctx = nullptr;

	int16_t _tmp_pcm_mono_buffer[kMaxPcmBuffer];
	float _tmp_pcm_float_buffer[kMaxPcmBuffer];
	int16_t _pcm_16_mono[kMaxPcmBuffer];
	bool _is_speech = false;
	bool _is_start_vad = false;
	uint32_t _continutiy_passives = 0;
	
	uint32_t _debug_file_idx = 1;
	std::string _debug_dump_file;

private:
	DISALLOW_EVIL_CONSTRUCTORS(RTCRNNoiseVad);
};

}

#endif // __RTC_RNNOISE_VAD_H__