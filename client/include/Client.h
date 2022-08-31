//
// Created by Khubaib.Umer on 8/25/2022.
//

#pragma once

#include <memory>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include "../../Lock.h"

namespace Communication
{

	class Client
	{
	public:
		explicit Client(const std::pair<std::string, uint16_t>& url);

		~Client();

		Client& operator=(const Client&) = delete;

		void ConnectServer();

		bool SendToServer(const std::string& msg);

		[[nodiscard]] std::string GetResponse();

	private:
		static std::string CreateGetRequest(const std::string& msg);

		static std::string CreateSetRequest(const std::string& msg);

		static std::string CreateUpdateRequest(const std::string& msg);

		static std::string CreateStatsRequest(const std::string& msg);

	private:
		int fd_{};
		uint16_t port_{};
		std::string ip_;
		sockaddr_in addr_{};
		socklen_t addrLen_{};
		std::mutex lock_;
	};
} // Communication
