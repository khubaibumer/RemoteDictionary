//
// Created by Khubaib.Umer on 8/25/2022.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <utility>
#include <sstream>
#include <atomic>
#include "Filter.h"
#include "BloomFilter.h"
#include "NoFilter.h"

enum class ResultStatus : bool
{
	FAILED = false,
	SUCCESS = true
};

enum class StatType : int8_t
{
	INVALID = -1,
	MIN_TIME,
	AVG_TIME,
	MAX_TIME,
	SUMMARY,
	DICTIONARY
};

struct Result
{
	Result(ResultStatus status, std::string value) : status_(status), value_(std::move(value))
	{
	}

	ResultStatus status_;
	std::string value_;
};

class Dictionary
{
public:
	static Dictionary* getInstance();

	Result fetch(const std::string& key);

	Result insert(const std::string& key, const std::string& value);

	Result update(const std::string& key, const std::string& value);

	Result getStats(const std::string& statType) const;

	void enableFilter(bool enableFilter)
	{
		if (enableFilter)
		{
			filter_ = std::make_unique<BloomFilter>();
		}
		else
		{
			filter_ = std::make_unique<NoFilter>();
		}
	}

private:
	Dictionary();

	[[nodiscard]] std::string stats() const
	{
		std::ostringstream os;
		os << "Total Get Operations: " << getOpsCnt_ << ", "
		   << "Total Successful Get Operations: " << getSuccessOpsCnt_ << ", "
		   << "Total Failed Get Operations: " << getFailOpsCnt_;
		return os.str();
	}

	StatType GetStatType(const std::string& stat) const
	{
		const auto& it = statTypeMap_.find(stat);
		return ((it == statTypeMap_.end()) ? StatType::INVALID : it->second);
	}

private:
	const std::string failure_;
	const std::string duplicate_;
	const std::string success_;
	const std::string invalidStat_;
	const std::unordered_map<std::string, StatType> statTypeMap_;
	std::unique_ptr<Filter> filter_;
	std::unordered_map<std::string, std::string> dictionary_;
	std::atomic_int64_t getOpsCnt_;
	std::atomic_int64_t getSuccessOpsCnt_;
	std::atomic_int64_t getFailOpsCnt_;
};
