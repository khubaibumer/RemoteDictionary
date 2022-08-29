//
// Created by Khubaib.Umer on 8/26/2022.
//
#pragma once

#include <unistd.h>
#include <syscall.h>
#include <memory>
#include <semaphore.h>
#include <list>
#include <sys/epoll.h>
#include "Client.h"
#include "LockLessQ.h"
#include "Server.h"

//struct RequestNode
//{
//	Communication::ServerRequest* req_;
//	std::atomic<RequestNode*> next_;
//
//	explicit RequestNode(Communication::ServerRequest* ptr) : req_(ptr), next_(nullptr)
//	{
//	}
//
//	RequestNode() = delete;
//};

class Thread
{
public:
	~Thread();

	static Thread* getInstance();

	[[nodiscard]] tid_t getTid() const
	{
		return tid_;
	}

	void AddClient(std::unique_ptr<Communication::Client> client);

	void RemoveClient(int fd)
	{
		auto client = clientMap_.find(fd);
		if (client != clientMap_.end())
		{
			clientMap_.erase(fd);
			--currentLoad_;
		}
	}

	[[nodiscard]] size_t GetLoad() const
	{
		return currentLoad_;
	}

	void Run();


	void Stop()
	{
		isRunning_ = false;
	}

private:
	Thread();

private:
	tid_t tid_;
	sem_t sem_{};
	std::atomic_uint64_t currentLoad_;
	std::atomic_bool isRunning_;
	int efd_{};
	epoll_event event_{};
	epoll_event* clientEvents_{};
	char* inMsg_;
	const size_t inMsgSize_;
	std::unique_ptr<Communication::ServerRequest> req_;
	std::unordered_map<tid_t, std::unique_ptr<Communication::Client>> clientMap_;
};

#define currentThread Thread::getInstance()