//
// Created by Khubaib.Umer on 8/26/2022.
//

#include "../include/Current.h"

CurrentThread::CurrentThread() : currentLoad_(0)
{
	tid_ = syscall(SYS_gettid);
	sem_init(&sem_, 0, 0);
	jobQ_ = std::make_unique<LockLessQ<RequestNode>>();
}

CurrentThread* CurrentThread::getInstance()
{
	static thread_local CurrentThread instance;
	return &instance;
}
