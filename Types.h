//
// Created by Khubaib.Umer on 8/25/2022.
//

#pragma once
#include <iostream>
#include <string.h>

using tid_t = long;

enum class ServerRequestType : int8_t
{
	INVALID = -1,
	GET,
	SET,
	UPDATE,
	STATS
};

enum class ResultStatus : uint8_t
{
	FAILED = 0,
	SUCCESS
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

struct ProgramOptions
{
	ProgramOptions() : threadCount_(0), port_(50003), ip_("127.0.0.1"), enableFilter_(false), benchmark_(false)
	{
	}

	int threadCount_;
	int port_;
	bool enableFilter_;
	bool benchmark_;
	std::string ip_;
};

struct Result
{
public:
	Result(ResultStatus status, std::string value) : status_(status), value_(std::move(value))
	{
	}

	[[nodiscard]] const std::string& getValue() const
	{
		return value_;
	}

	[[nodiscard]] ResultStatus getStatus() const
	{
		return status_;
	}

private:
	ResultStatus status_;
	const std::string value_;
};

struct LV
{
	LV() : len_(0)
	{
	}
	LV(const char* in, size_t len) : len_(len)
	{
		memcpy(buffer_, in, len);
	}
	size_t len_;
	char buffer_[512] = { 0 };
};
