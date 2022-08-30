//
// Created by Khubaib.Umer on 8/26/2022.
//
#pragma once

#include <unistd.h>
#include <syscall.h>
#include <memory>
#include <semaphore.h>
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>
#include "Client.h"
#include "../../Types.h"

class Thread
{
public:
	~Thread();

	static Thread* getInstance();

	void AddClient(std::unique_ptr<Communication::Client> client);

	void Run();

	[[nodiscard]] tid_t getTid() const
	{
		return tid_;
	}

	void RemoveClient(int fd)
	{
		auto client = clientMap_.find(fd);
		if (client != clientMap_.end())
		{
			std::cout << "Removed Client! " << client->second->str() << std::endl;
			clientMap_.erase(fd);
			--currentLoad_;
		}
	}

	[[nodiscard]] size_t GetLoad() const
	{
		return currentLoad_;
	}

	void Stop()
	{
		isRunning_ = false;
	}

private:
	Thread();

private:
	tid_t tid_;
	std::atomic_uint64_t currentLoad_;
	std::atomic_bool isRunning_;
	int efd_{};
	epoll_event event_{};
	epoll_event* clientEvents_{};
	LV inMsg_{};
	std::unordered_map<tid_t, std::unique_ptr<Communication::Client>> clientMap_;
	std::vector<std::unique_ptr<Communication::Client>> clients_;
};

#define currentThread Thread::getInstance()
