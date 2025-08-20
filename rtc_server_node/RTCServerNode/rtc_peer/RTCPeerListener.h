//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_PEER_LISTENER_H__
#define __RTC_PEER_LISTENER_H__

#include <string>

#include <utils/RefBase.h>
#include <media/stagefright/foundation/ABuffer.h>

#include "RTCAOneVoiceClientListener.h"


namespace android {

class RTCPeerListener {
public:
	RTCPeerListener() = default;
	~RTCPeerListener() = default;

public:
	virtual status_t OnSDP(std::string &id, std::string &sdp) = 0;
	virtual status_t OnCandidate(std::string &id, int32_t mline_idx, 
								 std::string &mid, std::string &candidate) = 0;
	virtual status_t OnConnectionState(std::string& id, aone::voice::RTCAOneVoiceClientListener::RTCAOneVoiceConnectionState state) = 0;
	virtual status_t OnVoiceActivity(std::string &id, sp<ABuffer> active_buffer) = 0;
};

}

#endif // __RTC_PEER_LISTENER_H__