//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_WEBSOCKET_CLIENT_H__
#define __RTC_WEBSOCKET_CLIENT_H__

#include <string>

#include <utils/RefBase.h>
#include <utils/Thread.h>
#include <utils/Mutex.h>

#include <libwebsockets.h>

namespace android {

class RTCWebSocketClientListener;
class RTCWebSocketClient : public RefBase {
public:
	RTCWebSocketClient() = delete;
	RTCWebSocketClient(RTCWebSocketClientListener *listener);
	virtual ~RTCWebSocketClient();

public:
	status_t Open(const std::string& host, int port, const std::string& path);
	status_t Close();
	status_t Send(std::string &msg);
	bool IsOpened() const;

public:
	typedef void(*LWS_LOG_EMIT_CALLBACK)(int32_t level, const char *line);

public:
	static void SetLogLevel(int32_t level, LWS_LOG_EMIT_CALLBACK callback);

private:
	class RTCWebSocketClientThread : public virtual Thread {
	public:
		RTCWebSocketClientThread(RTCWebSocketClient &rClient);
		virtual ~RTCWebSocketClientThread();
		status_t requestExitAndWait();

	private:
		friend class RTCWebSocketClient;
		bool threadLoop();

	private:
		RTCWebSocketClient &_rClient;
	};

private:
	enum ws_client_state {
		WS_CLIENT_STATE_INIT,
		WS_CLIENT_STATE_CONNECTING,
		WS_CLIENT_STATE_OPEN,
		WS_CLIENT_STATE_CLOSING,
		WS_CLIENT_STATE_ERROR,
	};

private:
	const uint16_t _WS_RING_BUFFER_SIZE = 1024;

	typedef struct {
		void *_payload;
		size_t _len;
		uint8_t  _is_binary;
		uint8_t  _is_first;
		uint8_t  _is_final;
	}ws_msg;

	typedef struct {
		struct lws_ring *_ring;
		uint32_t	_tail;
		uint8_t		_completed : 1;
		uint8_t		_write_consume_pending : 1;
	} ws_ring_info;

private:
	static int32_t _rtc_websocket_lws_callback(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user_data,
		void *in, size_t len);

	bool _websocket_service_loop();
	void _reset();

private:
	status_t _create_ws_ring_buffer();
	status_t _insert_ring_buffer(std::string &data);
	static void _ws_destroy_message(void *msg);

	void _reset_ring();
	status_t _start_service_thread();

private:

	std::string _recv_msg;
	ws_ring_info	_ws_ring_info;

	RTCWebSocketClientListener *_listener;

	sp<RTCWebSocketClientThread> _websocket_thread;

	struct lws_context *_wsc;
	struct lws *_wsi;
	struct lws_protocols _rtc_websocket_protocols[2];

	ws_client_state _state = WS_CLIENT_STATE_INIT;
	std::string _host;
	std::string _path;
	Mutex	_lock;

private:
	DISALLOW_EVIL_CONSTRUCTORS(RTCWebSocketClient);
};

}



#endif //__RTC_WEBSOCKET_CLIENT_H__