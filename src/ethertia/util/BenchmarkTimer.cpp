
#include "BenchmarkTimer.h"

#include <chrono>
#include <iostream>
#include <format>

static int64_t Nanoseconds() {  // 1ns = 1,000,000,000s * 0.000000001
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}


BenchmarkTimer::BenchmarkTimer(float* _ptm, const char* _msg) :
	m_TimeBegin(Nanoseconds()),
	m_pTimeAppend(_ptm),
	m_EndMsg(_msg)
{
}

BenchmarkTimer::~BenchmarkTimer()
{
	if (!m_Done)
	{
		Stop();
	}
}

void BenchmarkTimer::Stop()
{
	double durationMS = (Nanoseconds() - m_TimeBegin) * 0.000001;

	if (m_pTimeAppend)
	{
		*m_pTimeAppend += durationMS * 0.001;
	}

	if (m_EndMsg)
	{
		std::string time_str;

		if (durationMS > 1000)
		{
			time_str = std::to_string(durationMS * 0.001) + "s";
		}
		else
		{
			time_str = std::to_string(durationMS) + "ms";
		}

		std::cout << std::vformat(m_EndMsg, std::make_format_args(time_str));
		std::cout.flush();  // for what?
	}

	m_Done = true;
}
