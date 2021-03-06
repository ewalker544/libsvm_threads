/*
 ** Copyright 2014 Edward Walker
 **
 ** Description: Thread Pool - this class maintains a persistent thread pool 
 ** @author: Ed Walker
 */
#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <queue>

class ThreadPool {
public:

	ThreadPool(size_t threads) : stop(false) 
	{
		// Spawn workers
		for (size_t i = 0; i < threads; ++i) {
			workers.push_back(std::thread(Worker(*this)));
		}
	}

	void enqueue(std::function<void()> f) 
	{
		{
			std::unique_lock<std::mutex> lock(queue_lock);

			// add task to queue
			tasks.push(f);
		}

		condition.notify_one(); // notify a worker
	}

	virtual ~ThreadPool() 
	{
		// stop the workers
		{
			std::unique_lock<std::mutex> lock(queue_lock);
			stop = true;
		}

		condition.notify_all(); // notify all workers

		// wait for all workers to exit
		for (size_t i = 0; i < workers.size(); ++i) {
			workers[i].join();
		}
	}

private:

	/**
	 * Thread pool workers 
	 */
	class Worker {
	public:
		Worker(ThreadPool &s) : pool(s) { }

		void operator()()
		{
   			std::function<void()> task;
			while(true)
			{
				{   
					std::unique_lock<std::mutex> lock(pool.queue_lock);
										                
					// waiting for work
					pool.condition.wait(lock, [this] { return pool.stop || !pool.tasks.empty(); } );
			
					if(pool.stop) { 
						return;
					}
			
					// get the task from the queue
					task = pool.tasks.front();
					pool.tasks.pop();
				}  
			
				// execute the task
				task();
			}
		}

	private:
		ThreadPool &pool;
	};

	friend class Worker;

	/** 
	 * Threads in the pool
	 */
	std::vector< std::thread > workers;

	/**
	 *  Task queue
	 */
	std::queue< std::function<void()> > tasks;

	/**
	 *  Task queue synchronization
	 */
	std::mutex queue_lock;
	std::condition_variable condition;

	/**
	 *  Stop criteria
	 */
	bool stop;
};

#endif
