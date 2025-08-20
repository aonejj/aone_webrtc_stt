//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_AONE_VOICE_CLIENT_LISTENER_H__
#define __RTC_AONE_VOICE_CLIENT_LISTENER_H__

#include <string>

namespace aone {
namespace voice {

class RTCAOneVoiceClientListener {
public:
	RTCAOneVoiceClientListener() = default;
	~RTCAOneVoiceClientListener() = default;

public:
	typedef enum {
		RTC_VOICE_STATE_INIT,
		RTC_VOICE_STATE_CONNECTING,
		RTC_VOICE_STATE_CONNECTED,
		RTC_VOICE_STATE_DISCONNECTED,
		RTC_VOICE_STATE_CLOSED,
		RTC_VOICE_STATE_FAILED,
	} RTCAOneVoiceConnectionState;

public:
	virtual void onVoiceSdp(std::string& sdp) = 0;
	virtual void onVoiceCandidate(int32_t mline_idx,
		std::string& mid, std::string& candidate) = 0;
	virtual void onVoiceConnectionState(RTCAOneVoiceConnectionState state) = 0;
	virtual void onVoiceError() = 0;
};

}
}


#endif // __RTC_AONE_VOICE_CLIENT_LISTENER_H__