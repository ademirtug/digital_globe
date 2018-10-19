#ifndef __IDLETHREAD_H__
#define __IDLETHREAD_H__

class idlethread
{
private:

protected:
	shared_ptr<iwork> attached_work;

	mutex monitor;


	condition work_arrived;
public:

	void attach(shared_ptr<iwork> work)
	{
		attached_work = work;
	}

	shared_ptr<iwork> WaitAWork(int nsecTimeout = 1000)
	{

		if (attached_work.get() != NULL)
			return attached_work;

		mutex::scoped_lock lk(monitor);

		boost::xtime timeout;
		boost::xtime_get(&timeout, boost::TIME_UTC);
		timeout.nsec += nsecTimeout;

		work_arrived.timed_wait(lk, timeout);

		return attached_work;
	}

	idlethread()
	{

	}
	virtual ~idlethread();
};


#endif