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

#define LOG_TAG "RTCVADZmqClient"
#include <utils/Log.h>
#include <media/stagefright/foundation/ADebug.h>

#include <string>

#include "RTCVADZmqClient.h"
#include "../utils/AMessageWaitImpl.h"

namespace android {

RTCVADZmqClient::ProcessorThread::ProcessorThread() {

}

RTCVADZmqClient::ProcessorThread::~ProcessorThread() {
	if (_socket) {
		zmq_close(_socket);
	}

	if (_ctx) {
		zmq_ctx_destroy(_ctx);
	}
}

status_t RTCVADZmqClient::ProcessorThread::requestExitAndWait() {
	return Thread::requestExitAndWait();
}

void RTCVADZmqClient::ProcessorThread::stop() {
	_is_stoping = true;
	Mutex::Autolock lock(_lock);
	_cond.signal();
}

void RTCVADZmqClient::ProcessorThread::pushVADPcmBuffer(sp<ABuffer> activity_buffer) {
	Mutex::Autolock lock(_lock);
	_active_voices.push_back(activity_buffer);
	_cond.signal();
}


status_t RTCVADZmqClient::ProcessorThread::readyToRun() {
	_ctx = zmq_ctx_new();
	if (!_ctx) {
		ALOGE("Failed to create ZMQ context");
		return INVALID_OPERATION;
	}

	_socket = zmq_socket(_ctx, ZMQ_DEALER);
	if (!_socket) {
		ALOGE("Failed to create ZMQ socket");
		zmq_ctx_destroy(_ctx);
		_ctx = nullptr;
		return INVALID_OPERATION;
	}

	int linger = 0;
	zmq_setsockopt(_socket, ZMQ_LINGER, &linger, sizeof(linger));

	int rc = zmq_connect(_socket, "ipc:///tmp/stt_server.ipc");
	if (rc != 0) {
		ALOGE("Failed to connect to ipc:///tmp/stt_server.ipc : %s", zmq_strerror(zmq_errno()));
		zmq_close(_socket);
		zmq_ctx_destroy(_ctx);
		_socket = nullptr;
		_ctx = nullptr;
		return INVALID_OPERATION;
	}

	return OK;
}

bool RTCVADZmqClient::ProcessorThread::threadLoop() {
	Mutex::Autolock lock(_lock);
	if (_active_voices.empty()) {
		_cond.wait(_lock);
	}

	if (_is_stoping) {
		return true;
	}

	sp<ABuffer> voice = _active_voices.front();
	_active_voices.pop_front();

	AString id;
	AString senum;
	voice->meta()->findString("id", &id);
	voice->meta()->findString("enum", &senum);
	
	if (_socket != nullptr) {
//		ALOGI("ProcessorThread::threadLoop send+++");
		zmq_msg_t msg;
		zmq_msg_init_size(&msg, id.size());
		memcpy(zmq_msg_data(&msg), id.c_str(), id.size());
		zmq_msg_send(&msg, _socket, ZMQ_SNDMORE);
		zmq_msg_close(&msg);

		zmq_msg_init_size(&msg, senum.size());
		memcpy(zmq_msg_data(&msg), senum.c_str(), senum.size());
		if (senum != "end") {
			zmq_msg_send(&msg, _socket, ZMQ_SNDMORE);
		}
		else {
			zmq_msg_send(&msg, _socket, 0);
		}
		zmq_msg_close(&msg);

		if (senum != "end") {
			zmq_msg_init_size(&msg, voice->size());
			memcpy(zmq_msg_data(&msg), voice->data(), voice->size());
			zmq_msg_send(&msg, _socket, 0);
			zmq_msg_close(&msg);
		}
//		ALOGI("ProcessorThread::threadLoop send---");
	}

	return true;
}


sp<RTCVADZmqClient> RTCVADZmqClient::Create() {
	sp<RTCVADZmqClient> client = new RTCVADZmqClient;
	if (client != nullptr) {
		if (client->_init() != OK) {
			client.clear();
			return nullptr;
		}
	}

	return client;
}

RTCVADZmqClient::RTCVADZmqClient()
 : _thread(nullptr) {
	_looper = new ALooper;
	_looper->setName("RTCVADZmqClient");
	_looper->start();
}

RTCVADZmqClient::~RTCVADZmqClient() {
	if (_looper != nullptr) {
		_looper->unregisterHandler(id());
		_looper->stop();
	}
}

status_t RTCVADZmqClient::Start() {
	sp<AMessage> msg = new AMessage(kWhatStart, this);
	sp<AMessage> response;

	return AMessageWaitImpl::PostAndAWaitResponse(msg, &response);
}

status_t RTCVADZmqClient::Stop() {
	sp<AMessage> msg = new AMessage(kWhatStop, this);
	sp<AMessage> response;

	return AMessageWaitImpl::PostAndAWaitResponse(msg, &response);
}

void RTCVADZmqClient::SendVADPCM(std::string &id, sp<ABuffer> active_buffer) {
	active_buffer->meta()->setString("id", id.c_str());

	sp<AMessage> msg = new AMessage(kWhatSendVadPcm, this);
	msg->setBuffer("voice", active_buffer);
	msg->post();
}

void RTCVADZmqClient::onMessageReceived(const sp<AMessage> &msg) {
	switch (msg->what()) {
	case kWhatStart:
		{
			sp<AReplyToken> replyID;
			CHECK(msg->senderAwaitsResponse(&replyID));
			sp<AMessage> response = new AMessage;
			if (_state == RTC_VAD_ZMQ_CLIENT_START) {
				response->setInt32(AMessageWaitImpl::kErrorName,
					OK);
				response->postReply(replyID);
				break;
			}

			status_t ret = _start_m();
			if (ret != OK) {
				_state = RTC_VAD_ZMQ_CLIENT_ERROR;
			}
			else {
				_state = RTC_VAD_ZMQ_CLIENT_START;
			}

			response->setInt32(AMessageWaitImpl::kErrorName, ret);
			response->postReply(replyID);

			break; 
		}
	case kWhatStop:
		{
			sp<AReplyToken> replyID;
			CHECK(msg->senderAwaitsResponse(&replyID));
			sp<AMessage> response = new AMessage;
			if (_state != RTC_VAD_ZMQ_CLIENT_START) {
				response->setInt32(AMessageWaitImpl::kErrorName,
					INVALID_OPERATION);
				response->postReply(replyID);
				break;
			}

			status_t ret = _stop_m();
			if (ret != OK) {
				_state = RTC_VAD_ZMQ_CLIENT_ERROR;
			}
			else {
				_state = RTC_VAD_ZMQ_CLIENT_INIT;
			}

			response->setInt32(AMessageWaitImpl::kErrorName, ret);
			response->postReply(replyID);

			break;
		}
	case kWhatSendVadPcm:
		{
			if (_state == RTC_VAD_ZMQ_CLIENT_START) {
				sp<ABuffer> voice;
				CHECK(msg->findBuffer("voice", &voice));

				if (_thread != nullptr) {
					_thread->pushVADPcmBuffer(voice);
				}
			}
			break;
		}
	}
}

status_t RTCVADZmqClient::_init() {
	_looper->registerHandler(this);
	_state = RTC_VAD_ZMQ_CLIENT_UNINIT;
	return OK;
}

status_t RTCVADZmqClient::_start_m() {
	_thread = new ProcessorThread;
	if (_thread->run("RTCVADZmqClientThread") != OK) {
		_thread.clear();
		_thread = nullptr;
		return INVALID_OPERATION;
	}

	return OK;
}

status_t RTCVADZmqClient::_stop_m() {
	if (_thread != nullptr) {
		_thread->stop();
		_thread->requestExitAndWait();
		_thread.clear();
		_thread = nullptr;
	}
	return OK;
}

}