//
// Created by Khubaib.Umer on 8/26/2022.
//

#include <cassert>
#include "../include/Thread.h"
#include "../include/ServerRequest.h"
#include "../include/Server.h"

static constexpr size_t kMaxEvents = 64;

Thread::Thread() : currentLoad_(0), isRunning_(true)
{
	tid_ = syscall(SYS_gettid);
}

Thread::~Thread()
{
	for (const auto& client : clients_)
	{
		shutdown(client->getFd(), SHUT_RDWR);
		close(client->getFd());
	}
	free(clientEvents_);
}

Thread* Thread::getInstance()
{
	static thread_local Thread instance;
	return &instance;
}

void Thread::AddClient(std::unique_ptr<Communication::Client> client)
{
	std::cout << "New Client added! " << client->str() << std::endl;
	auto fd = client->getFd();
	event_.data.fd = fd;
	event_.events = EPOLLIN | EPOLLET;
	clients_.push_back(std::move(client));
	assert(epoll_ctl(efd_, EPOLL_CTL_ADD, fd, &event_) != -1);
	++currentLoad_;
}

void Thread::Run()
{
	efd_ = epoll_create1(0);
	assert(efd_ != -1);

	clientEvents_ = static_cast<epoll_event*>(calloc(kMaxEvents, sizeof event_));
	assert(clientEvents_ != nullptr);
	while (isRunning_)
	{
		auto ready = epoll_wait(efd_, clientEvents_, kMaxEvents, -1);
		for (auto i = 0; i < ready && i < kMaxEvents; ++i)
		{
			int fd = clientEvents_[i].data.fd;
			if ((clientEvents_[i].events & EPOLLERR) ||
				(clientEvents_[i].events & EPOLLHUP) ||
				(!(clientEvents_[i].events & EPOLLIN)))
			{
				RemoveClient(fd);
			}
			else
			{
				auto msgSz = read(fd, &inMsg_, sizeof inMsg_);
				if (msgSz == -1)
				{
					// errno should be EAGAIN
					if (errno != EAGAIN)
					{
						// Close this client
						// Remove from client Map
						RemoveClient(fd);
					}
				}
				else if (msgSz == 0)
				{
					// Close this client
					// Remove from client Map
					RemoveClient(fd);
				}
				else
				{
					inMsg_.buffer_[msgSz] = 0x00;
					inMsg_.buffer_[msgSz + 1] = 0x00;
					auto req = std::make_unique<Communication::ServerRequest>(fd, inMsg_.buffer_, msgSz);
					auto response = Communication::Server::GetResponse(req);
					req->SetResponseSize(Communication::Server::SendResponse(fd, response));
				}
			}
		}
	}
}
