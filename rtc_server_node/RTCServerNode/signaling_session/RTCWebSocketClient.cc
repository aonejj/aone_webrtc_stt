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

#define LOG_TAG "RTCWebSocketClient"

#include <string>
#include <utils/Log.h>

#include <utils/Errors.h>
#include <media/stagefright/foundation/ADebug.h>

#include "RTCWebSocketClientListener.h"
#include "RTCWebSocketClient.h"


namespace android {

//////////////////////////////////////////////////////////////////////////
// RTCWebsocketServerThread
	RTCWebSocketClient::RTCWebSocketClientThread::RTCWebSocketClientThread(
		RTCWebSocketClient &rClient)
	: Thread(false),
	_rClient(rClient) {

}

RTCWebSocketClient::RTCWebSocketClientThread::~RTCWebSocketClientThread() {

}

status_t RTCWebSocketClient::RTCWebSocketClientThread::requestExitAndWait() {
	return Thread::requestExitAndWait();
}

bool RTCWebSocketClient::RTCWebSocketClientThread::threadLoop() {
	return _rClient._websocket_service_loop();
}

RTCWebSocketClient::RTCWebSocketClient(RTCWebSocketClientListener *listener) 
 : _listener(listener),
   _wsc(nullptr),
   _wsi(nullptr) {

	CHECK(_listener);
	memset(&_ws_ring_info, 0x00, sizeof(ws_ring_info));

	_rtc_websocket_protocols[0] = {
		"rtc_node_protocol",
		RTCWebSocketClient::_rtc_websocket_lws_callback,
		0, 0, 0, nullptr, 0
	};

	_rtc_websocket_protocols[1] = LWS_PROTOCOL_LIST_TERM;

	_create_ws_ring_buffer();
}

RTCWebSocketClient::~RTCWebSocketClient() {
	_reset_ring();
	_reset();
}

status_t RTCWebSocketClient::Open(const std::string& host, int port, const std::string& path) {
	struct lws_context_creation_info info;
	memset(&info, 0x00, sizeof(info));
	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = _rtc_websocket_protocols;
	info.gid = -1;
	info.uid = -1;
	info.user = this;

	Mutex::Autolock lock(_lock);

	ALOGV("lws_create_context+++");
	_wsc = lws_create_context(&info);
	if (!_wsc) {
		return INVALID_OPERATION;
	}
	ALOGV("lws_create_context---");

	_host = host;
	_path = path;

	struct lws_client_connect_info ccinfo;
	memset(&ccinfo, 0x00, sizeof(ccinfo));

	ccinfo.context = _wsc;
	ccinfo.address = _host.c_str();
	ccinfo.port = port;
	ccinfo.path = _path.c_str();
	ccinfo.host = _host.c_str();
	ccinfo.origin = "origin";
	ccinfo.protocol = "rtc_node_protocol";

	_wsi = lws_client_connect_via_info(&ccinfo);
	if (!_wsi) {
		lws_context_destroy(_wsc);
		_wsc = nullptr;
		_state = WS_CLIENT_STATE_ERROR;
		ALOGE("lws_client_connect_via_info fail");
		return INVALID_OPERATION;
	}
	ALOGV("lws_client_connect_via_info---");

	status_t ret = _start_service_thread();
	if (ret != OK) {
		_reset();
	}

	_state = WS_CLIENT_STATE_CONNECTING;
	return OK;
}

status_t RTCWebSocketClient::Close() {
	Mutex::Autolock lock(_lock);
	if (!IsOpened()) {
		return OK;
	}

	if (_wsc) {
		if (_wsi) {
			_state = WS_CLIENT_STATE_CLOSING;
			lws_close_reason(_wsi, LWS_CLOSE_STATUS_NORMAL, (unsigned char*)"Client closing", strlen("Client closing"));
		}
		lws_cancel_service(_wsc);
	}

	_reset();

	return OK;
}

status_t RTCWebSocketClient::Send(std::string &msg) {
	status_t ret;
	Mutex::Autolock lock(_lock);
	ret = _insert_ring_buffer(msg);
	if (ret == OK) {
		lws_callback_on_writable(_wsi);
	}

	return ret;
}

bool RTCWebSocketClient::IsOpened() const {
	if (_state == WS_CLIENT_STATE_OPEN) {
		return true;
	}
	return false;
}


void RTCWebSocketClient::SetLogLevel(int32_t level, LWS_LOG_EMIT_CALLBACK callback) {
	lws_set_log_level(level, callback);
}

status_t RTCWebSocketClient::_create_ws_ring_buffer() {
	_ws_ring_info._ring = lws_ring_create(sizeof(ws_msg),
		_WS_RING_BUFFER_SIZE, RTCWebSocketClient::_ws_destroy_message);
	if (!_ws_ring_info._ring) {
		return INVALID_OPERATION;
	}

	return OK;
}

status_t RTCWebSocketClient::_insert_ring_buffer(std::string &data) {
	ALOGV("_insert_ring_buffer _ring %p", _ws_ring_info._ring);
	size_t size = lws_ring_get_count_free_elements(_ws_ring_info._ring);
	if (size == 0) {
		return INVALID_OPERATION;
	}


	ws_msg msg;
	msg._is_first = 0x01;
	msg._is_final = 0x01;
	msg._is_binary = 0x00;
	msg._len = data.size();
	msg._payload = new uint8_t[LWS_PRE + data.size()];

	if (!msg._payload) {
		return INVALID_OPERATION;
	}

	memcpy((uint8_t*)msg._payload + LWS_PRE, data.data(), data.size());
	if (!lws_ring_insert(_ws_ring_info._ring, &msg, 1)) {
		_ws_destroy_message(&msg);
		return INVALID_OPERATION;
	}
	return OK;
}

void RTCWebSocketClient::_reset_ring() {
	if (!_ws_ring_info._ring) {
		return;
	}

	size_t pendings = lws_ring_get_count_waiting_elements(
		_ws_ring_info._ring, &_ws_ring_info._tail);

	if (pendings > 0) {
		lws_ring_consume(_ws_ring_info._ring, &_ws_ring_info._tail, NULL, pendings);
	}

	lws_ring_destroy(_ws_ring_info._ring);
	_ws_ring_info._ring = NULL;
}

void RTCWebSocketClient::_ws_destroy_message(void *msg) {
	ws_msg *wmsg = (ws_msg*)msg;
	delete (uint8_t*)wmsg->_payload;
	wmsg->_payload = NULL;
	wmsg->_len = 0;
}

status_t RTCWebSocketClient::_start_service_thread() {
	_websocket_thread = new RTCWebSocketClientThread(*this);
	if (_websocket_thread == nullptr) {
		return INVALID_OPERATION;
	}

	ALOGV("_websocket_thread->run+++");
	status_t ret = _websocket_thread->run("RTCWebSocketClientThread");
	if (ret != OK) {
		ALOGE("_websocket_thread->run fail");
		_websocket_thread.clear();
		_websocket_thread = nullptr;
	}

	return ret;
}

int32_t RTCWebSocketClient::_rtc_websocket_lws_callback(struct lws *wsi,
														enum lws_callback_reasons reason,
														void *user_data,
														void *in, size_t len) {
	struct lws_context *ctx = lws_get_context(wsi);
	RTCWebSocketClient *client = (RTCWebSocketClient*)lws_context_user(ctx);

	if (!client->_listener) {
		return 0;
	}

	switch (reason) {
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		{
			ALOGV("_rtc_websocket_lws_callback LWS_CALLBACK_CLIENT_ESTABLISHED");
			client->_state = WS_CLIENT_STATE_OPEN;
			client->_listener->onWSConnected();
			break;
		}
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		{
			ALOGV("_rtc_websocket_lws_callback LWS_CALLBACK_CLIENT_CONNECTION_ERROR");
			client->_state = WS_CLIENT_STATE_ERROR;
			client->_listener->onWSError();
			break;
		}
	case LWS_CALLBACK_CLIENT_RECEIVE:
		{
			ALOGV("_rtc_websocket_lws_callback LWS_CALLBACK_CLIENT_RECEIVE");
			if (!in || len == 0) {
				break;
			}

			client->_recv_msg += std::string((char*)in, len);
			const size_t remaining = lws_remaining_packet_payload(wsi);
			if (remaining > 0 || !lws_is_final_fragment(wsi)) {
				// waiting for more fragments message
				ALOGV("[%p] LWS_CALLBACK_CLIENT_RECEIVE waiting for more fragment message", client);
				break;
			}

			client->_listener->onWSMessage(client->_recv_msg);
			client->_recv_msg = "";
			break;
		}
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		{
			ALOGV("_rtc_websocket_lws_callback LWS_CALLBACK_CLIENT_WRITEABLE");
			if (client->_ws_ring_info._write_consume_pending) {
				lws_ring_consume_single_tail(
					client->_ws_ring_info._ring, &(client->_ws_ring_info._tail), 1);
				client->_ws_ring_info._write_consume_pending = 0;
			}

			ws_msg *pmsg = (ws_msg*)lws_ring_get_element(
				client->_ws_ring_info._ring, &(client->_ws_ring_info._tail));
			if (!pmsg) {
				// noting..
				return OK;
			}

			int32_t flags = lws_write_ws_flags(
				pmsg->_is_binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT,
				pmsg->_is_first, pmsg->_is_final);

			int32_t len = lws_write(
				wsi, ((unsigned char*)pmsg->_payload) + LWS_PRE,
				pmsg->_len, (enum lws_write_protocol)flags);

			if (len < pmsg->_len) {
				break;
			}

			client->_ws_ring_info._write_consume_pending = 1;
			lws_callback_on_writable(wsi);
			break;
		}
	case LWS_CALLBACK_CLIENT_CLOSED:
		{
			ALOGV("_rtc_websocket_lws_callback LWS_CALLBACK_CLIENT_CLOSED");
			client->_state = WS_CLIENT_STATE_INIT;
			client->_listener->onWSDisconnected();
			break;
		}
	default:
		{
			break;
		}
	}

	return 0;
}

bool RTCWebSocketClient::_websocket_service_loop() {
	if (_state == WS_CLIENT_STATE_ERROR) {
		return false;
	}

	lws_service(_wsc, 5);

	return true;
}

void RTCWebSocketClient::_reset() {
	if (_websocket_thread != nullptr) {
		_websocket_thread->requestExitAndWait();
		_websocket_thread.clear();
		_websocket_thread = nullptr;
	}

	if (_wsc) {
		lws_context_destroy(_wsc);
		_wsc = nullptr;
		_wsi = nullptr;
	}
}

}