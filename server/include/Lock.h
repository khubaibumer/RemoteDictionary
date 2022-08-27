//
// Created by Khubaib.Umer on 8/25/2022.
//

#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <utility>
#include <pthread.h>

class SpinLock
{
public:
	SpinLock(SpinLock&) = default;
	SpinLock(SpinLock&&) = delete;

	~SpinLock()
	{
		pthread_spin_destroy(&lock_);
	}

	explicit SpinLock(std::string name)
		: name_(std::move(name)), exhaust_limit_(getExhaustLimit())
	{
		pthread_spin_init(&lock_, 0);
	}

	static uint64_t getExhaustLimit()
	{
		return std::thread::hardware_concurrency() * 100000;
	}

	__always_inline bool TryLock()
	{
		return (0 == pthread_spin_trylock(&lock_));
	}

	bool lock()
	{
		int count = 0;
		clock_t start = clock();
		while (((clock() - start) / CLOCKS_PER_SEC <= 10) || count < exhaust_limit_)
		{
			if (TryLock())
			{
				return true;
			}
			++count;
		}
		return false;
	}

	bool takeLock()
	{
		return (0 == pthread_spin_lock(&lock_));
	}

	void unlock()
	{
		pthread_spin_unlock(&lock_);
	}

	[[nodiscard]] const std::string& getName() const
	{
		return name_;
	}

private:
	pthread_spinlock_t lock_{};
	std::string name_;
	const uint64_t exhaust_limit_;
};

template<typename T>
class ScopedLock
{
public:
	ScopedLock(const char* fn, const int ln, T lock) : fn_(fn), ln_(ln), lock_(lock)
	{
	}

	~ScopedLock() = default;

private:
	const std::string fn_;
	const int ln_;
	T lock_;
};

template<>
class ScopedLock<SpinLock>
{
public:
	ScopedLock(const char* fn, const int ln, SpinLock& lock) : fn_(fn), ln_(ln), lock_(lock)
	{
		lock_.lock();
	}

	~ScopedLock()
	{
		lock_.unlock();
	}

private:
	const std::string fn_;
	const int ln_;
	SpinLock& lock_;
};

template<>
class ScopedLock<SpinLock*>
{
public:
	ScopedLock(const char* fn, const int ln, SpinLock* lock) : fn_(fn), ln_(ln), lock_(lock)
	{
		lock_->lock();
	}

	~ScopedLock()
	{
		lock_->unlock();
	}

private:
	const std::string fn_;
	const int ln_;
	SpinLock* lock_;
};

template<>
class ScopedLock<std::unique_ptr<SpinLock>>
{
public:
	ScopedLock(const char* fn, const int ln, const std::unique_ptr<SpinLock>& lock)
		: fn_(fn), ln_(ln), lock_(lock.get())
	{
		lock_->lock();
	}

	~ScopedLock()
	{
		lock_->unlock();
	}

private:
	const std::string fn_;
	const int ln_;
	SpinLock* lock_;
};

#define TAKE_LOCK(obj) ScopedLock<typeof(obj)> lock_obj_##obj(__FILE__, __LINE__, obj)

#define TAKE_CUSTOM_LOCK(obj, random) ScopedLock<typeof(obj)> lock_obj_##random(__FILE__, __LINE__, obj)
