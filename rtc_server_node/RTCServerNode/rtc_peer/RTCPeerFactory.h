//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_PEER_FACTORY_H__
#define __RTC_PEER_FACTORY_H__

#include <string>
#include <vector>
#include <utils/RefBase.h>
#include <media/stagefright/foundation/ABase.h>

#include "RTCPeerListener.h"
#include "RTCPeerInterface.h"
#include "../rtc_server_node/RTCIceServerInfo.h"

namespace android {

class RTCPeerFactory {
public:
	static sp<RTCPeerInterface> CreateRTCPeer(std::string &id, std::vector<RTCIceServerInfo>& rtcIceInfos,
											RTCPeerListener* listener);

private:
	RTCPeerFactory() = default;
	~RTCPeerFactory() = default;

private:
	DISALLOW_EVIL_CONSTRUCTORS(RTCPeerFactory);
};

}

#endif // __RTC_PEER_FACTORY_H__
