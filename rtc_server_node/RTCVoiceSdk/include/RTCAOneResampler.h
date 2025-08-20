//////////////////////////////////////////////////////////////////////////
//
// author : kimi
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RTC_AONE_RESAMPLER_H__
#define __RTC_AONE_RESAMPLER_H__

#include <memory>

namespace aone {
namespace voice {

class RTCAOneResampler {
public:
	RTCAOneResampler();
	~RTCAOneResampler();

public:
	static void DownmixInterleavedToMono(const int16_t* interleaved, size_t num_frames,
																int num_channels, int16_t* deinterleaved);

public:
	int32_t SetConfig(int32_t samplingRate,  int32_t resampleRate, int32_t channels);
	size_t Resample(const int16_t* source, size_t source_lenght, 
						     int16_t* destination, size_t destination_capacity);

private:	
	class Impl;
	std::unique_ptr<Impl> impl_;

private:
	RTCAOneResampler(const RTCAOneResampler&) = delete;
	RTCAOneResampler& operator=(const RTCAOneResampler&) = delete;
};

}
}

#endif // __RTC_AONE_RESAMPLER_H__