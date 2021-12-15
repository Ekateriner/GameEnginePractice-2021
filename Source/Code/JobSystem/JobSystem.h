#pragma once

#include <queue>
#include <functional>
#include <vector>
#include "../cppcoro/task.hpp"
#include "../cppcoro/static_thread_pool.hpp"

class JobSystem {
public:
	JobSystem();
	~JobSystem();
	JobSystem(int thread_count); // create_threadpool
	
	cppcoro::task<> AddTask(std::function<void()>& new_task);
	cppcoro::task<> AddMultipleTasks(std::vector<std::function<void()>>& new_tasks);
	cppcoro::task<> WaitTasks(std::vector<cppcoro::task<>>& tasks);
	//cppcoro::task<void> AddTask(std::function<void()> new_task);
	/*void RunTasks(std::vector<cppcoro::task<void>> tasks);
	void RunTasks_withBarrier(std::vector<cppcoro::task<void>> tasks);*/

private:
	cppcoro::static_thread_pool thead_pool;
};