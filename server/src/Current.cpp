//
// Created by Khubaib.Umer on 8/26/2022.
//

#include <cassert>
#include <cxxabi.h>
#include "../include/Current.h"

static constexpr size_t kMaxEvents = 64;

Thread::Thread() : currentLoad_(0), inMsgSize_(1024), isRunning_(true)
{
	tid_ = syscall(SYS_gettid);
	sem_init(&sem_, 0, 0);
	inMsg_ = static_cast<char*>(calloc(inMsgSize_, sizeof(char)));
}

Thread::~Thread()
{
	for (const auto& [id, client] : clientMap_)
	{
		shutdown(client->getFd(), SHUT_RD);
		close(client->getFd());
	}
	free(clientEvents_);
	free(inMsg_);
}

Thread* Thread::getInstance()
{
	static thread_local Thread instance;
	return &instance;
}

void Thread::AddClient(std::unique_ptr<Communication::Client> client)
{
	event_.data.fd = client->getFd();
	event_.events = EPOLLIN | EPOLLET;
	assert(epoll_ctl(efd_, EPOLL_CTL_ADD, client->getFd(), &event_) != -1);
	clientMap_.emplace(client->getFd(), std::move(client));
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
			if ((clientEvents_[i].events & EPOLLERR) ||
				(clientEvents_[i].events & EPOLLHUP) ||
				(!(clientEvents_[i].events & EPOLLIN)))
			{
				RemoveClient(clientEvents_[i].data.fd);
				continue;
			}
			else
			{
				auto msgSz = read(clientEvents_[i].data.fd, &inMsg_[0], inMsgSize_);
				int fd = clientEvents_[i].data.fd;
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
					inMsg_[msgSz] = 0x00;
					inMsg_[msgSz + 1] = 0x00;
					const auto& client = clientMap_[fd];
					req_ = std::make_unique<Communication::ServerRequest>(fd, inMsg_, msgSz);
					Communication::Server::SendResponse(client, Communication::Server::GetResponse(req_));
				}
			}
		}
	}
}
