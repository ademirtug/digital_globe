#ifndef __POOL_H__
#define __POOL_H__

#include "work.h"

class threadpool
{
protected:
	bool running;
	size_t maxallowed;


	vector< shared_ptr<iwork> > works;//queue
	vector< shared_ptr<thread> > working_threads;


	vector< shared_ptr<idlethread>> idle_threads;

	mutex mtx_working_threads;
	mutex mtx_idle_threads;
	mutex mtx_works;

	void handleworks();
public:

	void queue(shared_ptr<iwork> work);

	void stop(bool quickly, int nsecTimeout);

	threadpool();
	virtual ~threadpool();
};


#endif