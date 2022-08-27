//
// Created by Khubaib.Umer on 8/24/2022.
//

#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <unordered_map>
#include <queue>
#include <sys/epoll.h>
#include <nlohmann/json.hpp>
#include <list>
#include <mutex>
#include <semaphore.h>
#include "Client.h"
#include "../../Types.h"

class ThreadPool;

namespace Communication
{

	class Server
	{
	public:
		explicit Server(const ProgramOptions& ops);

		~Server();

		Server& operator=(const Server&) = delete;

		void StartServing();

	private:
		void Initialize();

		void StartServer();

		bool StartWorkerPool();

		static bool MakeNonBlockSocket(int fd);

		static void* RxThreadRoutine(void* args);

		int AcceptClient();

		void RemoveClient(ServerRequest* req);

		static std::string GetResponse(ServerRequest* req);

		size_t SendResponse(int fd, const std::string& response);

		static std::string ConsumeGetRequest(const nlohmann::json& req);

		static std::string ConsumeSetRequest(const nlohmann::json& req);

		static std::string ConsumeStatsRequest(const nlohmann::json& req);

		void CloseAllClients();

		static std::string ConsumeUpdateRequest(const nlohmann::json& req);

	private:
		friend ThreadPool;
		const int threadPoolSize_;
		const bool enableFilter_;
		int fd_{};
		pthread_t rxThread_{};
		std::string ip_;
		uint16_t port_{};
		std::atomic_bool isRunning_{};
		std::unique_ptr<SpinLock> clientLock_;
		std::unordered_map<int, std::shared_ptr<Client>> clientMap_;
		int efd_{};
		epoll_event serverEvent_{};
		epoll_event* clientEvents_{};
		std::unique_ptr<ThreadPool> threadPool_;
	};

} // Communication
