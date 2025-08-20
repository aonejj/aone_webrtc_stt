//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_PEER_H__
#define __RTC_PEER_H__

#include <string>
#include <vector>
#include <limits>

#include <utils/RefBase.h>
#include <utils/Errors.h>

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>

#include "RTCAOneVoiceClient.h"
#include "../rtc_voice_activity/RTCVadInterface.h"


#include "RTCPeerInterface.h"
#include "RTCPeerListener.h"

#include "../rtc_server_node/RTCIceServerInfo.h"

namespace android {

class RTCPeerRtpReceiverInterface;
class RTCPeer : public RTCPeerInterface,
				public AHandler,
				public aone::voice::RTCAOneVoiceClientListener
{
public:
	static sp<RTCPeerInterface> Create(std::string &id, std::vector<RTCIceServerInfo>& rtcIceInfos,
									   RTCPeerListener* listener);
	~RTCPeer();

public:
	// RTCPeerInterface methode
	status_t CreateVoiceClient() override;
	std::string& GetId() override;
	status_t SetSDP(std::string &sdp) override;
	status_t SetCandidate(int32_t mline_idx, 
						  std::string &mid, std::string &candidate) override;
	status_t Close() override;

	RTCPeerInterface::RTCPeerState GetState() override;

	status_t VoiceActive(sp<ABuffer> active_buffer) override;

private:
	typedef enum {
		kWhatCreateVoiceClient,
		kWhatSetRemoteDescription,
		kWhatRemoteCandidate,
		kWhatClose,
		kWhatError,
	} RTCPeerMsg;

private:
	// AHandler methods
	void onMessageReceived(const sp<AMessage> &msg) override;


	void onVoiceSdp(std::string& sdp) override;
	void onVoiceCandidate(int32_t mline_idx,
		std::string& mid, std::string& candidate) override;
	void onVoiceConnectionState(aone::voice::RTCAOneVoiceClientListener::RTCAOneVoiceConnectionState state) override;
	void onVoiceError() override;

private:
	RTCPeer(std::string &id, std::vector<RTCIceServerInfo>& rtcIceInfos, 
		    RTCPeerListener* listener);

private:
	status_t _init();

	status_t _create_voice_client_m();
	status_t _set_offer_description_m(AString &sdp);
	status_t _set_remote_candidate_m(int32_t mline_idx, AString &mid, AString &candidate);
	status_t _close_m();
	void _on_pcm_data(const int16_t* data, size_t samples, 	int sample_rate, int channels);

private:
	sp<ALooper> _looper;
	std::string _id;
	RTCPeerListener* _listener;

	std::vector<aone::voice::RTCAOneVoiceClient::RTCIceServerInfo> _ice_servers;
	std::unique_ptr<aone::voice::RTCAOneVoiceClient> _voice_client;

	RTCPeerInterface::RTCPeerState _state;

	bool _is_first_pcm = false;
	sp< RTCVadInterface> _vad;

private:
	DISALLOW_EVIL_CONSTRUCTORS(RTCPeer);
};

}

#endif // __RTC_PEER_H__