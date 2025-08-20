//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_AONE_SDK_H__
#define __RTC_AONE_SDK_H__

namespace aone {
namespace voice {

class RTCAOneSdk {
public:
	RTCAOneSdk() = default;
	~RTCAOneSdk() = default;

public:
	static void initialize();
	static void deinitialize();
};

}
}

#endif // __RTC_AONE_SDK_H__