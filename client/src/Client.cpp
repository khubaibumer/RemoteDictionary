//
// Created by Khubaib.Umer on 8/25/2022.
//

#include <cassert>
#include <arpa/inet.h>
#include <cstring>
#include <csignal>
#include <nlohmann/json.hpp>
#include "../include/Client.h"
#include "../../Types.h"

namespace Communication
{
	constexpr uint16_t kInvalidPort = 0;

	Client::Client(const std::pair<std::string, uint16_t>& url)
		: ip_(url.first), port_(url.second)
	{
		assert(port_ != kInvalidPort);
	}

	Client::~Client()
	{
		shutdown(fd_, SHUT_RDWR);
		close(fd_);
	}

	void Client::ConnectServer()
	{
		fd_ = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_ != -1);

		addr_.sin_family = AF_INET;
		addr_.sin_port = htons(port_);
		addr_.sin_addr.s_addr = inet_addr(ip_.c_str());
		memset(addr_.sin_zero, 0, sizeof addr_.sin_zero);
		addrLen_ = sizeof addr_;
		assert(0 == connect(fd_, (sockaddr*)&addr_, addrLen_));
	}

	bool Client::SendToServer(const std::string& msg) const
	{
		auto pos = msg.find(' ');
		auto type = msg.substr(0, pos);
		if (pos == std::string::npos)
			return false;

		auto req = msg.substr(pos + 1, msg.size());
		std::string request;
		if (type == "GET")
		{
			request = CreateGetRequest(req);
		}
		else if (type == "SET")
		{
			request = CreateSetRequest(req);
		}
		else if (type == "UPDATE")
		{
			request = CreateUpdateRequest(req);
		}
		else if (type == "STATS")
		{
			request = CreateStatsRequest(req);
		}

		if (request.empty())
		{
			std::cerr << "Invalid Request" << std::endl;
			return false;
		}

		LV outMsg(request.c_str(), request.size());
		if (write(fd_, &outMsg, sizeof outMsg) == -1)
		{
			perror("Send To Server Failed ");
			return false;
		}

		return true;
	}

	std::string Client::GetResponse() const
	{
		LV data;
		size_t bytes;

		bytes = read(fd_, &data, sizeof data);
		if (bytes > 0)
		{
			auto response = nlohmann::json::parse(data.buffer_);
			return response.dump();
		}
		return {};
	}
	std::string Client::CreateGetRequest(const std::string& msg)
	{
		nlohmann::json request = {{ "REQUEST", ServerRequestType::GET }, { "KEY", msg }};
		return request.dump();
	}

	std::string Client::CreateSetRequest(const std::string& msg)
	{
		auto pos = msg.find(' ');
		if (pos != std::string::npos)
		{
			auto key = msg.substr(0, pos);
			auto value = msg.substr(pos + 1, msg.size());
			if (!value.empty())
			{
				nlohmann::json request = {{ "REQUEST", ServerRequestType::SET }, { "KEY", key }, { "VALUE", value }};
				return request.dump();
			}
		}
		return {};
	}

	std::string Client::CreateUpdateRequest(const std::string& msg)
	{
		auto pos = msg.find(' ');
		if (pos != std::string::npos)
		{
			auto key = msg.substr(0, pos);
			auto value = msg.substr(pos + 1, msg.size());
			if (!value.empty())
			{
				nlohmann::json request = {{ "REQUEST", ServerRequestType::UPDATE }, { "KEY", key }, { "VALUE", value }};
				return request.dump();
			}
		}
		return {};
	}

	std::string Client::CreateStatsRequest(const std::string& msg)
	{
		auto pos = msg.find(' ');
		auto key = msg.substr(0, pos);
		if (!key.empty())
		{
			nlohmann::json request = {{ "REQUEST", ServerRequestType::STATS }, { "KEY", key }};
			return request.dump();
		}
		return {};
	}

} // Communication
