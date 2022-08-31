//
// Created by Khubaib.Umer on 8/27/2022.
//

#pragma once

#include "Filter.h"

class NoFilter : public Filter
{
public:
	NoFilter() = default;

	bool search(const std::string& key) override
	{
		return true;
	}

	void insert(const std::string& key) override
	{
	}

protected:
	uint64_t getMask(const std::string& key) override
	{
		return 0;
	}
};
