//
// Created by Khubaib.Umer on 8/27/2022.
//

#pragma once

#include <unistd.h>
#include <getopt.h>
#include "Types.h"

static ProgramOptions getProgramOptions(int argc, char** argv, const char* short_opts, option* long_options)
{
	char ch;
	ProgramOptions ops;
	while ((ch = getopt_long(argc, argv, short_opts, long_options, nullptr)) != -1)
	{
		switch (ch)
		{
		case 'i':
			ops.ip_ = optarg;
			break;
		case 'p':
			ops.port_ = atoi(optarg);
			break;
		case 'j':
			ops.threadCount_ = atoi(optarg);
			break;
		case 'f':
			ops.enableFilter_ = true;
			break;
		case 'b':
			ops.benchmark_ = true;
			break;
		default:
			break;
		}
	}
	return ops;
}
