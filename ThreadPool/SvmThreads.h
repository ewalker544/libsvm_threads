/*
 ** Copyright 2014 Edward Walker
 **
 ** Description: Interface to thread pool - used mainly for running parallel tasks
 ** @author: Ed Walker
 */
#ifndef _SVM_THREADS_H_
#define _SVM_THREADS_H_

#include "ThreadPool.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <utility>
#include <atomic>
#include <vector>
#include <iostream>

class SvmThreads 
{
	public:

		SvmThreads() {
			total_num_cpus = std::thread::hardware_concurrency();
			thrd_pool = new ThreadPool(total_num_cpus);
		}

		virtual ~SvmThreads() 
		{
			delete thrd_pool;
		}

		/**
		 * Given N iterations, returns the start iteration for thread ID tid.
		 */
		int start(int tid, int N) const
		{
			int C = total_num_cpus;
			if (N >= C) {
				return (tid * (N / C));
			} else {
				return (tid < N ? tid : 0);
			}
		}

		/**
		 * Given N iterations, returns the end iteration for thread ID tid
		 */
    	int end(int tid, int N) const
	    {
			int C = total_num_cpus;
			if (N >= C) {
				return ((tid == C-1) ? N : ((tid + 1) * (N / C)));
			} else {
				return (tid < N ? (tid + 1) : 0);
			}
		}

		/**
		 * Runs function on each thread in the pool.  Returns when all task completes.
		 */
		void run_workers(std::function<void(int)> func)
		{
			std::mutex cv_m;
			std::condition_variable cv;

			int run_count = total_num_cpus;

			for (int i = 0; i < total_num_cpus; ++i) {
				thrd_pool->enqueue([i, func, &cv, &cv_m, &run_count] {
						func(i);
						{
							std::unique_lock<std::mutex> grd(cv_m);
							if (--run_count == 0)
								cv.notify_one (); // wake the main thread
						}
					});
			}

			{
				std::unique_lock<std::mutex> grd(cv_m);
				cv.wait(grd, [&run_count] {return run_count == 0;}); // wait for notify
			}

			return;
		}

		/**
		 * Runs an ensemble of tasks in the thread pool
		 */
		template <typename T>
		void run_workers(std::vector<std::tuple<std::function<void(T)>, T>> &work_items)
		{
			std::mutex cv_m;
			std::condition_variable cv;

			std::size_t run_count = work_items.size();

			for (auto & ele : work_items) {

				auto func = std::get<0>(ele);
				auto arg = std::get<1>(ele);

				thrd_pool->enqueue([func, arg, &cv, &cv_m, &run_count] {
						func(arg);
						{
							std::unique_lock<std::mutex> grd(cv_m);
							if (--run_count == 0)
								cv.notify_one (); // wake the main thread
						}
					});
			}

			{
				std::unique_lock<std::mutex> grd(cv_m);
				cv.wait(grd, [&run_count] {return run_count == 0;}); // wait for notify
			}

			return;
		}

		/**
		 * Returns an instance of this class
		 */
		static SvmThreads * getInstance() {
			/**
			 * §6.7 [stmt.dcl] p4
			 *
			 * If control enters the declaration concurrently while the variable is being initialized, 
			 * the concurrent execution shall wait for completion of the initialization.
			 */
			static SvmThreads * s_instance = new SvmThreads();
			return s_instance;
		}

	private:

		/**
		 * Total number of CPUs on this computer
		 */
		int total_num_cpus;

    	ThreadPool *thrd_pool;
};

#endif
