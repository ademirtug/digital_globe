#ifndef __IDLETHREAD_H__
#define __IDLETHREAD_H__

#include <mutex>

class idlethread
{
private:

protected:
	shared_ptr<iwork> attached_work;

	mutex monitor;


	condition_variable work_arrived;
public:

	void attach(shared_ptr<iwork> work)
	{
		attached_work = work;
	}

	shared_ptr<iwork> WaitAWork(int nsecTimeout = 1000)
	{

		if (attached_work.get() != NULL)
			return attached_work;

		unique_lock<std::mutex> lk(monitor);

		work_arrived.wait_for(lk, std::chrono::milliseconds(nsecTimeout));



		return attached_work;
	}

	idlethread()
	{

	}
	virtual ~idlethread() {};
};


#endif