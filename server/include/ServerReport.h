//
// Created by Khubaib.Umer on 8/26/2022.
//

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <atomic>
#include "../../Types.h"
#include "Lock.h"

struct ThreadStats
{
	ThreadStats() : max_(0), min_(INT32_MAX), sum_(0), count_(0)
	{
	}

	void operator+=(long curr)
	{
		max_ = ((curr > max_) ? curr : max_);
		min_ = ((curr < min_) ? curr : min_);
		sum_ += curr;
		++count_;
	}

	[[nodiscard]] long getMax() const
	{
		return max_;
	}

	[[nodiscard]] long getMin() const
	{
		return min_;
	}

	[[nodiscard]] unsigned long getAvg() const
	{
		return (sum_ / count_);
	}

	long max_{};
	long min_;
	uint64_t sum_{};
	uint64_t count_{};
};

using ThreadStatsPtr = std::unique_ptr<ThreadStats>;

class ServerReport
{
public:
	static ServerReport* getInstance();

	[[nodiscard]] std::string getAvgResponseTime();

	[[nodiscard]] std::string getMaxResponseTime();

	[[nodiscard]] std::string getMinResponseTime();

	[[nodiscard]] std::string getSummary();

	void reportTxnTime(long timeTaken);

private:
	ServerReport();

private:
	std::unordered_map<tid_t, ThreadStatsPtr> perThreadStats_;
	std::unique_ptr<SpinLock> statsLock_;
};

#define reporter ServerReport::getInstance()
