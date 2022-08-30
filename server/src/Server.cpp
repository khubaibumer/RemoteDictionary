//
// Created by Khubaib.Umer on 8/24/2022.
//

#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <netdb.h>
#include "../include/Server.h"
#include "../include/Dictionary.h"
#include "../include/ThreadPool.h"

namespace Communication
{
	constexpr uint16_t kInvalidPort = 0;
	constexpr int kThreadPoolSize = 5;
	constexpr size_t kMaxEvents = 64;

	void Server::Initialize()
	{
		assert(!ip_.empty());
		assert(port_ != kInvalidPort);
		StartServer();
	}

	const std::unordered_map<std::string, StatType> Server::statTypeMap_({{ "max", StatType::MAX_TIME },
	                                                                      { "min", StatType::MIN_TIME },
	                                                                      { "avg", StatType::AVG_TIME },
	                                                                      { "summary", StatType::SUMMARY },
	                                                                      { "dictionary", StatType::DICTIONARY }});
	Server::Server(const ProgramOptions& ops)
		: threadPoolSize_((ops.threadCount_ == 0) ? kThreadPoolSize : ops.threadCount_),
		  ip_(ops.ip_), port_(ops.port_), enableFilter_(ops.enableFilter_), isRunning_(true)
	{
		auto dictionary = Dictionary::getInstance();
		dictionary->enableFilter(enableFilter_);
		Initialize();
	}

	Server::~Server()
	{
		isRunning_ = false;
		threadPool_->Destroy();
		pthread_cancel(rxThread_);
		free(this->clientEvents_);
		shutdown(fd_, SHUT_RD);
		close(fd_);
	}

	void Server::StartServer()
	{
		fd_ = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_ != -1);
		sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(port_) };
		addr.sin_addr.s_addr = inet_addr(ip_.c_str());
		memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

		auto result = (0 == bind(fd_, (sockaddr*)&addr, sizeof addr));
		assert(result == true);

		if (!MakeNonBlockSocket(fd_))
		{
			perror("Unable to make server socket non-block ");
		}
		result = (0 == listen(fd_, SOMAXCONN));
		assert(result == true);
	}

	bool Server::MakeNonBlockSocket(int fd)
	{
		int flags;
		flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1)
		{
			perror(__func__);
			return false;
		}
		flags |= O_NONBLOCK;
		return (-1 != fcntl(fd, F_SETFL, flags));
	}

	void Server::StartServing()
	{
		assert(StartWorkerPool() == true);
		pthread_create(&rxThread_, nullptr, &Server::RxThreadRoutine, this);
		pthread_detach(rxThread_);
	}

	bool Server::StartWorkerPool()
	{
		threadPool_ = std::make_unique<ThreadPool>(threadPoolSize_);
		return threadPool_->Ignite();
	}

	void* Server::RxThreadRoutine(void* args)
	{
		auto _this = (Server*)args;
		_this->efd_ = epoll_create1(0);
		assert(_this->efd_ != -1);

		_this->serverEvent_.data.fd = _this->fd_;
		_this->serverEvent_.events = EPOLLIN | EPOLLET;
		assert(epoll_ctl(_this->efd_, EPOLL_CTL_ADD, _this->fd_, &_this->serverEvent_) != -1);

		_this->clientEvents_ = static_cast<epoll_event*>(calloc(kMaxEvents, sizeof _this->serverEvent_));

		assert(_this->clientEvents_ != nullptr);

		while (_this->isRunning_)
		{
			auto ready = epoll_wait(_this->efd_, _this->clientEvents_, kMaxEvents, -1);
			for (auto i = 0; i < ready && i < kMaxEvents; ++i)
			{
				if ((_this->clientEvents_[i].events & EPOLLERR) ||
					(_this->clientEvents_[i].events & EPOLLHUP) ||
					(!(_this->clientEvents_[i].events & EPOLLIN)))
				{
					continue;
				}
				else if (_this->fd_ == _this->clientEvents_[i].data.fd)
				{
					while (true)
					{
						auto clientPtr = _this->AcceptClient();
						if (clientPtr != nullptr)
						{
							_this->MakeNonBlockSocket(clientPtr->getFd());
							ThreadPool::AddClient(std::move(clientPtr));
						}
						else
						{
							break;
						}
					}
				}
			}
		}

		return nullptr;
	}

	std::unique_ptr<Client> Server::AcceptClient() const
	{
		sockaddr addr{};
		socklen_t len = sizeof addr;

		char host[NI_MAXHOST] = { 0 }, port[NI_MAXSERV] = { 0 };

		auto cfd = accept(fd_, &addr, &len);
		if (cfd == -1)
		{
			return nullptr;
		}

		auto ret = getnameinfo(&addr, len, host, sizeof host, port, sizeof port, NI_NUMERICHOST | NI_NUMERICSERV);
		if (ret == 0)
		{
			auto client = std::make_unique<Client>(cfd, host, port);
			client->setAddr(&addr);
			client->setAddrLen(len);
			return client;
		}
		return {};
	}

	std::string Server::GetResponse(const std::unique_ptr<ServerRequest>& req)
	{
		static const nlohmann::json invalidRequest = {
			{ "REQUEST", "INVALID" },
			{ "STATUS", "FAILED" },
			{ "REASON", "Invalid Request" }
		};

		if (req->getRequest().empty())
		{
			return invalidRequest.dump();
		}
		try
		{
			auto jsonReq = nlohmann::json::parse(req->getRequest());

			switch (jsonReq["REQUEST"].get<ServerRequestType>())
			{
			case ServerRequestType::GET:
				return ConsumeGetRequest(jsonReq);
			case ServerRequestType::SET:
				return ConsumeSetRequest(jsonReq);
			case ServerRequestType::UPDATE:
				return ConsumeUpdateRequest(jsonReq);
			case ServerRequestType::STATS:
				return ConsumeStatsRequest(jsonReq);
			default:
				return invalidRequest.dump();
			}
		}
		catch (const std::exception& ex)
		{
			return invalidRequest.dump();
		}
	}

	size_t Server::SendResponse(const std::unique_ptr<Client>& client, const std::string& response)
	{
		LV resp(response.c_str(), response.size());
		return sendto(client->getFd(), &resp, sizeof resp, 0, client->getAddr(), client->getAddrLen());
	}

	std::string Server::ConsumeGetRequest(const nlohmann::json& req)
	{
		const auto& key = req["KEY"];
		auto response = Dictionary::getInstance()->get(key);
		nlohmann::json rep = {
			{ "REQUEST", ServerRequestType::GET },
			{ "KEY", key },
			{ "STATUS", response.getStatus() },
			{ "VALUE", response.getValue() }
		};
		return rep.dump();
	}

	std::string Server::ConsumeSetRequest(const nlohmann::json& req)
	{
		const auto& key = req["KEY"];
		auto response = Dictionary::getInstance()->set(key, req["VALUE"]);
		nlohmann::json rep = {
			{ "REQUEST", ServerRequestType::SET },
			{ "KEY", key },
			{ "STATUS", response.getStatus() },
			{ "VALUE", response.getValue() }
		};
		return rep.dump();
	}

	std::string Server::ConsumeUpdateRequest(const nlohmann::json& req)
	{
		const auto& key = req["KEY"];
		auto response = Dictionary::getInstance()->update(key, req["VALUE"]);
		nlohmann::json rep = {
			{ "REQUEST", ServerRequestType::UPDATE },
			{ "KEY", key },
			{ "STATUS", response.getStatus() },
			{ "VALUE", response.getValue() }
		};
		return rep.dump();
	}

	std::string Server::ConsumeStatsRequest(const nlohmann::json& req)
	{
		auto response = GetStats(req["KEY"]);
		nlohmann::json rep = {
			{ "REQUEST", ServerRequestType::STATS },
			{ "STATUS", response.getStatus() },
			{ "VALUE", response.getValue() }
		};
		return rep.dump();
	}

	Result Server::GetStats(const std::string& statType)
	{
		switch (GetStatType(statType))
		{
		case StatType::MAX_TIME:
			return { ResultStatus::SUCCESS, Stats::getInstance()->GetMaxTime() };
		case StatType::MIN_TIME:
			return { ResultStatus::SUCCESS, Stats::getInstance()->GetMinTime() };
		case StatType::AVG_TIME:
			return { ResultStatus::SUCCESS, Stats::getInstance()->GetAvgTime() };
		case StatType::SUMMARY:
			return { ResultStatus::SUCCESS, Stats::getInstance()->GetSummary() };
		case StatType::DICTIONARY:
			return { ResultStatus::SUCCESS, Dictionary::getInstance()->stats() };
		default:
			return { ResultStatus::FAILED, "Invalid Key" };
		}
	}

} // Communication