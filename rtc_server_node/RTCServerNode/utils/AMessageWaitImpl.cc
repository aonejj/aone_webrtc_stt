//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#include "AMessageWaitImpl.h"

namespace android {

const char* const AMessageWaitImpl::kErrorName = "err";

status_t AMessageWaitImpl::PostAndAWaitResponse(
	const sp<AMessage> &msg, sp<AMessage> *response) {
	status_t err = msg->postAndAwaitResponse(response);

	if (err != OK) {
		return err;
	}

	if (!(*response)->findInt32(AMessageWaitImpl::kErrorName, &err)) {
		err = OK;
	}

	return err;
}

}