//
// Created by Khubaib.Umer on 8/30/2022.
//

#pragma once
#include <atomic>
#include <semaphore.h>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "LockLessQ.h"

class Stats;

struct TxnData
{
	explicit TxnData(size_t reqLen, size_t respLen, size_t time) : time_(time), reqLen_(reqLen), respLen_(respLen)
	{
	}

	const size_t respLen_;
	const size_t time_;
	const size_t reqLen_;
};

struct StatNode
{
	TxnData* stat_;
	std::atomic<StatNode*> next_;

	explicit StatNode(TxnData* ptr) : stat_(ptr), next_(nullptr)
	{
	}

	StatNode() = delete;
};

class Stats
{
public:
	static Stats* getInstance();

	static void ReportTxn(TxnData* node);

	[[nodiscard]] std::string GetSummary() const
	{
		auto out = static_cast<double>(responseSz_.load()) / 1000.0f;
		auto in = static_cast<double>(requestSz_.load()) / 1000.0f;
		auto time = static_cast<double>(timeTaken_.load()) / 1000.0f;
		auto speed = ((time == 0) ? 0 : ((in + out) / time)) * 8;
		nlohmann::json summary = {
			{ "KB (Out)", out },
			{ "KB (In)", in },
			{ "TimeTaken (ms)", time },
			{ "Throughput (Mbps)", speed },
		};
		return summary.dump();
	}

	[[nodiscard]] std::string GetMaxTime() const
	{
		nlohmann::json summary = {{ "MaxTimeTaken (us)", maxTimeTaken_.load() }};
		return summary.dump();
	}

	[[nodiscard]] std::string GetMinTime() const
	{
		nlohmann::json summary = {{ "MinTimeTaken (us)", minTimeTaken_.load() }};
		return summary.dump();
	}

	[[nodiscard]] std::string GetAvgTime() const
	{
		auto avg = static_cast<size_t>(timeTaken_.load() / reqCount_.load());
		nlohmann::json summary = {{ "AvgTimeTaken (us)", avg }};
		return summary.dump();
	}

private:
	Stats();

	[[noreturn]] static void* StatsCalculator(void* args);

private:
	static LockLessQ<StatNode> stats_;
	pthread_t statCalculator_{};
	std::atomic_uint64_t timeTaken_;
	std::atomic_uint64_t maxTimeTaken_;
	std::atomic_uint64_t minTimeTaken_;
	std::atomic_uint64_t reqCount_;
	std::atomic_uint64_t responseSz_;
	std::atomic_uint64_t requestSz_;
};
