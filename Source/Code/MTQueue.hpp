#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <cppcoro/generator.hpp>
//#include <algorithm>

template<class T>
class MTQueue {

private:
	struct CommandNode {
		CommandNode* next;
		T command;
	};

	std::atomic<CommandNode*> head;

public:
	MTQueue() { head.store(nullptr); };
	~MTQueue() { clear(); };

	void add_command(T new_command) {
		CommandNode* new_node = new CommandNode{ head.load(), new_command };
		while (!head.compare_exchange_strong(new_node->next, new_node)) {
			new_node->next = head.load();
		}
	}
	cppcoro::generator<T> get_queue() {
		std::vector<T> out;
		CommandNode* current = head.exchange(nullptr);
		while (current != nullptr) {
			out.push_back(current->command);
			CommandNode* to_delete = current;
			current = current->next;
			delete to_delete;
		}
		for (int i = out.size() - 1; i >= 0; i--)
			co_yield out[i];
		out.clear();
	}

	void clear() {
		CommandNode* current = head.exchange(nullptr);
		while (current != nullptr) {
			CommandNode* to_delete = current;
			current = current->next;
			delete to_delete;
		}
	}
};