//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_PEER_INTERFACE_H__
#define __RTC_PEER_INTERFACE_H__

#include <string>

#include <utils/RefBase.h>
#include <media/stagefright/foundation/ABuffer.h>


namespace android {

class RTCPeerInterface : virtual public RefBase {
public:
	typedef enum {
		RTC_PEER_STATE_UNINIT,
		RTC_PEER_STATE_INIT,
		RTC_PEER_STATE_CREATE_VOICE_CLIENT,
		RTC_PEER_STATE_SET_OFFER_DESCRIPTION,
		RTC_PEER_STATE_CREATE_ANSWER,
		RTC_PEER_STATE_SET_ANSWER_DESCRIPTION,
		RTC_PEER_STATE_ERROR,
	} RTCPeerState;

public:
	RTCPeerInterface() = default;
	~RTCPeerInterface() = default;

public:
	virtual status_t CreateVoiceClient() = 0;
	virtual std::string& GetId() = 0;
	virtual status_t SetSDP(std::string &sdp) = 0;
	virtual status_t SetCandidate(int32_t mline_idx, std::string &mid, std::string &candidate) = 0;
	virtual status_t Close() = 0;
	virtual RTCPeerState GetState() = 0;

	virtual status_t VoiceActive(sp<ABuffer> active_buffer) = 0;;
};
	
}

#endif // __RTC_PEER_INTERFACE_H__