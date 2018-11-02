#include "stdafx.h"
#include "threadpool.h"


threadpool::threadpool() : maxallowed(5), running(1)
{
}
threadpool::~threadpool()
{
	stop();
}

void threadpool::queue(shared_ptr<iwork> work)
{
	unique_lock<std::mutex> lk_dead(mtx_dthreads);
	deadthreads.erase(deadthreads.begin(), deadthreads.end());
	lk_dead.unlock();

	unique_lock ul_works(mtx_works);
	works.push_back(work);
	ul_works.unlock();


	unique_lock ul_threads(mtx_threads);
	if (threads.size() < maxallowed)
	{
		shared_ptr<safethread> st(new safethread(std::bind(&threadpool::handleworks, this)));
		threads.push_back(st);
	}
	ul_threads.unlock();
}

void threadpool::handleworks()
{
	while (running)
	{
		shared_ptr<iwork> work;
		unique_lock<std::mutex> lk_works(mtx_works);
		if (works.size() > 0)
		{
			work = works[0];
			works.erase(works.begin());
		}
		lk_works.unlock();

		if (work.get() != NULL)
		{
			try
			{
				work->perform();
			}
			catch (...)
			{
			}
		}
		else
		{
			shared_ptr<safethread> th;

			unique_lock<std::mutex> lk_threads(mtx_threads);
			for (size_t i = 0; i < threads.size(); i++)
			{
				if (threads[i]->th->get_id() == std::this_thread::get_id())
				{
					th = threads[i];
					threads.erase(threads.begin() + i);
				}
			}
			lk_threads.unlock();

			if (th != nullptr)
			{
				unique_lock<std::mutex> lk_dead(mtx_dthreads);
				deadthreads.push_back(th);
				lk_dead.unlock();
			}
			break;
		}
	}
}

void threadpool::stop()
{
	running = false;

	unique_lock<std::mutex> lk_threads(mtx_threads);
	threads.erase(threads.begin(), threads.end());
	lk_threads.unlock();


	unique_lock<std::mutex> lk_dead(mtx_dthreads);
	deadthreads.erase(deadthreads.begin(), deadthreads.end());
	lk_dead.unlock();

}
