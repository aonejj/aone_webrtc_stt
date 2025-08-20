//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_SIGNALING_NODE_SESSION_LISTENER_H__
#define __RTC_SIGNALING_NODE_SESSION_LISTENER_H__

#include <string>
#include <utils/Errors.h>

namespace android {

class RTCSignalingNodeSessionListener {
public:
	RTCSignalingNodeSessionListener() = default;
	~RTCSignalingNodeSessionListener() = default;

public:
	virtual status_t OnSignalingNodeSessionOpened() = 0;
	virtual status_t OnSignalingNodeSessionClosed() = 0;
	virtual status_t OnSignalingNodeSessionError() = 0;

	virtual status_t OnAddPeer(std::string& id) = 0;
	virtual status_t OnRemovePeer(std::string& id) = 0;
	virtual status_t OnPeerSDP(std::string& id, std::string& sdp) = 0;
	virtual status_t OnPeerCandidate(std::string& id, int32_t mline_idx, std::string& mid, std::string& candidate) = 0;
};

}

#endif	// __RTC_SIGNALING_NODE_SESSION_LISTENER_H__