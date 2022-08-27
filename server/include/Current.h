//
// Created by Khubaib.Umer on 8/26/2022.
//
#pragma once

#include <unistd.h>
#include <syscall.h>
#include <memory>
#include <semaphore.h>
#include <list>
#include "Client.h"
#include "LockLessQ.h"

struct RequestNode
{
	Communication::ServerRequest* req_;
	std::atomic<RequestNode*> next_;

	explicit RequestNode(Communication::ServerRequest* ptr) : req_(ptr), next_(nullptr)
	{
	}

	RequestNode() = delete;
};

class CurrentThread
{
public:
	static CurrentThread* getInstance();

	[[nodiscard]] tid_t getTid() const
	{
		return tid_;
	}

	void PostJob(Communication::ServerRequest* req)
	{
		jobQ_->enqueue(new RequestNode(req));
		++currentLoad_;
		sem_post(&sem_);
	}

	Communication::ServerRequest* GetJob()
	{
		if (!jobQ_->is_empty())
		{
			auto node = jobQ_->dequeue();
			if (node != nullptr)
			{
				auto job = node->req_;
				delete node;
				return job;
			}
		}
		return nullptr;
	}

	[[nodiscard]] size_t GetLoad() const
	{
		return currentLoad_;
	}

	void WaitForJob()
	{
		sem_wait(&sem_);
	}

private:
	CurrentThread();

private:
	tid_t tid_;
	sem_t sem_{};
	std::atomic_uint64_t currentLoad_;
	std::unique_ptr<LockLessQ<RequestNode>> jobQ_;
};

#define currentThread CurrentThread::getInstance()
