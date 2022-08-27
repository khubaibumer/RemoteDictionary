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
		clientLock_ = std::make_unique<SpinLock>("ClientLock");
		StartServer();
	}

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
		CloseAllClients();
		shutdown(fd_, SHUT_RDWR);
		close(fd_);
	}

	void Server::CloseAllClients()
	{
		TAKE_LOCK(clientLock_);
		for (const auto& [fd, client] : clientMap_)
		{
			close(fd);
		}
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
		threadPool_->SetServerInstance(this);
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
					_this->threadPool_->AddJob(new ServerRequest(RequestType::REMOVE_CLIENT,
						_this->clientEvents_[i].data.fd));
					continue;
				}
				else if (_this->fd_ == _this->clientEvents_[i].data.fd)
				{
					while (true)
					{
						auto clientFd = _this->AcceptClient();
						if (clientFd != -1)
						{
							_this->MakeNonBlockSocket(clientFd);
							_this->serverEvent_.data.fd = clientFd;
							_this->serverEvent_.events = EPOLLIN | EPOLLET;
							assert(epoll_ctl(_this->efd_, EPOLL_CTL_ADD, clientFd, &_this->serverEvent_) != -1);
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					char msg[512] = { 0 };
					auto msgSz = read(_this->clientEvents_[i].data.fd, msg, sizeof msg - 1);
					if (msgSz == -1)
					{
						// errno should be EAGAIN
						if (errno != EAGAIN)
						{
							// Close this client
							// Remove from client Map
							_this->threadPool_->AddJob(new ServerRequest(RequestType::REMOVE_CLIENT,
								_this->clientEvents_[i].data.fd));
						}
					}
					else if (msgSz == 0)
					{
						// Close this client
						// Remove from client Map
						_this->threadPool_->AddJob(new ServerRequest(RequestType::REMOVE_CLIENT,
							_this->clientEvents_[i].data.fd));
					}
					else
					{
						_this->threadPool_->AddJob(new ServerRequest(RequestType::SERVE_CLIENT,
							_this->clientEvents_[i].data.fd, msg, msgSz));
					}
				}
			}
		}

		return nullptr;
	}

	int Server::AcceptClient()
	{
		sockaddr addr{};
		socklen_t len = sizeof addr;

		char host[NI_MAXHOST] = { 0 }, port[NI_MAXSERV] = { 0 };

		auto cfd = accept(fd_, &addr, &len);
		if (cfd == -1)
		{
			return cfd;
		}

		auto ret = getnameinfo(&addr, len, host, sizeof host, port, sizeof port, NI_NUMERICHOST | NI_NUMERICSERV);
		if (ret == 0)
		{
			auto client = std::make_shared<Client>(cfd, host, port);
			client->setAddr(&addr);
			client->setAddrLen(len);
			TAKE_LOCK(clientLock_);
			clientMap_.emplace(cfd, client);
			return cfd;
		}
		return -1;
	}

	void Server::RemoveClient(ServerRequest* request)
	{
		const auto cfd = request->getFd();
		TAKE_LOCK(clientLock_);
		auto it = clientMap_.find(cfd);
		if (it == clientMap_.end())
		{
			// Shouldn't happen
			return;
		}
		auto client = it->second;
		clientMap_.erase(it);
		close(client->getFd());
	}

	std::string Server::GetResponse(ServerRequest* req)
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

	size_t Server::SendResponse(int fd, const std::string& response)
	{
		std::shared_ptr<Client> client;
		{
			TAKE_LOCK(clientMap_);
			auto itr = clientMap_.find(fd);
			if (itr == clientMap_.end() || itr->second == nullptr)
				return 0;
			client = itr->second;
		}
		return sendto(client->getFd(), response.c_str(), response.size(), 0, client->getAddr(), client->getAddrLen());
	}

	std::string Server::ConsumeGetRequest(const nlohmann::json& req)
	{
		const auto& key = req["KEY"];
		auto response = Dictionary::getInstance()->fetch(key);
		nlohmann::json rep = {
			{ "REQUEST", ServerRequestType::GET },
			{ "KEY", key },
			{ "STATUS", response.status_ },
			{ "VALUE", response.value_ }
		};
		return rep.dump();
	}
	std::string Server::ConsumeSetRequest(const nlohmann::json& req)
	{
		const auto& key = req["KEY"];
		auto response = Dictionary::getInstance()->insert(key, req["VALUE"]);
		nlohmann::json rep = {
			{ "REQUEST", ServerRequestType::SET },
			{ "KEY", key },
			{ "STATUS", response.status_ },
			{ "VALUE", response.value_ }
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
			{ "STATUS", response.status_ },
			{ "VALUE", response.value_ }
		};
		return rep.dump();
	}

	std::string Server::ConsumeStatsRequest(const nlohmann::json& req)
	{
		auto response = Dictionary::getInstance()->getStats(req["KEY"]);
		nlohmann::json rep = {
			{ "REQUEST", ServerRequestType::STATS },
			{ "STATUS", response.status_ },
			{ "VALUE", response.value_ }
		};
		return rep.dump();
	}

} // Communication