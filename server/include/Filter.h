//
// Created by Khubaib.Umer on 8/27/2022.
//

#pragma once

#include <string>

class Filter
{
public:
	Filter() = default;

	virtual ~Filter() = default;

	virtual bool search(const std::string&) = 0;

	virtual void insert(const std::string&) = 0;

protected:
	virtual uint64_t getMask(const std::string& key) = 0;
};

