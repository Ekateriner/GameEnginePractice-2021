#include "JobSystem.h"
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/when_all.hpp>

JobSystem::JobSystem() :
	thread_pool() {}

JobSystem::JobSystem(size_t thread_count) :
	thread_pool(thread_count) {}

JobSystem::~JobSystem() {

};

cppcoro::task<> JobSystem::ScheduleTask(std::function<void()>& new_task) {
	co_await thread_pool.schedule();
	new_task();
}

//cppcoro::task<> JobSystem::ScheduleTasks(std::vector<cppcoro::task<>>& new_task) {
//	return cppcoro::schedule_on(thread_pool);
//	new_task();
//}

cppcoro::async_scope JobSystem::AddMultipleTasks(std::vector<std::function<cppcoro::task<>()>>& new_tasks) {
	std::vector<cppcoro::task<>> temp;
	for (auto& task : new_tasks) {
		temp.emplace_back(co_await cppcoro::schedule_on(thread_pool, task()));
	}
	cppcoro::async_scope scope;
	scope.spawn(temp);
	co_return scope;
}

cppcoro::async_scope JobSystem::AddTask(cppcoro::task<>& new_task) {
	cppcoro::async_scope scope;
	scope.spawn(co_await cppcoro::schedule_on(thread_pool, new_task));
	co_return scope;
}

void JobSystem::WaitTasks(cppcoro::async_scope& scope) {
	scope.join();
}
