//
// Created by Khubaib.Umer on 8/25/2022.
//

#pragma once
#include <iostream>

using tid_t = long;

enum class ServerRequestType : int8_t
{
	INVALID = -1,
	GET,
	SET,
	UPDATE,
	STATS
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

enum class ResultStatus : uint8_t
{
	FAILED = 0,
	SUCCESS
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