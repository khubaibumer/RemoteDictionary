#include <iostream>
#include "../include/Server.h"
#include "../../ProgramOptions.h"

static struct option long_options[] =
	{
		{ "ip_", optional_argument, nullptr, 'i' },
		{ "port_", optional_argument, nullptr, 'p' },
		{ "threads", required_argument, nullptr, 'j' },
		{ "enable_filter", no_argument, nullptr, 'f' },
		{ nullptr, 0, nullptr, 0 }
	};

int main(int argc, char** argv)
{
	auto options = getProgramOptions(argc, argv, "i:p:j:f", long_options);
	auto server = Communication::Server(options);

	server.StartServing();
	std::string msg;
	while (true)
	{
		std::cin >> msg;
		if (msg == "quit")
			break;
	}
	std::cout << "Hello, World!" << std::endl;
	return 0;
}
