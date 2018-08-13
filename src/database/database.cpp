#include "database/database.hpp"


Database::Database(mongocxx::uri uri, uint8_t numThreads):
    _instance(),
	_pool(uri),
	_queue(256), // TODO: Configurable queue size,
	_stop(false)
{
    for (int i = 0; i < numThreads; ++i)
    {
	    _databaseThreads.emplace_back(std::thread([this] { run(); }));
    }
}

Database::~Database()
{
    _stop = true;
    for (auto&& thread : _databaseThreads)
    {
        thread.join();
    }
}

void Database::run()
{
    auto client = _pool.acquire();

    while (!_stop)
    {
        while (!_queue.empty())
        {
            InternalWork* work;
            if (_queue.pop(work))
            {
                (*work)(*client);
                delete work;
            }
        }

        std::unique_lock<std::mutex> work_signal_lock(_work_signal_mutex);
        _work_signal.wait(work_signal_lock);
    }
}
