//
// Created by Khubaib.Umer on 8/25/2022.
//

#pragma once
#include <iostream>

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
	ProgramOptions() : threadCount_(0), port_(50003), ip_("127.0.0.1"), enableFilter_(false), benchmark_(true)
	{
	}

	int threadCount_;
	int port_;
	bool enableFilter_;
	bool benchmark_;
	std::string ip_;
};

using tid_t = long;

