//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_LEGACY_VAD_H__
#define __RTC_LEGACY_VAD_H__

#include <memory>
#include <deque>
#include <limits>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>

#include "RTCAOneLegacyVad.h"
#include "RTCAOneResampler.h"


#include "RTCVadInterface.h"

namespace android {

class RTCPeerInterface;
class RTCLegacyVad : public RTCVadInterface {
public:
	RTCLegacyVad(RTCPeerInterface* peer, bool is_send_stt = true);
	~RTCLegacyVad();

public:
	void setConfig(int32_t inSamplingRate, int32_t inChannels) override;
	void pushPCM(int16_t* pcm, int32_t samples) override;

private:
	void _process_16khz_10ms(int16_t* pcm);
	void _send_stt_pcm(int16_t *pcm, int32_t samples, VadSpeechingState speech_enum);

private:
	aone::voice::RTCAOneLegacyVad _legacy_vad;
	aone::voice::RTCAOneResampler _resampler;

	uint32_t _continutiy_passives = 0;
	bool _is_speech = false;
	RTCPeerInterface* _peer;
	bool _is_send_stt;

	sp<ABuffer> _pending_buffer = nullptr;
	bool		_is_start_vad = false;

	int32_t _in_sampling_rate;
	int32_t _in_channels;

	int16_t _tmp_mono_buffer[kMaxPcmBuffer];
	int16_t _pcm_16_mono[kMaxPcmBuffer];

	uint32_t _debug_file_idx = 1;
	std::string _debug_dump_file;

private:
	DISALLOW_EVIL_CONSTRUCTORS(RTCLegacyVad);
};

}

#endif	// __RTC_LEGACY_VAD_H__