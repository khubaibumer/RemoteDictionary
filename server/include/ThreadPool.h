//
// Created by Khubaib.Umer on 8/27/2022.
//

#pragma once

#include <cstddef>
#include <unordered_map>
#include "Thread.h"
#include "Server.h"

class ServerReport;

class ThreadPool
{
public:
	explicit ThreadPool(size_t numThread);

	bool Ignite();

	void RegisterThread();

	void Destroy();

	static void AddClient(std::unique_ptr<Communication::Client> client);

private:
	static void* WorkerThreadRoutine(void* arg);

	static Thread* GetSuitableThread();

private:
	friend ServerReport;

	std::unique_ptr<SpinLock> registryLock_;
	const size_t threadCount_;
	std::vector<pthread_t> threadPool_;
	static std::unordered_map<tid_t, Thread*> threadsRegistry_;
};

