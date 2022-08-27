//
// Created by Khubaib.Umer on 8/27/2022.
//

#pragma once

#include <cstddef>
#include <unordered_map>
#include "Current.h"
#include "Server.h"

class ThreadPool
{
public:
	explicit ThreadPool(size_t numThread);

	bool Ignite();

	void RegisterThread();

	void Destroy();

	void SetServerInstance(Communication::Server* server)
	{
		server_ = server;
	}

	static void AddJob(Communication::ServerRequest* request);

private:
	static void* WorkerThreadRoutine(void* arg);

	static CurrentThread* GetSuitableThread();

private:
	std::unique_ptr<SpinLock> registryLock_;
	const size_t threadCount_;
	Communication::Server* server_{};
	std::vector<pthread_t> threadPool_;
	static std::unordered_map<tid_t, CurrentThread*> threadsRegistry_;
};

