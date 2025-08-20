//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_VAD_ZMQ_CLIENT_H__
#define __RTC_VAD_ZMQ_CLIENT_H__

#include <deque>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <utils/threads.h>


#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>

#include <media/stagefright/foundation/ABuffer.h>

#include <zmq.h>

namespace android {

class RTCVADZmqClient : public AHandler {
public:
	virtual ~RTCVADZmqClient();

public:
	static sp<RTCVADZmqClient> Create();

public:
	status_t Start();
	status_t Stop();

	void SendVADPCM(std::string &id, sp<ABuffer> active_buffer);

private:
	RTCVADZmqClient();

private:
	enum {
		kWhatStart,
		kWhatStop,
		kWhatSendVadPcm,
	};

	typedef enum {
		RTC_VAD_ZMQ_CLIENT_UNINIT,
		RTC_VAD_ZMQ_CLIENT_INIT,
		RTC_VAD_ZMQ_CLIENT_START,
		RTC_VAD_ZMQ_CLIENT_ERROR,
	} RTCVADZmqClientState;

private:
	class ProcessorThread : public virtual Thread {
	public:
		ProcessorThread();
		virtual ~ProcessorThread();

		status_t requestExitAndWait();

		void	stop();
		void	pushVADPcmBuffer(sp<ABuffer> activity_buffer);


	private:
		virtual status_t readyToRun();
		virtual bool threadLoop();

	private:
		std::deque<sp<ABuffer>> _active_voices;
		Mutex		_lock;
		Condition	_cond;
		bool		_is_stoping = false;

		void* _ctx = nullptr;     // ZeroMQ context
		void* _socket = nullptr;  // ZeroMQ socket

	private:
		DISALLOW_EVIL_CONSTRUCTORS(ProcessorThread);
	};

private:
	// AHandler methods
	void onMessageReceived(const sp<AMessage> &msg) override;

private:
	status_t _init();
	status_t _start_m();
	status_t _stop_m();

private:
	sp<ALooper> _looper;
	sp<ProcessorThread> _thread;
	RTCVADZmqClientState _state = RTC_VAD_ZMQ_CLIENT_UNINIT;
private:
	DISALLOW_EVIL_CONSTRUCTORS(RTCVADZmqClient);
};



}


#endif // __RTC_VAD_ZMQ_CLIENT_H__