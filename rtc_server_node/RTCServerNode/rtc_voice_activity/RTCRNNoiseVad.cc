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

#define LOG_TAG "RTCRNNoiseVad"
#include <utils/Log.h>

#include <cstring>

#include <utils/Errors.h>
#include <media/stagefright/foundation/ALooper.h>

#include "../rtc_peer/RTCPeerInterface.h"
#include "../utils/RTCUtils.h"

#include "RTCRNNoiseVad.h"

//#define __DUMP_PCM__
#define __DUMP_STT_PCM__

namespace android {

RTCRNNoiseVad::RTCRNNoiseVad(RTCPeerInterface* peer, bool is_send_stt /*= true*/)
  : _peer(peer),
	_is_send_stt(is_send_stt) {

#ifdef __DUMP_PCM__
	RTCUtils::removeDumpFile("rnn_origin_dump.pcm");
#endif

#ifdef __DUMP_STT_PCM__
	RTCUtils::removeDumpFiles("rnn_vad_dump");
#endif 
}

RTCRNNoiseVad::~RTCRNNoiseVad() {
	if (_ds_ctx) {
		rnnoise_destroy(_ds_ctx);
	}
}

void RTCRNNoiseVad::setConfig(int32_t inSamplingRate, int32_t inChannels) {
	_in_sampling_rate = inSamplingRate;
	_in_channels = inChannels;

	_resampler.SetConfig(inSamplingRate, kSampling_16khz, kMonoChannel);

	_ds_ctx = rnnoise_create(nullptr);
}

void RTCRNNoiseVad::pushPCM(int16_t* pcm, int32_t samples) {
	if (!_ds_ctx) {
		return;
	}

#ifdef __DUMP_PCM__
	RTCUtils::dumpFile("rnn_origin_dump.pcm", (uint8_t*)pcm, samples * 2*2);	// 2 channel 16 bit
#endif

	aone::voice::RTCAOneResampler::DownmixInterleavedToMono(pcm, samples, kStereoChannel, _tmp_pcm_mono_buffer);

	int32_t samples_in_10ms = _in_sampling_rate / 100;
	int32_t n = samples / samples_in_10ms;

	for (int32_t i = 0; i < samples; i++) {
		_tmp_pcm_float_buffer[i] = static_cast<float>(_tmp_pcm_mono_buffer[i]);
	}

	float fret;
	float fmax = 0.0f;

	for (int32_t i = 0; i < n; i++) {
		int64_t nowMs = ALooper::GetNowUs()/1000;
		fret = rnnoise_process_frame(_ds_ctx,
			_tmp_pcm_float_buffer + (i*samples_in_10ms),
			_tmp_pcm_float_buffer + (i*samples_in_10ms));
//		ALOGI("kimi rnnoise_process_frame ret %f eleapsed %lld", fret, (ALooper::GetNowUs() / 1000) - nowMs);
		fmax = std::max(fmax, fret);
	}

	for (int i = 0; i < samples; ++i) {
		int32_t temp_val = static_cast<int32_t>(_tmp_pcm_float_buffer[i]);


		if (temp_val > 32767) {
			temp_val = 32767;
		}
		else if (temp_val < -32768) {
			temp_val = -32768;
		}
		_tmp_pcm_mono_buffer[i] = static_cast<int16_t>(temp_val);
	}

	for (int32_t i = 0; i < n; i++) {
		_resampler.Resample(_tmp_pcm_mono_buffer + (i*samples_in_10ms), samples_in_10ms,
			_pcm_16_mono + (i*kSampling_16khz_10ms), kSampling_16khz_10ms);
	}

	if (fmax >= 0.6f) {
		_continutiy_passives = 0;
		// speech
		if (!_is_speech) {
			_is_speech = true;
			_is_start_vad = true;
			if (_is_send_stt) {
				_send_stt_pcm(_pcm_16_mono, kSampling_16khz_10ms * n, VOICE_SPEECHING_STATE_START);
			}
			_debug_dump_file = "rnn_vad_dump_" + std::to_string(_debug_file_idx) + ".pcm";
			_debug_file_idx++;
		}
		else {
			if (_is_send_stt) {
				_send_stt_pcm(_pcm_16_mono, kSampling_16khz_10ms * n, VOICE_SPEECHING_STATE);
			}
		}

#ifdef __DUMP_STT_PCM__
		RTCUtils::dumpFile(_debug_dump_file.c_str(), (uint8_t*)_pcm_16_mono, kSampling_16khz_10ms*n*2);
#endif
	}
	else {
		// noise
		if (_is_speech) {
			_continutiy_passives++;
			if (_continutiy_passives > (kNonSpeechThreshold / 2)) {
				_is_speech = false;
				if (_is_send_stt) {
					_send_stt_pcm(nullptr, 0, VOICE_SPEECHING_STATE_END);
				}
			}
		}
	}
}

void RTCRNNoiseVad::_send_stt_pcm(int16_t *pcm, int32_t samples, VadSpeechingState speech_enum) {

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
				ALOGI("kimi_voice RTCRNNoiseVad start");
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
				ALOGI("kimi_voice RTCRNNoiseVad start");
				_pending_buffer->meta()->setString("enum", "start");
			}
			else {
				_pending_buffer->meta()->setString("enum", "speeching");
			}
			_peer->VoiceActive(_pending_buffer);
			_pending_buffer = nullptr;
		}

		ALOGI("kimi_voice RTCRNNoiseVad end");
		sp<ABuffer> buffer = new (std::nothrow) ABuffer(0);
		buffer->setRange(0, 0);
		buffer->meta()->setString("enum", "end");
		_peer->VoiceActive(buffer);
	}
}

}