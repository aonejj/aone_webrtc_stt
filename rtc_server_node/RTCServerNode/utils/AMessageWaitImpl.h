//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __AMESSAGE_WAIT_IMPL_H__
#define __AMESSAGE_WAIT_IMPL_H__

#include <utils/RefBase.h>
#include <utils/Errors.h>

#include <media/stagefright/foundation/AMessage.h>

namespace android {

class AMessageWaitImpl {
public:
	AMessageWaitImpl() = delete;
	~AMessageWaitImpl() = delete;

public:
	static const char* const kErrorName;

public:
	static status_t PostAndAWaitResponse(
		const sp<AMessage> &msg, sp<AMessage> *response);
};

}


#endif // __AMESSAGE_WAIT_IMPL_H__
