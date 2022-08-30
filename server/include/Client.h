//
// Created by Khubaib.Umer on 8/25/2022.
//

#pragma once

#include <sys/socket.h>
#include <cstring>
#include <sstream>
#include "Lock.h"
#include "ServerReport.h"

namespace Communication
{
	class Client
	{
	public:
		Client(int fd, char* host, char* port) : fd_(fd), host_(host), port_(port)
		{
		}

		Client(const Client&) = default;

		explicit Client(const std::pair<std::string, uint16_t>& url);

		[[nodiscard]] const std::string& getHost() const
		{
			return host_;
		}

		[[nodiscard]] const std::string& getPortStr() const
		{
			return port_;
		}

		[[nodiscard]] uint16_t getPort() const
		{
			return atoi(port_.c_str());
		}

		[[nodiscard]] int getFd() const
		{
			return fd_;
		}

		sockaddr* getAddr()
		{
			return &addr_;
		}

		[[nodiscard]] socklen_t getAddrLen() const
		{
			return addrLen_;
		}

		void setAddr(sockaddr* addr)
		{
			memcpy(&addr_, addr, sizeof addr_);
		}

		void setAddrLen(socklen_t len)
		{
			addrLen_ = len;
		}

		[[nodiscard]] std::string str() const
		{
			std::stringstream os;
			os << "Host: " << getHost() << " port: " << getPortStr();
			return os.str();
		}

	private:
		int fd_;
		sockaddr addr_{};
		socklen_t addrLen_{};
		std::string host_;
		std::string port_;
	};
} // Communication
