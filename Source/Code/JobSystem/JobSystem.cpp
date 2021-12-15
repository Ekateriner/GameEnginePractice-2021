#include "JobSystem.h"
#include "../cppcoro/schedule_on.hpp"
#include "../cppcoro/when_all.hpp"

JobSystem::JobSystem() :
	thead_pool() {}

JobSystem::JobSystem(int thread_count) :
	thead_pool(thread_count) {}

JobSystem::~JobSystem() {

};

cppcoro::task<> JobSystem::AddTask(std::function<void()>& new_task) {
	co_await cppcoro::schedule_on(thead_pool, new_task());
}

//cppcoro::task<> JobSystem::AddTask(std::function<void()>& new_task) {
//	co_await thread_pool.schedule();
//	new_task();
//}

cppcoro::task<> JobSystem::AddMultipleTasks(std::vector<std::function<void()>>& new_tasks) {
	std::vector<cppcoro::task<>> tasks;
	for (auto & new_task : new_tasks) {
		tasks.emplace_back(new_task());
	}
	co_await cppcoro::schedule_on(thead_pool, tasks);

}

cppcoro::task<> JobSystem::WaitTasks(std::vector<cppcoro::task<>>& tasks) {
	co_return when_all(tasks);
}
