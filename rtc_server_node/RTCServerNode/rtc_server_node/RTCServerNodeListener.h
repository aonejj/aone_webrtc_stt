//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_SERVER_NODE_LISTENER_H__
#define __RTC_SERVER_NODE_LISTENER_H__

namespace android {

class RTCServerNodeListener {
public:
	RTCServerNodeListener() = default;
	~RTCServerNodeListener() = default;

public:
	enum ServerNodeSignalState {
		S_NDOE_STATE_SIGNAL_INIT,
		S_NDOE_STATE_SIGNAL_OPENED,
		S_NODE_STATE_SIGNAL_CLOSED,
		S_NODE_STATE_SIGNAL_ERROR,
	};

public:
	virtual void OnRTCServerNodeSignalState(ServerNodeSignalState state) = 0;
};

}

#endif // __RTC_SERVER_NODE_LISTENER_H__