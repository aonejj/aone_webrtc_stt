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

#define LOG_TAG "RTCPeer"
#include <utils/Log.h>

#include <media/stagefright/foundation/ADebug.h>

#include "../utils/AMessageWaitImpl.h"
#include "RTCPeer.h"


#define _ENABLE_SDP_LOG__

const bool kIsUsingLegacyVad = false;

namespace android {

sp<RTCPeerInterface> RTCPeer::Create(std::string &id, std::vector<RTCIceServerInfo>& rtcIceInfos,
									 RTCPeerListener* listener) {
	sp<RTCPeerInterface> peer = new RTCPeer(id, rtcIceInfos, listener);
	if (peer != nullptr) {
		if (((RTCPeer*)peer.get())->_init() != OK) {
			peer.clear();
			return nullptr;
		}
	}

	return peer;
}

RTCPeer::RTCPeer(std::string &id, std::vector<RTCIceServerInfo>& rtcIceInfos,
			     RTCPeerListener* listener) 
 : _id(id), 
   _listener(listener),
	_voice_client(nullptr) {
	_looper = new ALooper;
	_looper->setName("RTCPeer");
	_looper->start();

	_state = RTC_PEER_STATE_UNINIT;

	for (auto it : rtcIceInfos) {
		aone::voice::RTCAOneVoiceClient::RTCIceServerInfo info;
		info._is_stun = it._is_stun;
		info._uri = it._uri;
		info._user_name = it._user_name;
		info._credential = it._credential;
		_ice_servers.push_back(info);
	}
}

RTCPeer::~RTCPeer() {
	if (_looper != nullptr) {
		_looper->unregisterHandler(id());
		_looper->stop();
	}
}

// RTCPeerInterface methode
status_t RTCPeer::CreateVoiceClient() {
	sp<AMessage> msg = new AMessage(kWhatCreateVoiceClient, this);
	sp<AMessage> response;

	return AMessageWaitImpl::PostAndAWaitResponse(msg, &response);
}

std::string& RTCPeer::GetId() {
	return _id;
}

status_t RTCPeer::SetSDP(std::string &sdp) {
	sp<AMessage> msg = new AMessage(kWhatSetRemoteDescription, this);
	msg->setString("sdp", sdp.c_str());
	msg->post();
	return OK;
}

status_t RTCPeer::SetCandidate(int32_t mline_idx,
							   std::string &mid, std::string &candidate) {
	sp<AMessage> msg = new AMessage(kWhatRemoteCandidate, this);
	msg->setString("candidate", candidate.c_str());
	msg->setInt32("midIdx", mline_idx);
	msg->setString("mid", mid.c_str());
	msg->post();

	return OK;
}

status_t RTCPeer::Close() {
	sp<AMessage> msg = new AMessage(kWhatClose, this);
	sp<AMessage> response;

	return AMessageWaitImpl::PostAndAWaitResponse(msg, &response);
}

RTCPeerInterface::RTCPeerState RTCPeer::GetState() {
	return _state;
}

status_t RTCPeer::VoiceActive(sp<ABuffer> active_buffer) {
	return _listener->OnVoiceActivity(_id, active_buffer);
}

// AHandler methods
void RTCPeer::onMessageReceived(const sp<AMessage> &msg) {
	switch (msg->what()) {
	case kWhatCreateVoiceClient:
		{
			ALOGD("kWhatCreateVoiceClient");
			sp<AReplyToken> replyID;
			CHECK(msg->senderAwaitsResponse(&replyID));
			sp<AMessage> response = new AMessage;
			status_t ret = _create_voice_client_m();
			if (ret != OK) {
				_state = RTC_PEER_STATE_ERROR;
			}

			response->setInt32(AMessageWaitImpl::kErrorName, ret);
			response->postReply(replyID);
			break;
		}
	case kWhatSetRemoteDescription:
		{
			AString sdp;
			CHECK(msg->findString("sdp", &sdp));

			status_t sret = _set_offer_description_m(sdp);
			if (sret != OK) {
				sp<AMessage> msg = new AMessage(kWhatError, this);
				msg->setInt32("last_state", (int32_t)_state);
				msg->setString("desc", "setOffer");
				msg->post();
				_state = RTC_PEER_STATE_ERROR;
				break;
			}

			_state = RTC_PEER_STATE_SET_OFFER_DESCRIPTION;
			break;
		}
	case kWhatRemoteCandidate:
		{
			int32_t mline_idx;
			AString candidate;
			AString mid;
			CHECK(msg->findInt32("midIdx", &mline_idx));
			CHECK(msg->findString("mid", &mid));
			CHECK(msg->findString("candidate", &candidate));

			_set_remote_candidate_m(mline_idx, mid, candidate);

			break;
		}
	case kWhatClose:
		{
			sp<AReplyToken> replyID;
			CHECK(msg->senderAwaitsResponse(&replyID));
			sp<AMessage> response = new AMessage;
			status_t ret = _close_m();
			if (ret != OK) {
				_state = RTC_PEER_STATE_ERROR;
			}

			response->setInt32(AMessageWaitImpl::kErrorName, ret);
			response->postReply(replyID);
			break;
		}
	case kWhatError:
		{
			break;
		}
	}
}

void RTCPeer::onVoiceSdp(std::string& sdp) {
	_listener->OnSDP(_id, sdp);
}

void RTCPeer::onVoiceCandidate(int32_t mline_idx,
												  std::string& mid, std::string& candidate) {
	_listener->OnCandidate(_id, mline_idx, mid, candidate);
}

void RTCPeer::onVoiceConnectionState(aone::voice::RTCAOneVoiceClientListener::RTCAOneVoiceConnectionState state) {
	_listener->OnConnectionState(_id, state);
}

void RTCPeer::onVoiceError() {

}

status_t RTCPeer::_init() {
	_looper->registerHandler(this);
	_state = RTCPeerInterface::RTC_PEER_STATE_INIT;
	return OK;
}

status_t RTCPeer::_create_voice_client_m() {
	_voice_client = aone::voice::RTCAOneVoiceClient::Create(_ice_servers, this);
	if (_voice_client == nullptr) {
		return INVALID_OPERATION;
	}

	if (_voice_client->Open()) {
		return INVALID_OPERATION;
	}

	_voice_client->SetPCMCallback(
		[this](const int16_t* data, size_t samples, int sample_rate, int channels) {
			this->_on_pcm_data(data, samples, sample_rate, channels);
		}
	);

	_state = RTC_PEER_STATE_CREATE_VOICE_CLIENT;

	return OK;
}

status_t RTCPeer::_set_offer_description_m(AString &sdp) {
	if (!_voice_client) {
		return INVALID_OPERATION;
	}

	if (_voice_client->SetRemoteDescription(sdp.c_str())) {
		return INVALID_OPERATION;
	}

	return OK;
}


status_t RTCPeer::_set_remote_candidate_m(int32_t mline_idx, AString &mid, AString &candidate) {
	if (_voice_client == nullptr) {
		return INVALID_OPERATION;
	}

	_voice_client->SetRemoteIceCandidate(mline_idx, mid.c_str(), candidate.c_str());

	return OK;
}

status_t RTCPeer::_close_m() {
	if (_voice_client) {
		_voice_client->SetPCMCallback(nullptr);
		_voice_client->Close();
		_voice_client = nullptr;
	}

	return OK;
}

void RTCPeer::_on_pcm_data(const int16_t* data, size_t samples, int sample_rate, int channels) {
	if (!_is_first_pcm) {
		_vad = RTCVadInterface::CreateVad(this, kIsUsingLegacyVad);
		_vad->setConfig(sample_rate, channels);
		_is_first_pcm = true;
	}

	if (_vad) {
		_vad->pushPCM(const_cast<int16_t*>(data), samples);
	}
}

}