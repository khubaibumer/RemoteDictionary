//
// Created by Khubaib.Umer on 8/26/2022.
//

#include <sstream>
#include <nlohmann/json.hpp>
#include <climits>
#include "../include/ServerReport.h"
#include "../include/Current.h"

ServerReport* ServerReport::getInstance()
{
	static ServerReport instance;
	return &instance;
}

std::string ServerReport::getAvgResponseTime()
{
	std::ostringstream os;
	{
		TAKE_LOCK(statsLock_);
		for (const auto& itr : perThreadStats_)
		{
			os << "Thread ID: " << itr.first << " AvgTime: " << itr.second->getAvg() << ",";
		}
	}
	auto str = os.str();
	return str.substr(0, str.size() - 1);
}

[[maybe_unused]] std::string ServerReport::getMaxResponseTime()
{
	std::ostringstream os;
	{
		TAKE_LOCK(statsLock_);
		for (const auto& itr : perThreadStats_)
		{
			os << "Thread ID: " << itr.first << " MaxTime: " << itr.second->getMax() << ",";
		}
	}
	auto str = os.str();
	return str.substr(0, str.size() - 1);
}

std::string ServerReport::getMinResponseTime()
{
	std::ostringstream os;
	{
		TAKE_LOCK(statsLock_);
		for (const auto& itr : perThreadStats_)
		{
			os << "Thread ID: " << itr.first << " MinTime: " << itr.second->getMin() << ",";
		}
	}
	auto str = os.str();
	return str.substr(0, str.size() - 1);
}

std::string ServerReport::getSummary()
{
	nlohmann::json stats;
	long min = LONG_MAX, max = 0, count = 0;
	unsigned long avg = 0;
	{
		TAKE_LOCK(statsLock_);
		for (const auto& itr : perThreadStats_)
		{
			auto _max = itr.second->getMax();
			auto _min = itr.second->getMin();
			auto _avg = itr.second->getAvg();
			min = ((min < _min) ? min : _min);
			max = ((max > _max) ? max : _max);
			avg += _avg;
			++count;
		}
	}
	stats["Min"] = min;
	stats["Avg"] = avg;
	stats["Max"] = max;
	return stats.dump();
}

ServerReport::ServerReport()
{
	statsLock_ = std::make_unique<SpinLock>("StatsLock");
}

void ServerReport::reportTxnTime(long timeTaken)
{
	auto tid = currentThread->getTid();
	TAKE_LOCK(statsLock_);
	const auto& it = perThreadStats_.find(tid);
	if (it == perThreadStats_.end())
	{
		auto stats = std::make_unique<ThreadStats>();
		*stats += timeTaken;
		perThreadStats_.emplace(tid, std::move(stats));
	}
	else
	{
		*it->second += timeTaken;
	}
}
