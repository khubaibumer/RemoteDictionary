//
// Created by Khubaib.Umer on 8/26/2022.
//

#include <sstream>
#include <nlohmann/json.hpp>
#include <climits>
#include "../include/ServerReport.h"
#include "../include/ThreadPool.h"
#include "../include/Thread.h"

ServerReport* ServerReport::getInstance()
{
	static ServerReport instance;
	return &instance;
}

std::string ServerReport::getAvgResponseTime()
{
	std::ostringstream os;
//	for (const auto& [id, thread] : ThreadPool::threadsRegistry_)
//	{
//		os << "Thread ID: " << id << " AvgTime: " << thread->getAvgTime() << ",";
//	}
	auto str = os.str();
	return str.substr(0, str.size() - 1);
}

[[maybe_unused]] std::string ServerReport::getMaxResponseTime()
{
	std::ostringstream os;
//	for (const auto& [id, thread] : ThreadPool::threadsRegistry_)
//	{
//		os << "Thread ID: " << id << " AvgTime: " << thread->getMaxTime() << ",";
//	}
	auto str = os.str();
	return str.substr(0, str.size() - 1);
}

std::string ServerReport::getMinResponseTime()
{
	std::ostringstream os;
//	for (const auto& [id, thread] : ThreadPool::threadsRegistry_)
//	{
//		os << "Thread ID: " << id << " AvgTime: " << thread->getMinTime() << ",";
//	}
	auto str = os.str();
	return str.substr(0, str.size() - 1);
}

std::string ServerReport::getSummary()
{
	nlohmann::json stats;
	size_t totalTime = 0, totalEgress = 0, totalIngress = 0, count = 0;
//	{
//		for (const auto& [id, thread] : ThreadPool::threadsRegistry_)
//		{
//			if (thread->GetLoad() != 0)
//			{
//				totalTime += thread->getTotalTime();
//				totalEgress += thread->getTotalEgress();
//				totalIngress += thread->getTotalIngress();
//				++count;
//			}
//		}
//	}
//	auto inSpeed = (8 * totalIngress) / (totalTime / 1000);
//	auto outSpeed = (8 * totalEgress) / (totalTime / 1000);
//	auto speed = inSpeed + outSpeed;
//	stats["Throughput Ingress (kbps)"] = inSpeed;
//	stats["Throughput Egress (kbps)"] = outSpeed;
//	stats["Throughput (kbps)"] = speed;
//	return stats.dump();
	return {};
}

ServerReport::ServerReport() /*: statsLock_(std::make_unique<SpinLock>("StatsLock"))*/
{
}
