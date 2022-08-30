//
// Created by Khubaib.Umer on 8/30/2022.
//
#pragma once

#include <cstddef>
#include <chrono>
#include <string>
#include "Thread.h"
#include "Stats.h"

namespace Communication
{
	struct ServerRequest
	{
		ServerRequest(int cfd, char* req, size_t reqLen)
			: cfd_(cfd), request_(req), requestLen_(reqLen)
		{
			start_ = std::chrono::high_resolution_clock::now();
		}

		~ServerRequest()
		{
			end_ = std::chrono::high_resolution_clock::now();
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
			Stats::ReportTxn(new TxnData(requestLen_, responseLen_, microseconds));
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

		void SetResponseSize(size_t sz)
		{
			responseLen_ = sz;
		}

	private:
		const int cfd_;
		const std::string request_;
		const size_t requestLen_{};
		size_t responseLen_{};
		std::chrono::high_resolution_clock::time_point start_;
		std::chrono::high_resolution_clock::time_point end_;
	};

}
