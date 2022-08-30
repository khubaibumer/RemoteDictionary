//
// Created by Khubaib.Umer on 8/26/2022.
//

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <atomic>
#include <climits>
#include "../../Types.h"
#include "Lock.h"

struct Stats
{
public:
	Stats() : max_(0), min_(LONG_MAX), sum_(0), count_(0)
	{
	}

	void operator+=(size_t curr)
	{
		max_ = ((curr > max_) ? curr : max_);
		min_ = ((curr < min_) ? curr : min_);
		sum_ += curr;
		++count_;
	}

	[[nodiscard]] size_t getMax() const
	{
		return max_;
	}

	[[nodiscard]] size_t getMin() const
	{
		return min_;
	}

	[[nodiscard]] size_t getAvg() const
	{
		return ((sum_ == 0 || count_ == 0) ? 0 : (sum_ / count_));
	}

	[[nodiscard]] size_t getSum() const
	{
		return sum_;
	}

private:
	size_t max_{};
	size_t min_;
	uint64_t sum_{};
	uint64_t count_{};
};

using ThreadStatsPtr = std::unique_ptr<Stats>;

class ServerReport
{
public:
	static ServerReport* getInstance();

	[[nodiscard]] std::string getAvgResponseTime();

	[[nodiscard]] std::string getMaxResponseTime();

	[[nodiscard]] std::string getMinResponseTime();

	[[nodiscard]] std::string getSummary();

private:
	ServerReport();

private:
//	std::unordered_map<tid_t, ThreadStatsPtr> perThreadStats_;
//	std::unique_ptr<SpinLock> statsLock_;
};

#define reporter ServerReport::getInstance()
