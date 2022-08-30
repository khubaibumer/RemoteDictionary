//
// Created by Khubaib.Umer on 8/30/2022.
//
#pragma once

#include <cstddef>
#include <chrono>
#include <string>
#include "Thread.h"

namespace Communication
{
	struct ServerRequest
	{
		ServerRequest(int cfd, char* req, size_t reqLen)
			: cfd_(cfd), request_(req), requestLen_(reqLen), responseLen_(0)
		{
			start_ = std::chrono::high_resolution_clock::now();
		}

		~ServerRequest()
		{
			end_ = std::chrono::high_resolution_clock::now();
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
//			currentThread->reportTxn(responseLen_, requestLen_, microseconds);
		}

		[[nodiscard]] constexpr int getFd() const
		{
			return cfd_;
		}

		[[nodiscard]] std::string str() const
		{
			return request_ + " received from Client Id: " + std::to_string(cfd_);
		}

		[[nodiscard]] const std::string& getRequest() const
		{
			return request_;
		}

		void SetResponseLen(size_t len)
		{
			responseLen_ = len;
		}

	private:
		std::string request_;
		size_t requestLen_{};
		size_t responseLen_{};
		std::chrono::high_resolution_clock::time_point start_;
		std::chrono::high_resolution_clock::time_point end_;
		int cfd_;
	};

}
