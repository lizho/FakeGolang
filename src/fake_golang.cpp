#include "fake_golang.h"

using namespace std;

bool goroutines_vm::run = true;
size_t goroutines_vm::auto_id = 0;
goroutines_vm::go_threads goroutines_vm::threads;
std::mutex goroutines_vm::threads_mutex;
std::queue<size_t> goroutines_vm::completed_threads;
std::mutex goroutines_vm::completed_threads_mutex;
std::thread goroutines_vm::gc_thread{ goroutines_vm::gc };
const std::thread::id goroutines_vm::emplty_id;

void goroutines_vm::quit()
{
	run = false;
	if (gc_thread.joinable())
	{
		gc_thread.join();
	}
}

void goroutines_vm::gc()
{
	while (run)
	{
		if (auto p = pop(); p.has_value())
		{
			GO_LOCK(threads_mutex);
			if (auto iter = threads.find(p.value()); iter != threads.end())
			{
				iter->second.join();
				threads.erase(iter);
			}
		}
		std::this_thread::sleep_for(10ms);
	}
}

std::optional<size_t> goroutines_vm::pop()
{
	GO_LOCK(completed_threads_mutex);
	if (!completed_threads.empty())
	{
		auto res = completed_threads.front();
		completed_threads.pop();
		return res;
	}
	return {};
}

std::tuple<std::thread&, size_t> goroutines_vm::new_thread()
{
	GO_LOCK(threads_mutex);
	auto id = auto_id++;
	return { threads[id], id };
}

void goroutines_vm::complete(size_t id)
{
	GO_LOCK(completed_threads_mutex);
	completed_threads.emplace(id);
}

defer_impl::~defer_impl()
{
	if (operator bool())
		operator()();
}