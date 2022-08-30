//
// Created by Khubaib.Umer on 8/25/2022.
//

#include "../include/Dictionary.h"

Dictionary* Dictionary::getInstance()
{
	static Dictionary instance;
	return &instance;
}

Dictionary::Dictionary()
	: failure_("Invalid Key"), duplicate_("Key Already Exists. Try Update"),
	  success_("Operation Successful"), getFailOpsCnt_(0), getOpsCnt_(0), getSuccessOpsCnt_(0),
	  statTypeMap_({{ "max", StatType::MAX_TIME },
	                { "min", StatType::MIN_TIME },
	                { "avg", StatType::AVG_TIME },
	                { "summary", StatType::SUMMARY },
	                { "dictionary", StatType::DICTIONARY }})
{
}

Result Dictionary::fetch(const std::string& key)
{
	++getOpsCnt_;
	if (filter_->search(key))
	{
		if (dictionary_.find(key) != dictionary_.end())
		{
			++getSuccessOpsCnt_;
			return { ResultStatus::SUCCESS, dictionary_[key] };
		}
	}
	++getFailOpsCnt_;
	return { ResultStatus::FAILED, failure_ };
}

Result Dictionary::insert(const std::string& key, const std::string& value)
{
	if (filter_->search(key))
	{
		if (dictionary_.find(key) != dictionary_.end())
		{
			return { ResultStatus::FAILED, duplicate_ };
		}
	}
	dictionary_.emplace(key, value);
	filter_->insert(key);
	return { ResultStatus::SUCCESS, success_ };
}

Result Dictionary::update(const std::string& key, const std::string& value)
{
	if (filter_->search(key))
	{
		if (dictionary_.find(key) != dictionary_.end())
		{
			dictionary_[key] = value;
			return { ResultStatus::SUCCESS, success_ };
		}
	}
	return { ResultStatus::FAILED, failure_ };
}

Result Dictionary::getStats(const std::string& statType) const
{
	switch (GetStatType(statType))
	{
	case StatType::MAX_TIME:
		return { ResultStatus::FAILED, { "Not Available" }};
	case StatType::MIN_TIME:
		return { ResultStatus::FAILED, { "Not Available" }};
	case StatType::AVG_TIME:
		return { ResultStatus::FAILED, { "Not Available" }};
	case StatType::SUMMARY:
		return { ResultStatus::FAILED, { "Not Available" }};
	case StatType::DICTIONARY:
		return { ResultStatus::SUCCESS, this->stats() };
	default:
		return { ResultStatus::FAILED, failure_ };
	}
}