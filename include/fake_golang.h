#pragma once
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <type_traits>

#define _GO_COMBINE_IMPL_(a, b) a##b
#define _GO_COMBINE_(a, b) _GO_COMBINE_IMPL_(a, b)
#define _GO_DECLARE_(_Type, _Name_Temp) [[maybe_unused]] _Type _GO_COMBINE_(_Name_Temp, __LINE__)
#define _GO_AUTO_DEC_(_Name) _GO_DECLARE_(_Name##_impl, __temp__##_Name##__) =
#define defer _GO_AUTO_DEC_(defer)
#define go _GO_AUTO_DEC_(go)

#define lets_go defer goroutines_vm::quit
#define GO_LOCK(_Mutex) _GO_DECLARE_(std::lock_guard<std::mutex>, __lock__){_Mutex}

using go_callable = std::function<void()>;

class defer_impl : public go_callable
{
public:
	using go_callable::go_callable;

	defer_impl(const defer_impl&) = delete;
	defer_impl(defer_impl&&) = default;
	defer_impl& operator=(const defer_impl&) = delete;
	defer_impl& operator=(defer_impl&&) = default;
	virtual ~defer_impl();
};

class goroutines_vm
{
public:
	static void quit();
private:
	using go_thread_ptr = std::unique_ptr<std::thread>;
	using go_threads = std::map<std::size_t, std::thread>;
	static bool run;
	static size_t auto_id;
	static go_threads threads;
	static std::mutex threads_mutex;
	static std::queue<size_t> completed_threads;
	static std::mutex completed_threads_mutex;
	static std::thread gc_thread;
	static const std::thread::id emplty_id;
	friend class go_impl;

	static void gc();
	static std::optional<size_t> pop();
	static std::tuple<std::thread&, size_t> new_thread();
	static void complete(size_t id);
};

class go_impl
{
public:
	template<class Fx, std::enable_if_t<std::conjunction_v<std::negation<std::is_same<std::decay_t<Fx>, go_callable>>, std::is_invocable_r<void, Fx>>, std::nullptr_t> = nullptr>
	go_impl(Fx&& fx)
	{
		auto [pt, id] = goroutines_vm::new_thread();
		if (pt.get_id() != goroutines_vm::emplty_id)
		{
			return;
		}
		pt = std::thread{ [id, &fx] {
			fx();
			goroutines_vm::complete(id);
			}
		};
	}

	go_impl(const go_impl&) = delete;
	go_impl(go_impl&&) = delete;
	go_impl& operator=(const go_impl&) = delete;
	go_impl& operator=(go_impl&&) = delete;
};

