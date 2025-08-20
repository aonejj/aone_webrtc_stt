//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_SIGNALING_NODE_SESSION_H__
#define __RTC_SIGNALING_NODE_SESSION_H__

#include <string>
#include <vector>
#include <limits>

#include <utils/RefBase.h>
#include <utils/Errors.h>

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>

#include <jansson.h>

#include "../signaling_session/RTCWebSocketClientListener.h"
#include "RTCIceServerInfo.h"

namespace android {

class RTCWebSocketClient;
class RTCSignalingNodeSessionListener;

class RTCSignalingNodeSession : public AHandler,
								public RTCWebSocketClientListener {
public:
	virtual ~RTCSignalingNodeSession();

public:
	static sp<RTCSignalingNodeSession> Create(RTCSignalingNodeSessionListener *listener);

public:
	status_t Open(std::string &host, int port, std::string &path);
	status_t Close();

	std::vector<RTCIceServerInfo>& GetIceInfos();

	status_t SenCommandRes(std::string &cmd, std::string &id, std::string &result);
	status_t SendSDP(std::string &id, std::string &sdp);
	status_t SendCandidate(std::string &id, int32_t index_line, std::string &mid, std::string &candidate);
	status_t SendIceState(std::string &id, int32_t state, std::string &desc);
private:
	// RTCWebSocketClientListener methods
	status_t onWSConnected() override;
	status_t onWSDisconnected() override;
	status_t onWSError() override;
	status_t onWSMessage(std::string &ws_msg) override;

	// AHandler methods
	void onMessageReceived(const sp<AMessage> &msg) override;

private:
	typedef enum {
		RTC_SIGNAL_SESSION_UNINT,
		RTC_SIGNAL_SESSION_INT,
		RTC_SIGNAL_SESSION_CONNECTED,
		RTC_SIGNAL_SESSION_DISCONNECTED,
		RTC_SIGNAL_SESSION_ERROR,
	} RTCSignalSessionState;

	// message enum
	enum {
		kWhatOpen,
		kWhatClose,
		kWhatError,
		kWhatWSConnected,
		kWhatWSDisconnected,
		kWhatWSError,
		kWhatWSMessage,
		kWhatSendCommandRes,
		kWhatSendSDP,
		kWhatSendCandidate,
		kWhatSendIceState,
	};

private:
	RTCSignalingNodeSession(RTCSignalingNodeSessionListener *listener);

private:
	status_t _init();
	void	 _reset();

	status_t _open_m(AString &host, int port, AString &path);
	void _close_m();
	status_t _regist_req_m();
	status_t _received_msg_m(char *pmsg);
	
	status_t _send_command_res(const char* cmd, const char* id, const char* result);
	status_t _send_sdp(const char *id, const char *sdp);
	status_t _send_candidate(const char *id, int32_t mline_index, const char* mid, const char* candidate);
	status_t _send_ice_state(const char *id, int32_t state, const char* desc);

private:
	sp<ALooper>				_looper;
	sp<RTCWebSocketClient> _ws_client;
	RTCSignalingNodeSessionListener* _listener;
	std::string _id;
	std::vector<RTCIceServerInfo> _ice_infos;
	RTCSignalSessionState _state;

private:
	DISALLOW_EVIL_CONSTRUCTORS(RTCSignalingNodeSession);
};

}

#endif	// __RTC_SIGNALING_NODE_SESSION_H__