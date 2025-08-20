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

#define LOG_TAG "RTCServerNode"

#include <utils/Log.h>
#include <media/stagefright/foundation/ADebug.h>

#include "RTCAOneSdk.h"

#include "../rtc_peer/RTCPeerFactory.h"
#include "RTCSignalingNodeSession.h"
#include "RTCServerNodeListener.h"
#include "../utils/AMessageWaitImpl.h"

#include "RTCServerNode.h"

namespace android {

sp<RTCServerNode> RTCServerNode::Create(RTCServerNodeListener* listener) {
	sp<RTCServerNode> node = new RTCServerNode(listener);
	if (node != nullptr) {
		if (node->_init() != OK) {
			node.clear();
			return nullptr;
		}
	}

	return node;
}

void RTCServerNode::LogToDebug(RTCLoggingSeverity min_sev) {

}

RTCServerNode::~RTCServerNode() {
	if (_looper != nullptr) {
		_looper->unregisterHandler(id());
		_looper->stop();
	}

	aone::voice::RTCAOneSdk::deinitialize();
}

status_t RTCServerNode::Start(std::string &host, int port) {
	sp<AMessage> msg = new AMessage(kWhatStart, this);
	msg->setString("host", host.c_str());
	msg->setInt32("port", port);
	msg->post();
	return OK;
}

status_t RTCServerNode::Stop() {
	sp<AMessage> msg = new AMessage(kWhatStop, this);
	sp<AMessage> response;

	return AMessageWaitImpl::PostAndAWaitResponse(msg, &response);
}


RTCServerNode::RTCServerNode(RTCServerNodeListener* listener)
	: _node_session(nullptr),
	_listener(listener) {

	aone::voice::RTCAOneSdk::initialize();

	_looper = new ALooper;
	_looper->setName("RTCServerNode");
	_looper->start();

	_vad_zmq_client = RTCVADZmqClient::Create();
}


status_t RTCServerNode::OnSignalingNodeSessionOpened() {
	sp<AMessage> msg = new AMessage(kWhatSignalingOpened, this);
	msg->post();
	return OK;
}

status_t RTCServerNode::OnSignalingNodeSessionClosed() {
	sp<AMessage> msg = new AMessage(kWhatSignalingClosed, this);
	msg->post();
	return OK;
}

status_t RTCServerNode::OnSignalingNodeSessionError() {
	sp<AMessage> msg = new AMessage(kWhatSignalingError, this);
	msg->post();
	return OK;
}

status_t RTCServerNode::OnAddPeer(std::string& rid) {
	sp<AMessage> msg = new AMessage(kWhatAddPeerReq, this);
	msg->setString("peer_id", rid.c_str());
	msg->post();
	return OK;
}

status_t RTCServerNode::OnRemovePeer(std::string& id) {
	sp<AMessage> msg = new AMessage(kWhatRemovePeerReq, this);
	msg->setString("peer_id", id.c_str());
	msg->post();
	return OK;
}

status_t RTCServerNode::OnPeerSDP(std::string& id, std::string& sdp) {
	sp<AMessage> msg = new AMessage(kWhatPeerSDP, this);
	msg->setString("peer_id", id.c_str());
	msg->setString("sdp", sdp.c_str());
	msg->post();
	return OK;
}

status_t RTCServerNode::OnPeerCandidate(std::string& id, int32_t mline_idx, 
										std::string& mid, std::string& candidate) {
	sp<AMessage> msg = new AMessage(kWhatPeerCandidate, this);
	msg->setString("peer_id", id.c_str());
	msg->setInt32("idx", mline_idx);
	msg->setString("mid", mid.c_str());
	msg->setString("candidate", candidate.c_str());
	msg->post();
	return OK;
}


void RTCServerNode::onMessageReceived(const sp<AMessage> &msg) {
	switch (msg->what()) {
	case kWhatInit:
		{
			_signal_state = RTCServerNodeListener::ServerNodeSignalState::S_NDOE_STATE_SIGNAL_INIT;
			_listener->OnRTCServerNodeSignalState(_signal_state);
			break;
		}
	case kWhatStart:
		{
			AString host;
			int port;
			CHECK(msg->findString("host", &host));
			CHECK(msg->findInt32("port", &port));

			_start_m(host, port);
			break;
		}
	case kWhatStop:
		{
			sp<AReplyToken> replyID;
			CHECK(msg->senderAwaitsResponse(&replyID));
			sp<AMessage> response = new AMessage;
			_stop_m();
			response->setInt32(AMessageWaitImpl::kErrorName, OK);
			response->postReply(replyID);
			break;
		}
	case kWhatSignalingOpened:
		{
			_signal_state = RTCServerNodeListener::ServerNodeSignalState::S_NDOE_STATE_SIGNAL_OPENED;
			_listener->OnRTCServerNodeSignalState(_signal_state);
			break;
		}
	case kWhatSignalingClosed:
		{
			_signal_state = RTCServerNodeListener::ServerNodeSignalState::S_NODE_STATE_SIGNAL_CLOSED;
			_listener->OnRTCServerNodeSignalState(_signal_state);
			break;
		}
	case kWhatSignalingError:
		{
			_signal_state = RTCServerNodeListener::ServerNodeSignalState::S_NODE_STATE_SIGNAL_ERROR;
			_listener->OnRTCServerNodeSignalState(_signal_state);
			break;
		}
	case kWhatAddPeerReq:
		{
			AString peer_id;
			CHECK(msg->findString("peer_id", &peer_id));
			std::string cmd("add_peer_res");
			std::string id(peer_id.c_str());

			if (_add_peer_m(id.c_str()) != OK) {
				std::string result("fail");
				_node_session->SenCommandRes(cmd, id, result);
			}
			else {
				std::string result("success");
				_node_session->SenCommandRes(cmd, id, result);
			}
			break;
		}
	case kWhatRemovePeerReq:
		{
			AString peer_id;
			CHECK(msg->findString("peer_id", &peer_id));
			std::string id(peer_id.c_str());
			std::string cmd("remove_peer_res");
			std::string result("success");
			_remove_peer_m(id.c_str());
			_node_session->SenCommandRes(cmd, id, result);
			break;
		}
	case kWhatPeerSDP:
		{
			AString peer_id;
			AString sdp;
			CHECK(msg->findString("peer_id", &peer_id));
			CHECK(msg->findString("sdp", &sdp));
			_on_peer_sdp_m(peer_id.c_str(), sdp.c_str());
			break;
		}
	case kWhatPeerCandidate:
		{
			AString peer_id;
			int32_t mline_idx;
			AString mid;
			AString candidate;
			CHECK(msg->findString("peer_id", &peer_id));
			CHECK(msg->findInt32("idx", &mline_idx));
			CHECK(msg->findString("mid", &mid));
			CHECK(msg->findString("candidate", &candidate));
			_on_peer_candidate_m(peer_id.c_str(), mline_idx, mid.c_str(), candidate.c_str());
			break;
		}
	}
}

status_t RTCServerNode::OnSDP(std::string &id, std::string &sdp) {
	_node_session->SendSDP(id, sdp);
	return OK;
}

status_t RTCServerNode::OnCandidate(std::string &id, int32_t mline_idx,
									std::string &mid, std::string &candidate)  {
	_node_session->SendCandidate(id, mline_idx, mid, candidate);
	return OK;
}

status_t RTCServerNode::OnConnectionState(std::string& id,
								aone::voice::RTCAOneVoiceClientListener::RTCAOneVoiceConnectionState state) {
	// TODO...
	return OK;
}

status_t RTCServerNode::OnVoiceActivity(std::string &id, sp<ABuffer> active_buffer) {
	_vad_zmq_client->SendVADPCM(id, active_buffer);
	return OK;
}

status_t RTCServerNode::_init() {
	_looper->registerHandler(this);
	_signal_state = RTCServerNodeListener::ServerNodeSignalState::S_NDOE_STATE_SIGNAL_INIT;
	sp<AMessage> msg = new AMessage(kWhatInit, this);
	msg->post();
	return OK;
}

void RTCServerNode::_reset() {

}

status_t RTCServerNode::_start_m(AString &host, int port) {
	if (_vad_zmq_client != nullptr) {
		_vad_zmq_client->Start();
	}

	_node_session = RTCSignalingNodeSession::Create(this);
	if (!_node_session) {
		sp<AMessage> msg = new AMessage(kWhatSignalingError, this);
		msg->post();
		return INVALID_OPERATION;
	}

	std::string hostStr(host.c_str());
	std::string pathStr("node");

	status_t ret = _node_session->Open(hostStr, port, pathStr);
	if (ret != OK) {
		_node_session.clear();
		_node_session = nullptr;
		sp<AMessage> msg = new AMessage(kWhatSignalingError, this);
		msg->post();
		return INVALID_OPERATION;
	}

	return OK;
}

void RTCServerNode::_stop_m() {
	if (_vad_zmq_client != nullptr) {
		_vad_zmq_client->Stop();
	}

	if (_node_session != nullptr) {
		_node_session->Close();
	}
}

status_t RTCServerNode::_add_peer_m(const char* peer_id) {
	std::string id(peer_id);
	sp<RTCPeerInterface> peer = RTCPeerFactory::CreateRTCPeer(id, _node_session->GetIceInfos(), this);

	if (peer == nullptr) {
		return INVALID_OPERATION;
	} 

	if (peer->CreateVoiceClient() != OK) {
		return INVALID_OPERATION;
	}

	_peers[id] = peer;

	return OK;
}

status_t RTCServerNode::_remove_peer_m(const char* peer_id) {
	std::string id(peer_id);
	auto it = _peers.find(id);
	if (it == _peers.end()) {
		return INVALID_OPERATION;
	}
	sp<RTCPeerInterface> rtc_peer = it->second;
	_peers.erase(it);
	rtc_peer->Close();

	return OK;
}

status_t RTCServerNode::_on_peer_sdp_m(const char* peer_id, const char* sdp) {
	std::string id(peer_id);
	std::string std_sdp(sdp);
	auto it = _peers.find(id);
	if (it == _peers.end()) {
		ALOGE("RTCServerNode::_on_peer_sdp_m not found peer");
		return INVALID_OPERATION;
	}

	if (it->second->SetSDP(std_sdp) != OK) {
		ALOGE("RTCServerNode::_on_peer_sdp_m SetSDP fail");
		return INVALID_OPERATION;
	}

	return OK;
}

status_t RTCServerNode::_on_peer_candidate_m(const char* peer_id, int32_t mline_idx,
											 const char* mid, const char* candidate) {
	std::string id(peer_id);
	std::string std_mid(mid);
	std::string std_candiate(candidate);

	auto it = _peers.find(id);
	if (it == _peers.end()) {
		ALOGE("RTCServerNode::_on_peer_candidate_m not found peer");
		return INVALID_OPERATION;
	}

	if (it->second->SetCandidate(mline_idx, std_mid, std_candiate) != OK) {
		ALOGE("RTCServerNode::_on_peer_sdp_m SetSDP fail");
		return INVALID_OPERATION;
	}

	return OK;
}

}