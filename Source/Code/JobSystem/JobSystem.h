#pragma once

#include <queue>
#include <functional>
#include <vector>
#include <cppcoro/task.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/async_scope.hpp>
//#include "../cppcoro/static_thread_pool.hpp"

class JobSystem {
public:
	JobSystem();
	JobSystem(size_t thread_count); // create_threadpool
	~JobSystem();
	
	cppcoro::task<> ScheduleTask(std::function<void()>& new_task);
	
	cppcoro::async_scope AddMultipleTasks(std::vector<std::function<cppcoro::task<>()>>& new_tasks);
	cppcoro::async_scope AddTask(cppcoro::task<>& new_task);
	void WaitTasks(cppcoro::async_scope& scope);
	//cppcoro::task<void> AddTask(std::function<void()> new_task);
	/*void RunTasks(std::vector<cppcoro::task<void>> tasks);
	void RunTasks_withBarrier(std::vector<cppcoro::task<void>> tasks);*/

private:
	cppcoro::static_thread_pool thread_pool;
};