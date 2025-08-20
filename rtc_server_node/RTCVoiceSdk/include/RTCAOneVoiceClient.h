//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_AONE_VOICE_CLIENT_H__
#define __RTC_AONE_VOICE_CLIENT_H__

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "RTCAOneVoiceClientListener.h"

namespace aone {
namespace voice {

using PCMCallback = std::function<void(const int16_t* data, size_t samples, int sample_rate, int channels)>;

class RTCAOneVoiceClient {
public:
	struct RTCIceServerInfo {
		bool			_is_stun = true;
		std::string		_uri;
		std::string		_user_name;
		std::string		_credential;
	};

public:
	static std::unique_ptr<RTCAOneVoiceClient> Create(std::vector<RTCIceServerInfo>& rtcIceInfos, RTCAOneVoiceClientListener* listener);

public:
	int32_t Open();
	int32_t Close();

	int32_t SetRemoteDescription(const std::string& sdp);
	int32_t SetRemoteIceCandidate(int32_t mline_idx,
				const std::string& mid, const std::string& candidate);

	void SetPCMCallback(PCMCallback cb);
	
public:
	~RTCAOneVoiceClient();

private:
	RTCAOneVoiceClient(std::vector<RTCIceServerInfo>& rtcIceInfos, RTCAOneVoiceClientListener* listener);

private:
	class Impl;
	std::unique_ptr<Impl> impl_;

private:
	RTCAOneVoiceClient(const RTCAOneVoiceClient&) = delete;
	RTCAOneVoiceClient& operator=(const RTCAOneVoiceClient&) = delete;

};

}	// voice
}	// aone

#endif // __RTC_AONE_VOICE_CLIENT_H__