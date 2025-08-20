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

#define LOG_TAG "RTCServerNodeController"

#include <utils/Log.h>

#include <thread>

#include "../rtc_server_node/RTCServerNodeListener.h"
#include "../rtc_server_node/RTCServerNode.h"
#include "../signaling_session/RTCWebSocketClient.h"

#include "RTCServerNodeController.h"

class RTCServerNodeController::Impl : public android::RTCServerNodeListener {
public:
	Impl(std::function<void(NodeSignalState, std::string)> stateCallback);
	~Impl() = default;

public:
	uint32_t Start(std::string &host, int port);
	void Stop();

private:
	void OnRTCServerNodeSignalState(RTCServerNodeListener::ServerNodeSignalState state) override;

private:
	static void _lws_log_emit_callback(int level, const char *line);

private:
	android::sp<android::RTCServerNode> _node;
	std::function<void(NodeSignalState, std::string)> _stateCallback;

private:
	Impl(const Impl &) = delete;
	Impl &operator=(const Impl &) = delete;
};

RTCServerNodeController::Impl::Impl(std::function<void(NodeSignalState, std::string)> stateCallback)
	: _stateCallback(stateCallback) {
	_node = android::RTCServerNode::Create(this);
	android::RTCWebSocketClient::SetLogLevel(
		LLL_ERR | LLL_WARN /*| LLL_NOTICE | LLL_INFO | LLL_DEBUG*/,
		RTCServerNodeController::Impl::_lws_log_emit_callback);
}

uint32_t RTCServerNodeController::Impl::Start(std::string &host, int port) {
	return _node->Start(host, port);
}

void RTCServerNodeController::Impl::Stop() {
	_node->Stop();
}

void RTCServerNodeController::Impl::OnRTCServerNodeSignalState(RTCServerNodeListener::ServerNodeSignalState state) {
	std::string state_desc = "";
	RTCServerNodeController::NodeSignalState node_state;
	switch (state) {
	case RTCServerNodeListener::S_NDOE_STATE_SIGNAL_INIT:
		{
			state_desc = "NodeState_Signal_Init";
			node_state = NodeState_Signal_Init;
			break;
		}
	case RTCServerNodeListener::S_NDOE_STATE_SIGNAL_OPENED:
		{
			state_desc = "NodeState_Signal_Opened";
			node_state = NodeState_Signal_Opened;
			break;
		}
	case RTCServerNodeListener::S_NODE_STATE_SIGNAL_CLOSED:
		{
			state_desc = "NodeState_Signal_Closed";
			node_state = NodeState_Signal_Closed;
			break;
		}
	case RTCServerNodeListener::S_NODE_STATE_SIGNAL_ERROR:
		{
			state_desc = "NodeState_Signal_Error";
			node_state = NodeState_Signal_Error;
			break;
		}
	}

	std::function<void()> task = [this, node_state, state_desc]() {

		_stateCallback(node_state, state_desc);
	};

	std::thread(task).detach();

}

void RTCServerNodeController::Impl::_lws_log_emit_callback(int level, const char *line) {
	ALOGV("level %d line %s", level, line);
}

//////////////////////////////////////////////////////////////////////////

std::unique_ptr<RTCServerNodeController> RTCServerNodeController::Create(std::function<void(NodeSignalState, std::string)> stateCallback) {
	std::unique_ptr<RTCServerNodeController> ptr(new RTCServerNodeController(stateCallback));
	return std::move(ptr);
}

RTCServerNodeController::~RTCServerNodeController() {

}

RTCServerNodeController::RTCServerNodeController(std::function<void(NodeSignalState, std::string)> stateCallback) {
	impl_ = std::make_unique<RTCServerNodeController::Impl>(stateCallback);
}

uint32_t RTCServerNodeController::Start(std::string &host, int port) {
	if (impl_ == nullptr) {
		return -1;
	}
	return impl_->Start(host, port);
}

uint32_t RTCServerNodeController::Stop() {
	if (impl_ == nullptr) {
		return 0;
	}

	impl_->Stop();
	return 0;
}