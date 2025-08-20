//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#define LOG_NDEBUG 1
#else
#define LOG_NDEBUG 0
#endif

#define LOG_TAG "RTCLegacyVAD"
#include <utils/Log.h>

#include <cstring>

#include <utils/Errors.h>

#include "../rtc_peer/RTCPeerInterface.h"
#include "../utils/RTCUtils.h"

#include "RTCLegacyVad.h"


//#define __DUMP_PCM__
#define __DUMP_STT_PCM__

namespace android {

RTCLegacyVad::RTCLegacyVad(RTCPeerInterface *peer, bool is_send_stt /*= true*/)
 : _peer(peer),
   _is_send_stt(is_send_stt) {

#ifdef __DUMP_PCM__
   RTCUtils::removeDumpFile("legacy_origin_dump.pcm");
#endif

#ifdef __DUMP_STT_PCM__
   RTCUtils::removeDumpFiles("legacy_vad_dump");
#endif
}

RTCLegacyVad::~RTCLegacyVad() {
}

void RTCLegacyVad::setConfig(int32_t inSamplingRate, int32_t inChannels) {
	_in_sampling_rate = inSamplingRate;
	_in_channels = inChannels;

	_legacy_vad.CreateVad();
	_resampler.SetConfig(inSamplingRate, kSampling_16khz, kMonoChannel);
}

void RTCLegacyVad::pushPCM(int16_t* pcm, int32_t samples) {
#ifdef __DUMP_PCM__
	RTCUtils::dumpFile("legacy_origin_dump.pcm", (uint8_t*)pcm, samples * 2 * 2);
#endif

	aone::voice::RTCAOneResampler::DownmixInterleavedToMono(pcm, samples, kStereoChannel, _tmp_mono_buffer);

	int32_t samples_in_10ms = _in_sampling_rate / 100;
	int32_t n = samples / samples_in_10ms;

	for (int32_t i = 0; i < n; i++) {
		size_t out_len = _resampler.Resample(_tmp_mono_buffer + (samples_in_10ms*i), samples_in_10ms,
			_pcm_16_mono, kSampling_16khz_10ms);
		_process_16khz_10ms(_pcm_16_mono);
	}
}

void RTCLegacyVad::_process_16khz_10ms(int16_t* pcm) {
//	RTC_CLASS_FUNCTION_TRACER("RTCLegacyVAD::_process_16khz_10ms", this);
	aone::voice::RTCAOneLegacyVad::VadActivity ret = _legacy_vad.VoiceActivity(pcm, kSampling_16khz_10ms, kSampling_16khz);
	if (ret == aone::voice::RTCAOneLegacyVad::VadActivity::kActive) {
		_continutiy_passives = 0;
		if (!_is_start_vad) {
			_is_speech = true;
			_is_start_vad = true;
			if (_is_send_stt) {
				_send_stt_pcm(pcm, kSampling_16khz_10ms, VOICE_SPEECHING_STATE_START);
			}
			_debug_dump_file = "legacy_vad_dump_" + std::to_string(_debug_file_idx) + ".pcm";
			_debug_file_idx++;
		}
		else {
			if (_is_send_stt) {
				_send_stt_pcm(pcm, kSampling_16khz_10ms, VOICE_SPEECHING_STATE);
			}
		}
#ifdef __DUMP_STT_PCM__
		RTCUtils::dumpFile(_debug_dump_file.c_str(), (uint8_t*)pcm, kSampling_16khz_10ms * 2);
#endif
	}
	else if (ret == aone::voice::RTCAOneLegacyVad::VadActivity::kPassive) {
		// noise
		if (_is_speech) {
			_continutiy_passives++;
			if (_continutiy_passives > kNonSpeechThreshold) {
				_is_speech = false;
				if (_is_send_stt) {
					_send_stt_pcm(nullptr, 0, VOICE_SPEECHING_STATE_END);
				}
			}
		}
	}
}

void RTCLegacyVad::_send_stt_pcm(int16_t *pcm, int32_t samples, VadSpeechingState speech_enum) {
	if (speech_enum == VOICE_SPEECHING_STATE_START || speech_enum == VOICE_SPEECHING_STATE) {
		if (_pending_buffer == nullptr) {
			_pending_buffer = new (std::nothrow) ABuffer(kMaxPcmPendingSize);
			_pending_buffer->setRange(0, 0);
		}
		size_t size = samples * 2;
		size_t cur_size = _pending_buffer->size();
		memcpy(_pending_buffer->data() + cur_size, (uint8_t*)pcm, size);
		cur_size += size;
		_pending_buffer->setRange(0, cur_size);
		if (cur_size == kMaxPcmPendingSize) {
			if (_is_start_vad) {
				_is_start_vad = false;
				ALOGI("kimi_voice RTCLegacyVad start");
				_pending_buffer->meta()->setString("enum", "start");
			}
			else {
				_pending_buffer->meta()->setString("enum", "speeching");
			}
			_peer->VoiceActive(_pending_buffer);
			_pending_buffer = nullptr;
		}
	}
	else {
		if (_pending_buffer != nullptr) {
			if (_is_start_vad) {
				_is_start_vad = false;
				ALOGI("kimi_voice RTCLegacyVad start");
				_pending_buffer->meta()->setString("enum", "start");
			}
			else {
				_pending_buffer->meta()->setString("enum", "speeching");
			}
			_peer->VoiceActive(_pending_buffer);
			_pending_buffer = nullptr;
		}

		ALOGI("kimi_voice RTCLegacyVad end");
		sp<ABuffer> buffer = new (std::nothrow) ABuffer(0);
		buffer->setRange(0, 0);
		buffer->meta()->setString("enum", "end");
		_peer->VoiceActive(buffer);
	}
}

}