//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_ICE_SERVER_INFO_H__
#define __RTC_ICE_SERVER_INFO_H__

#include <string>

namespace android {

struct RTCIceServerInfo  {
	bool			_is_stun = true;
	std::string		_uri;
	std::string		_user_name;
	std::string		_credential;
};

}


#endif //__RTC_ICE_SERVER_INFO_H__