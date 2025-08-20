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

#include <utils/Errors.h>

#include "../rtc_peer/RTCPeerInterface.h"
#include "RTCVadInterface.h"

#include "RTCLegacyVad.h"
#include "RTCRNNoiseVad.h"

namespace android {

sp<RTCVadInterface> RTCVadInterface::CreateVad(RTCPeerInterface* peer, const bool isUsingLegacy, const bool is_send_stt /*= true*/) {
	sp<RTCVadInterface> vad = nullptr;
	if (isUsingLegacy) {
		vad = new RTCLegacyVad(peer, is_send_stt);
	}
	else {
		vad = new RTCRNNoiseVad(peer, is_send_stt);
	}

	
	return vad;
}

}