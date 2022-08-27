//
// Created by Khubaib.Umer on 8/25/2022.
//

#pragma once

#include <sys/socket.h>
#include <cstring>
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

	private:
		int fd_;
		sockaddr addr_{};
		socklen_t addrLen_{};
		std::string host_;
		std::string port_;
	};

	enum class RequestType : int8_t
	{
		SERVE_CLIENT = 11,
		REMOVE_CLIENT
	};

	struct ServerRequest
	{
		ServerRequest(RequestType type, int cfd, char* req, size_t reqLen)
			: type_(type), cfd_(cfd), request_(req), requestLen_(reqLen)
		{
			start_ = std::chrono::high_resolution_clock::now();
		}

		ServerRequest(RequestType type, int cfd)
			: type_(type), cfd_(cfd)
		{
			start_ = std::chrono::high_resolution_clock::now();
		}

		~ServerRequest()
		{
			end_ = std::chrono::high_resolution_clock::now();
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
			if (type_ == RequestType::SERVE_CLIENT)
			{
				reporter->reportTxnTime(microseconds);
			}
		}

		[[nodiscard]] RequestType getRequestType() const
		{
			return type_;
		}

		[[nodiscard]] constexpr int getFd() const
		{
			return cfd_;
		}

		[[nodiscard]] std::string str() const
		{
			return request_ + " received from Client Id: " + std::to_string(cfd_);
		}

		[[nodiscard]] const std::string& getRequest() const
		{
			return request_;
		}

	private:
		RequestType type_;
		std::string request_;
		size_t requestLen_{};
		std::chrono::high_resolution_clock::time_point start_;
		std::chrono::high_resolution_clock::time_point end_;
		int cfd_;
	};

} // Communication
