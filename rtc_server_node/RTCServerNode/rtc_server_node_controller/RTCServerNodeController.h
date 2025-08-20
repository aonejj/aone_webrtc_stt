//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_SERVER_NODE_CONTROLLER_H__
#define __RTC_SERVER_NODE_CONTROLLER_H__

#include <memory>
#include <string>
#include <functional>

class  RTCServerNodeController {
public:
	enum NodeSignalState{
		NodeState_Signal_Init,
		NodeState_Signal_Opened,
		NodeState_Signal_Closed,
		NodeState_Signal_Error,
	};

public:
	static std::unique_ptr<RTCServerNodeController> Create(std::function<void(NodeSignalState, std::string)> stateCallback);
	~RTCServerNodeController();

public:
	uint32_t Start(std::string &host, int port);
	uint32_t Stop();

private:
	RTCServerNodeController(std::function<void(NodeSignalState, std::string)> stateCallback);

private:
	class Impl;
	std::unique_ptr<Impl> impl_;

private:
	RTCServerNodeController(const RTCServerNodeController &) = delete;
	RTCServerNodeController &operator=(const RTCServerNodeController &) = delete;
};

#endif // __RTC_SERVER_NODE_CONTROLLER_H__