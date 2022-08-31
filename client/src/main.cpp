//
// Created by Khubaib.Umer on 8/25/2022.
//

#include <iostream>
#include <vector>
#include "../include/Client.h"
#include "../../ProgramOptions.h"

static struct option long_options[] =
	{
		{ "ip", optional_argument, nullptr, 'i' },
		{ "port", optional_argument, nullptr, 'p' },
		{ "benchmark", optional_argument, nullptr, 'b' },
		{ nullptr, 0, nullptr, 0 }
	};

int main(int argc, char** argv)
{
	std::cout << "Client started..." << std::endl;

	auto options = getProgramOptions(argc, argv, "i:p:b", long_options);
	auto client = Communication::Client(std::make_pair(options.ip_, options.port_));
	client.ConnectServer();

	if (options.benchmark_)
	{
		std::vector<std::string> testCommands = {
			{ "GET somekey" },
			{ "GET some-key" },
			{ "SET some-key value1" },
			{ "SET some-key value1" },
			{ "SET some-key value2" },
			{ "UPDATE some-key value2" },
			{ "SET somekey value3" },
			{ "GET somekey" },
			{ "UPDATE somekey value4" },
			{ "SET somekey value5" },
			{ "GET somekey" },
			{ "GET some-key" },
			{ "GET some-key" },
			{ "GET somekey1" },
			{ "STATS dictionary" }
		};

		for (auto i = 0; i < 100000; ++i)
		{
			for (const auto& it : testCommands)
			{
				client.SendToServer(it);
				auto msg = client.GetResponse();
				std::cout << msg << std::endl;
			}
		}
	}

	while (true)
	{
		std::cout << "Enter Request: " << std::endl;
		std::string inMsg;
		std::getline(std::cin, inMsg);

		if (inMsg == "quit")
			break;

		client.SendToServer(inMsg);
		auto msg = client.GetResponse();
		std::cout << msg << std::endl;
	}
	return 0;
}
