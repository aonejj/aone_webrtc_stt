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

#define LOG_TAG "RTCSignalingNodeSession"

#include <utils/Log.h>
#include <media/stagefright/foundation/ADebug.h>

#include "RTCSignalingNodeSessionListener.h"
#include "../signaling_session/RTCWebSocketClient.h"
#include "RTCSignalingNodeSession.h"

// attibute
const char kCommandAttr[] = "cmd";
const char kIdAttr[] = "id";
const char kResultAttr[] = "result";
const char kIceInfoAttr[] = "iceinfo";
const char kSdpAttr[] = "sdp";
const char kCandidateAttr[] = "candidate";
const char kMLineIndexAttr[] = "mline_index";
const char kMIdAttr[] = "mid";
const char kUrlAttr[] = "url";
const char kUserNameAttr[] = "username";
const char kCredentialAttr[] = "credential";
const char kIceStateAttr[] = "state";
const char kIceStateDescAttr[] = "desc";

// commands
const char kRegistMsg[] = "regist";
const char kRegistMsgRes[] = "regist_res";

const char kAddPeerMsg[] = "add_peer";
const char kAddPeerMsgRes[] = "add_peer_res";

const char kRemovePeerMsg[] = "remove_peer";
const char kRemovePeerMsgRes[] = "remove_peer_res";

const char kOfferMsg[] = "offer";
const char kAnswerMsg[] = "answer";
const char kCandidateMsg[] = "candidate";
const char kIceStateMsg[] = "iceState";

// result
const char kSuccess[] = "success";
const char kFail[] = "fail";



namespace android {

sp<RTCSignalingNodeSession> RTCSignalingNodeSession::Create(RTCSignalingNodeSessionListener *listener) {
	sp<RTCSignalingNodeSession> node_session = new RTCSignalingNodeSession(listener);
	if (node_session != nullptr) {
		if (node_session->_init() != OK) {
			node_session.clear();
			return nullptr;
		}
	}

	return node_session;
}

RTCSignalingNodeSession::~RTCSignalingNodeSession() {
	if (_looper != nullptr) {
		_looper->unregisterHandler(id());
		_looper->stop();
	}
}

status_t RTCSignalingNodeSession::Open(std::string &host, int port, std::string &path) {
	sp<AMessage> msg = new AMessage(kWhatOpen, this);
	msg->setString("host", host.c_str());
	msg->setInt32("port", port);
	if (!path.empty()) {
		msg->setString("path", path.c_str());
	}
	msg->post();
	return OK;
}

status_t RTCSignalingNodeSession::Close() {
	sp<AMessage> msg = new AMessage(kWhatClose, this);
	msg->post();
	return OK;
}

std::vector<RTCIceServerInfo>& RTCSignalingNodeSession::GetIceInfos() {
	return _ice_infos;
}

status_t RTCSignalingNodeSession::SenCommandRes(std::string &cmd, std::string &id, std::string &result) {
	sp<AMessage> msg = new AMessage(kWhatSendCommandRes, this);
	msg->setString("cmd", cmd.c_str());
	msg->setString("id", id.c_str());
	msg->setString("result", result.c_str());
	msg->post();

	return OK;
}

status_t RTCSignalingNodeSession::SendSDP(std::string &id, std::string &sdp) {
	sp<AMessage> msg = new AMessage(kWhatSendSDP, this);
	msg->setString("id", id.c_str());
	msg->setString("sdp", sdp.c_str());
	msg->post();

	return OK;
}

status_t RTCSignalingNodeSession::SendCandidate(std::string &id, int32_t index_line, 
											    std::string &mid, std::string &candidate) {
	sp<AMessage> msg = new AMessage(kWhatSendCandidate, this);
	msg->setString("id", id.c_str());
	msg->setInt32("idx", index_line);
	msg->setString("mid", mid.c_str());
	msg->setString("candidate", candidate.c_str());
	msg->post();

	return OK;
}

status_t RTCSignalingNodeSession::SendIceState(std::string &id, int32_t state, std::string &desc) {
	sp<AMessage> msg = new AMessage(kWhatSendIceState, this);
	msg->setString("id", id.c_str());
	msg->setInt32("state", state);
	msg->setString("desc", desc.c_str());
	msg->post();

	return OK;
}

RTCSignalingNodeSession::RTCSignalingNodeSession(RTCSignalingNodeSessionListener *listener)
	: _ws_client(nullptr),
	_listener(listener) {

	_looper = new ALooper;
	_looper->setName("RTCSignalingNodeSession");
	_looper->start();
	_state = RTC_SIGNAL_SESSION_UNINT;
}


status_t RTCSignalingNodeSession::onWSConnected() {
	sp<AMessage> msg = new AMessage(kWhatWSConnected, this);
	msg->post();
	return OK;
}

status_t RTCSignalingNodeSession::onWSDisconnected() {
	ALOGV("onWSDisconnected");
	sp<AMessage> msg = new AMessage(kWhatWSDisconnected, this);
	msg->post();
	return OK;
}

status_t RTCSignalingNodeSession::onWSError() {
	sp<AMessage> msg = new AMessage(kWhatWSError, this);
	msg->post();
	return OK;
}

status_t RTCSignalingNodeSession::onWSMessage(std::string &ws_msg) {
	sp<AMessage> msg = new AMessage(kWhatWSMessage, this);
	msg->setString("msg", ws_msg.c_str());
	msg->post();
	return OK;
}

void RTCSignalingNodeSession::onMessageReceived(const sp<AMessage> &msg) {
	switch (msg->what()) {
	case kWhatOpen:
		{
			AString host;
			AString path;
			int port;

			CHECK(msg->findString("host", &host));
			CHECK(msg->findInt32("port", &port));
			msg->findString("path", &path);

			_open_m(host, port, path);
			break;
		}
	case kWhatClose:
		{
			_close_m();
			break;
		}
	case kWhatError:
		{
			break;
		}
	case kWhatWSConnected:
		{
			_state = RTC_SIGNAL_SESSION_CONNECTED;
			_regist_req_m();
			break;
		}
	case kWhatWSDisconnected:
		{
			_state = RTC_SIGNAL_SESSION_DISCONNECTED;
			if (_listener) {
				ALOGV("onMessageReceived OnSignalingNodeSessionClosed");
				_listener->OnSignalingNodeSessionClosed();
			}
			break;
		}
	case kWhatWSError:
		{
			_state = RTC_SIGNAL_SESSION_ERROR;
			if (_listener) {
				ALOGV("onMessageReceived kWhatWSError");
				_listener->OnSignalingNodeSessionError();
			}
			break;
		}
	case kWhatWSMessage:
		{
			AString astr;
			CHECK(msg->findString("msg", &astr));
			_received_msg_m((char*)astr.c_str());
			break;
		}
	case kWhatSendCommandRes:
		{
			AString cmd;
			AString id;
			AString result;
			CHECK(msg->findString("cmd", &cmd));
			CHECK(msg->findString("id", &id));
			CHECK(msg->findString("result", &result));

			_send_command_res(cmd.c_str(), id.c_str(), result.c_str());
			break;
		}
	case kWhatSendSDP:
		{
			AString id;
			AString sdp;
			CHECK(msg->findString("id", &id));
			CHECK(msg->findString("sdp", &sdp));

			_send_sdp(id.c_str(), sdp.c_str());
			break;
		}
	case kWhatSendCandidate:
		{
			AString id;
			int32_t index_line;
			AString mid;
			AString candidate;

			CHECK(msg->findString("id", &id));
			CHECK(msg->findInt32("idx", &index_line));
			CHECK(msg->findString("mid", &mid));
			CHECK(msg->findString("candidate", &candidate));

			_send_candidate(id.c_str(), index_line, mid.c_str(), candidate.c_str());

			break;
		}
	case kWhatSendIceState:
		{
			AString id;
			int32_t state;
			AString desc;

			CHECK(msg->findString("id", &id));
			CHECK(msg->findInt32("state", &state));
			CHECK(msg->findString("desc", &desc));

			_send_ice_state(id.c_str(), state, desc.c_str());
			break;
		}
	}
}

status_t RTCSignalingNodeSession::_init() {
	_looper->registerHandler(this);
	_state = RTC_SIGNAL_SESSION_INT;
	return OK;
}

void RTCSignalingNodeSession::_reset() {

}

status_t RTCSignalingNodeSession::_open_m(AString &host, int port, AString &path) {
	// TODO... check state

	_ws_client = new RTCWebSocketClient(this);
	if (_ws_client == nullptr) {
		sp<AMessage> msg = new AMessage(kWhatError, this);
		msg->post();
		return INVALID_OPERATION;
	}

	status_t ret = _ws_client->Open(host.c_str(), port, path.c_str());
	if (ret != OK) {
		_ws_client.clear();
		_ws_client = nullptr;

		sp<AMessage> msg = new AMessage(kWhatError, this);
		msg->post();
		return INVALID_OPERATION;
	}

	return OK;
}

void RTCSignalingNodeSession::_close_m() {
	if (_ws_client != nullptr) {
		_ws_client->Close();
	}
}

status_t RTCSignalingNodeSession::_regist_req_m() {
	json_t *jroot = json_object();

	json_object_set_new(jroot, kCommandAttr, json_string(kRegistMsg));
	std::string msg = json_dumps(jroot, 0);
	json_decref(jroot);

	_ws_client->Send(msg);

	return OK;
}

status_t RTCSignalingNodeSession::_received_msg_m(char *pmsg) {
	ALOGV("_received_msg_m %s", pmsg);
	json_error_t jerror;
	json_t *jroot = json_loads(pmsg, strlen(pmsg), &jerror);
	if (!jroot) {
		return INVALID_OPERATION;
	}

	status_t sret = OK;
	json_t *jcmd = json_object_get(jroot, kCommandAttr);
	std::string cmd = json_string_value(jcmd);

	if (cmd == kRegistMsgRes) {
		json_t *jresult = json_object_get(jroot, kResultAttr);
		std::string result = json_string_value(jresult);
		if (result != kSuccess) {
			sp<AMessage> msg = new AMessage(kWhatWSError, this);
			msg->post();
			sret = INVALID_OPERATION;
		}
		else {
			json_t *jnode_id = json_object_get(jroot, kIdAttr);
			_id = json_string_value(jnode_id);
			json_t *jiceinfo_array = json_object_get(jroot, kIceInfoAttr);
			if (jiceinfo_array && json_is_array(jiceinfo_array)) {
				size_t index;
				json_t* jice_entry;

				json_array_foreach(jiceinfo_array, index, jice_entry) {
					RTCIceServerInfo ice_info;
					json_t* jurl = json_object_get(jice_entry, kUrlAttr);
					ice_info._uri = json_string_value(jurl);

					json_t* jusername = json_object_get(jice_entry, kUserNameAttr);
					if (jusername) {
						ice_info._is_stun = false;
						ice_info._user_name = json_string_value(jusername);
					}

					json_t* jcredential = json_object_get(jice_entry, kCredentialAttr);
					if (jcredential) {
						ice_info._credential = json_string_value(jcredential);
					}
					_ice_infos.push_back(ice_info);
				}

			}
			_listener->OnSignalingNodeSessionOpened();
		}
	}
	else if (cmd == kAddPeerMsg) {
		json_t *jpeer_id = json_object_get(jroot, kIdAttr);
		std::string peer_id = json_string_value(jpeer_id);
		_listener->OnAddPeer(peer_id);
	} 
	else if (cmd == kRemovePeerMsg) {
		json_t *jpeer_id = json_object_get(jroot, kIdAttr);
		std::string peer_id = json_string_value(jpeer_id);
		_listener->OnRemovePeer(peer_id);
	} 
	else if (cmd == kOfferMsg) {
		json_t *jpeer_id = json_object_get(jroot, kIdAttr);
		std::string peer_id = json_string_value(jpeer_id);
		json_t *jsdp = json_object_get(jroot, kSdpAttr);
		std::string sdp = json_string_value(jsdp);
		_listener->OnPeerSDP(peer_id, sdp);
	}
	else if (cmd == kCandidateMsg){
		json_t *jpeer_id = json_object_get(jroot, kIdAttr);
		std::string peer_id = json_string_value(jpeer_id);
		json_t *jmline_idx = json_object_get(jroot, kMLineIndexAttr);
		int32_t mline_idx = json_integer_value(jmline_idx);
		json_t *jmid = json_object_get(jroot, kMIdAttr);
		std::string mid = json_string_value(jmid);
		json_t *jcandidate = json_object_get(jroot, kCandidateAttr);
		std::string candidate = json_string_value(jcandidate);

		_listener->OnPeerCandidate(peer_id, mline_idx, mid, candidate);
	}

	json_decref(jroot);

	return sret;
}

status_t RTCSignalingNodeSession::_send_command_res(const char* cmd, const char* id, const char* result) {
	json_t *jroot = json_object();
	json_object_set_new(jroot, kCommandAttr, json_string(cmd));
	json_object_set_new(jroot, kIdAttr, json_string(id));
	json_object_set_new(jroot, kResultAttr, json_string(result));

	std::string msg = json_dumps(jroot, 0);
	json_decref(jroot);
	_ws_client->Send(msg);
	return OK;
}

status_t RTCSignalingNodeSession::_send_sdp(const char *id, const char *sdp) {
	json_t *jroot = json_object();
	json_object_set_new(jroot, kCommandAttr, json_string(kAnswerMsg));
	json_object_set_new(jroot, kIdAttr, json_string(id));
	json_object_set_new(jroot, kSdpAttr, json_string(sdp));

	std::string msg = json_dumps(jroot, 0);
	json_decref(jroot);
	_ws_client->Send(msg);

	return OK;
}

status_t RTCSignalingNodeSession::_send_candidate(const char *id, int32_t mline_index, const char* mid, const char* candidate) {
	json_t *jroot = json_object();
	json_object_set_new(jroot, kCommandAttr, json_string(kCandidateMsg));
	json_object_set_new(jroot, kIdAttr, json_string(id));
	json_object_set_new(jroot, kMLineIndexAttr, json_integer(mline_index));
	json_object_set_new(jroot, kMIdAttr, json_string(mid));
	json_object_set_new(jroot, kCandidateAttr, json_string(candidate));

	std::string msg = json_dumps(jroot, 0);
	json_decref(jroot);
	_ws_client->Send(msg);

	return OK;
}

status_t RTCSignalingNodeSession::_send_ice_state(const char *id, int32_t state, const char* desc) {
	json_t *jroot = json_object();
	json_object_set_new(jroot, kCommandAttr, json_string(kIceStateMsg));
	json_object_set_new(jroot, kIdAttr, json_string(id));
	json_object_set_new(jroot, kIceStateAttr, json_integer(state));
	json_object_set_new(jroot, kIceStateDescAttr, json_string(desc));

	std::string msg = json_dumps(jroot, 0);
	json_decref(jroot);
	_ws_client->Send(msg);

	return OK;
}

}