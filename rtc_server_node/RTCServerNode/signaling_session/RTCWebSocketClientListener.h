//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_WEBSOCKET_CLIENT_LISTENER_H__
#define __RTC_WEBSOCKET_CLIENT_LISTENER_H__

namespace android {

class RTCWebSocketClientListener {
public:
	RTCWebSocketClientListener() = default;
	~RTCWebSocketClientListener() = default;

public:
	virtual status_t onWSConnected() = 0;
	virtual status_t onWSDisconnected() = 0;
	virtual status_t onWSError() = 0;
	virtual status_t onWSMessage(std::string &ws_msg) = 0;
};

}

#endif // __RTC_WEBSOCKET_CLIENT_LISTENER_H__