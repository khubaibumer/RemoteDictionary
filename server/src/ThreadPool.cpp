//
// Created by Khubaib.Umer on 8/27/2022.
//

#include "../include/ThreadPool.h"

std::unordered_map<tid_t, Thread*> ThreadPool::threadsRegistry_;

ThreadPool::ThreadPool(size_t numThread)
	: threadCount_(numThread), registryLock_(std::make_unique<SpinLock>("ThreadRegistry"))
{
}

void ThreadPool::Destroy()
{
	{
		TAKE_LOCK(registryLock_);
		for (const auto& [id, thr] : threadsRegistry_)
		{
			thr->Stop();
		}
	}
	for (const auto& tid : threadPool_)
	{
		pthread_cancel(tid);
	}
}

bool ThreadPool::Ignite()
{
	bool status = true;
	for (auto i = 0; i < threadCount_; ++i)
	{
		pthread_t tid;
		status &= (0 == pthread_create(&tid, nullptr, &ThreadPool::WorkerThreadRoutine, this));
		pthread_detach(tid);
		threadPool_.push_back(tid);
	}
	return status;
}

void ThreadPool::RegisterThread()
{
	auto thread = currentThread;
	auto tid = thread->getTid();
	TAKE_LOCK(registryLock_);
	threadsRegistry_.emplace(tid, thread);
}

void* ThreadPool::WorkerThreadRoutine(void* arg)
{
	auto _this = (ThreadPool*)arg;
	_this->RegisterThread();
	currentThread->Run();
	return nullptr;
}

void ThreadPool::AddClient(std::unique_ptr<Communication::Client> client)
{
	auto thread = GetSuitableThread();
	thread->AddClient(std::move(client));
}

Thread* ThreadPool::GetSuitableThread()
{
	const auto& min = std::min_element(threadsRegistry_.begin(), threadsRegistry_.end(),
		[](const auto& l, const auto& r)
		{
		  return l.second->GetLoad() < r.second->GetLoad();
		});
	return min->second;
}


