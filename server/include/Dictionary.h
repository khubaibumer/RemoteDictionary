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
#include "../../Types.h"

class Dictionary
{
public:
	static Dictionary* getInstance();

	Result fetch(const std::string& key);

	Result insert(const std::string& key, const std::string& value);

	Result update(const std::string& key, const std::string& value);

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

	[[nodiscard]] std::string stats() const
	{
		std::ostringstream os;
		os << "Total Get Operations: " << getOpsCnt_ << ", "
		   << "Total Successful Get Operations: " << getSuccessOpsCnt_ << ", "
		   << "Total Failed Get Operations: " << getFailOpsCnt_;
		return os.str();
	}

private:
	Dictionary();

private:
	const std::string failure_;
	const std::string duplicate_;
	const std::string success_;
	const std::string invalidStat_;
	std::unique_ptr<Filter> filter_;
	std::unordered_map<std::string, std::string> dictionary_;
	std::atomic_int64_t getOpsCnt_;
	std::atomic_int64_t getSuccessOpsCnt_;
	std::atomic_int64_t getFailOpsCnt_;
};
