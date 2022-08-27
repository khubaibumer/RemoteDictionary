//
// Created by Khubaib.Umer on 8/27/2022.
//

#include "../include/ThreadPool.h"

std::unordered_map<tid_t, CurrentThread*> ThreadPool::threadsRegistry_;

ThreadPool::ThreadPool(size_t numThread)
	: threadCount_(numThread), registryLock_(std::make_unique<SpinLock>("ThreadRegistry"))
{
}

void ThreadPool::Destroy()
{
	for (const auto& th : threadPool_)
	{
		pthread_cancel(th);
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
	while (_this->server_->isRunning_)
	{
		Communication::ServerRequest* req;
		currentThread->WaitForJob();
		req = currentThread->GetJob();
		if (req != nullptr)
		{
			std::string response;
			switch (req->getRequestType())
			{
			case Communication::RequestType::REMOVE_CLIENT:
				_this->server_->RemoveClient(req);
				break;
			case Communication::RequestType::SERVE_CLIENT:
				(void)_this->server_->SendResponse(req->getFd(), Communication::Server::GetResponse(req));
				break;
			default:
				break;
			}
		}
		delete req;
	}
	return nullptr;
}

void ThreadPool::AddJob(Communication::ServerRequest* request)
{
	auto thread = GetSuitableThread();
	thread->PostJob(request);
}

CurrentThread* ThreadPool::GetSuitableThread()
{
	const auto& min = std::min_element(threadsRegistry_.begin(), threadsRegistry_.end(),
		[](const auto& l, const auto& r)
		{
		  return l.second->GetLoad() < r.second->GetLoad();
		});
	return min->second;
}

