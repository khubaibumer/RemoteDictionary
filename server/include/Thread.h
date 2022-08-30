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

/*	void reportTxn(size_t egress, size_t ingress, size_t time)
	{
		*time_ += time;
		*ingress_ += ingress;
		*egress_ += egress;
	}

	[[nodiscard]] size_t getMinTime() const
	{
		return time_->getMin();
	}

	[[nodiscard]] size_t getAvgTime() const
	{
		return time_->getAvg();
	}

	[[nodiscard]] size_t getMaxTime() const
	{
		return time_->getMax();
	}

	[[nodiscard]] size_t getTotalTime() const
	{
		return time_->getSum();
	}

	*//* Egress *//*
	[[nodiscard]] size_t getMinEgress() const
	{
		return egress_->getMin();
	}

	[[nodiscard]] size_t getAvgEgress() const
	{
		return egress_->getAvg();
	}

	[[nodiscard]] size_t getMaxEgress() const
	{
		return egress_->getMax();
	}

	[[nodiscard]] size_t getTotalEgress() const
	{
		return egress_->getSum();
	}

	*//* Ingress *//*
	[[nodiscard]] size_t getMinIngress() const
	{
		return ingress_->getMin();
	}

	[[nodiscard]] size_t getAvgIngress() const
	{
		return ingress_->getAvg();
	}

	[[nodiscard]] size_t getMaxIngress() const
	{
		return ingress_->getMax();
	}

	[[nodiscard]] size_t getTotalIngress() const
	{
		return ingress_->getSum();
	}*/

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
//	std::unique_ptr<Stats> time_;
//	std::unique_ptr<Stats> ingress_;
//	std::unique_ptr<Stats> egress_;
	std::unordered_map<tid_t, std::unique_ptr<Communication::Client>> clientMap_;
};

#define currentThread Thread::getInstance()
