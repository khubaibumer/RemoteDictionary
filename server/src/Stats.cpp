//
// Created by Khubaib.Umer on 8/30/2022.
//

#include "../include/Stats.h"

LockLessQ<StatNode> Stats::stats_;

void Stats::ReportTxn(TxnData* node)
{
	stats_.enqueue(new StatNode(node));
}

Stats* Stats::getInstance()
{
	static Stats instance;
	return &instance;
}
Stats::Stats()
	: timeTaken_(0), responseSz_(0), requestSz_(0), minTimeTaken_(UINT64_MAX), maxTimeTaken_(0), reqCount_(0)
{
	pthread_create(&statCalculator_, nullptr, &Stats::StatsCalculator, this);
	pthread_detach(statCalculator_);
}

[[noreturn]] void* Stats::StatsCalculator(void* args)
{
	auto _this = (Stats*)args;
	static const timespec idleSleep = { .tv_sec = 0, .tv_nsec = 50 };
	while (true)
	{
		if (!stats_.is_empty())
		{
			auto node = stats_.dequeue();
			auto stat = node->stat_;
			delete node;

			_this->responseSz_ += stat->respLen_;
			_this->requestSz_ += stat->reqLen_;
			_this->timeTaken_ += stat->time_;

			if (_this->minTimeTaken_ > stat->time_)
				_this->minTimeTaken_ = stat->time_;

			if (_this->maxTimeTaken_ < stat->time_)
				_this->maxTimeTaken_ = stat->time_;

			++_this->reqCount_;

			delete stat;
		}
		else
		{
			nanosleep(&idleSleep, nullptr);
		}
	}
}
