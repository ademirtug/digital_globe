#include "stdafx.h"
#include "work.h"
#include "idlethread.h"
#include "threadpool.h"
#include "sphere.h"

#include "stdafx.h"



threadpool::threadpool() : maxallowed(5), running(1)
{
}
threadpool::~threadpool()
{
}

void threadpool::queue(shared_ptr<iwork> work)
{
	{
		lock_guard<std::mutex> lk_idle_threads(mtx_idle_threads);
		if (idle_threads.size() > 0)
		{
			shared_ptr<idlethread> idle_thread;
			idle_thread = idle_threads[0];
			idle_threads.erase(idle_threads.begin());

			idle_thread->attach(work);
			return;
		}
	}


	{
		lock_guard<std::mutex> lk_works(mtx_works);
		works.push_back(work);
	}

	lock_guard<std::mutex> lk_working_threads(mtx_working_threads);
	if (working_threads.size() < maxallowed)
	{
		shared_ptr< thread > th(new thread(&threadpool::handleworks, this));

		working_threads.insert(working_threads.begin(), th);
	}
}

void threadpool::handleworks()
{
	while (running)
	{
		shared_ptr<iwork> work;
		{
			lock_guard<std::mutex> lk_works(mtx_works);
			if (works.size() > 0)
			{
				work = works[0];
				works.erase(works.begin());
			}
		}


		if (work.get() == NULL)
		{
			shared_ptr<idlethread> idle_thread(new idlethread());

			{
				lock_guard<std::mutex> lk_idle_threads(mtx_idle_threads);
				idle_threads.insert(idle_threads.begin(), idle_thread);
			}

			work = idle_thread->WaitAWork(2 * 1000);
		}

		if (work.get() != NULL)
		{
			try
			{
				work->perform();
			}
			catch (exception&)
			{

			}
		}
		else
			break;
	}
}

void threadpool::stop(bool quickly = false, int nsecTimeout = 1000)
{
}
