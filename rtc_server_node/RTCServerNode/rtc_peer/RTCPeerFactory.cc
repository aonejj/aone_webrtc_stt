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

#define LOG_TAG "RTCPeerFactory"
#include <utils/Log.h>

#include <media/stagefright/foundation/ADebug.h>

#include "RTCPeerFactory.h"

#include "RTCPeer.h"

namespace android {

sp<RTCPeerInterface> RTCPeerFactory::CreateRTCPeer(std::string &id, 
												   std::vector<RTCIceServerInfo>& rtcIceInfos,
												   RTCPeerListener* listener) {
	return RTCPeer::Create(id, rtcIceInfos, listener);
}

}