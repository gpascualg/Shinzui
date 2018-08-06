#pragma once

#include <atomic>
#include <map>
#include <vector>
#include <chrono>
#include <thread>

#include <threadpool11/threadpool11.hpp>
#include <threadpool11/utility.hpp>
#include <threadpool11/worker.hpp>

#include <boost/lockfree/queue.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/pool.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>


using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;


class Database
{
	template <typename T>
	using Work = std::function<T(mongocxx::database&)>;
	using InternalWork = std::function<void(mongocxx::client&)>;

public:
	Database(mongocxx::uri uri, uint8_t numThreads);
    virtual ~Database();

	template <typename T>
	std::future<T> query(std::string dbname, Work<T> callable)
	{
		std::promise<T> promise;
		auto future = promise.get_future();

		/* TODO: how to avoid copy of callable into this lambda and the ones below? In a decent way... */
		/* evil move hack */
		auto move_callable = threadpool11::make_move_on_copy(std::move(callable));
		/* evil move hack */
		auto move_promise = threadpool11::make_move_on_copy(std::move(promise));
		
		auto workFunc = new InternalWork([move_callable, move_promise, &dbname, this](mongocxx::client& client) mutable {
			move_promise.value().set_value((move_callable.value())(client[dbname]));
		});

		bool result = _queue.push(workFunc);
		assert(result && "Database queue is full, consider using more threads or increasing queue size");

		std::unique_lock<std::mutex> work_signal_lock(_work_signal_mutex);
		_work_signal.notify_all(); // Whoever takes it

		return future;
	}

	inline void stop() { _stop = true; }

private:
	void run();

private:
    mongocxx::instance _instance;
	mongocxx::pool _pool;
	boost::lockfree::queue<InternalWork*> _queue;
	std::vector<std::thread> _databaseThreads;
	std::atomic<bool> _stop;

	mutable std::mutex _work_signal_mutex;
	std::condition_variable _work_signal;
};
