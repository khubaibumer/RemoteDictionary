//
// Created by Khubaib.Umer on 8/27/2022.
//

#pragma once

#include "Filter.h"

class BloomFilter : public Filter
{
public:
	BloomFilter() : bloomFilter_(0)
	{
	}

	bool search(const std::string& key) override
	{
		return bloomFilter_ & getMask(key);
	}

	void insert(const std::string& key) override
	{
		if (!(bloomFilter_ & getMask(key)))
		{
			bloomFilter_ |= getMask(key);
		}
	}

protected:
	uint64_t getMask(const std::string& key) override
	{
		auto hs = (hash_(key) % 64);
		return ((1 << hs) - 1);
	}

private:
	std::hash<std::string> hash_;
	uint64_t bloomFilter_;
};

