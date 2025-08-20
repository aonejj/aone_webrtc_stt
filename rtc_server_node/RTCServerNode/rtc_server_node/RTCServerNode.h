//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_SERVER_NODE_H__
#define __RTC_SERVER_NODE_H__

#include <string>
#include <unordered_map>

#include <utils/RefBase.h>
#include <utils/Errors.h>

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>

#include "RTCSignalingNodeSessionListener.h"
#include "../rtc_peer/RTCPeerListener.h"
#include "../rtc_peer/RTCPeerInterface.h"
#include "../rtc_zmq_client/RTCVADZmqClient.h"

namespace android {

class RTCServerNodeListener;
class RTCSignalingNodeSession;

class RTCServerNode : public AHandler,
					  public RTCSignalingNodeSessionListener,
					  public RTCPeerListener {
public:
	virtual ~RTCServerNode();

public:
	enum RTCLoggingSeverity {
		RTC_VERBOSE,
		RTC_INFO,
		RTC_WARNING,
		RTC_ERROR,
		RTC_NONE
	};

public:
	static sp<RTCServerNode> Create(RTCServerNodeListener* listener);
	static void LogToDebug(RTCLoggingSeverity min_sev);

public:
	status_t Start(std::string &host, int port);
	status_t Stop();

private:
	RTCServerNode(RTCServerNodeListener* listener);

private:
	// RTCSignalingNodeSessionListener method
	status_t OnSignalingNodeSessionOpened() override;
	status_t OnSignalingNodeSessionClosed() override;
	status_t OnSignalingNodeSessionError() override;

	status_t OnAddPeer(std::string& rid) override;
	status_t OnRemovePeer(std::string& id) override;
	status_t OnPeerSDP(std::string& id, std::string& sdp) override;
	status_t OnPeerCandidate(std::string& id, int32_t mline_idx, std::string& mid, std::string& candidate) override;


	// AHandler methods
	void onMessageReceived(const sp<AMessage> &msg) override;

	// RTCPeerListener methods
	status_t OnSDP(std::string &id, std::string &sdp) override;
	status_t OnCandidate(std::string &id, int32_t mline_idx,
		std::string &mid, std::string &candidate) override;
	status_t OnConnectionState(std::string& id, aone::voice::RTCAOneVoiceClientListener::RTCAOneVoiceConnectionState state) override;
	status_t OnVoiceActivity(std::string &id, sp<ABuffer> active_buffer) override;

private:
	// message enum
	enum {
		kWhatInit,
		kWhatStart,
		kWhatStop,
		kWhatSignalingOpened,
		kWhatSignalingClosed,
		kWhatSignalingError,
		kWhatAddPeerReq,
		kWhatRemovePeerReq,
		kWhatPeerSDP,
		kWhatPeerCandidate,
	};

private:
	status_t _init();
	void	 _reset();

	status_t _start_m(AString &host, int port);
	void _stop_m();
	status_t _add_peer_m(const char* peer_id);
	status_t _remove_peer_m(const char* peer_id);
	status_t _on_peer_sdp_m(const char* peer_id, const char* sdp);
	status_t _on_peer_candidate_m(const char* peer_id, int32_t mline_idx, const char* mid, const char* candidate);

private:
	sp<ALooper>					_looper;
	sp<RTCSignalingNodeSession>	_node_session;
	RTCServerNodeListener* _listener;
	RTCServerNodeListener::ServerNodeSignalState _signal_state;
	std::unordered_map<std::string, sp<RTCPeerInterface>> _peers;
	sp<RTCVADZmqClient> _vad_zmq_client;

private:
	DISALLOW_EVIL_CONSTRUCTORS(RTCServerNode);
};

}


#endif // __RTC_SERVER_NODE_H__