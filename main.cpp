#include "stdafx.h"
#include "camera.h"
#include "sphere.h"
#include <thread>
#include <mutex>
#include <queue>

using namespace std;
extern engine eng;



class iwork
{
public:
	virtual void dojob()
	{

	}

};




class threadpool
{
public:
	int maxthreads = 5;
	mutex mxthreads;
	mutex mxjobs;
	bool halt = false;

	vector<shared_ptr<std::thread>> threads;
	queue<shared_ptr<iwork>> jobs;


	threadpool() {}
	~threadpool() {}


	void enqueue(shared_ptr<iwork> work)
	{
		mxjobs.lock();
		jobs.push(work);
		mxjobs.unlock();

		mxthreads.lock();
		if (threads.size() < maxthreads)
		{
			shared_ptr<std::thread> t(new std::thread(&threadpool::run, this));
			threads.push_back(t);
		}
		mxthreads.unlock();
	}

	void run()
	{
		while (!halt)
		{
			mxjobs.lock();
			shared_ptr<iwork> work = jobs.front();
			jobs.pop();
			mxjobs.unlock();


			work->dojob();
		}
	}


};

int main()
{

	

	qball_camera *cam = new qball_camera();

	eng.cam = cam;
	eng.init(1024, 768);
	eng.maxfps = 25;

	spheroid earth(6378137.0f, 6356752.3f);

	eng.sc->generate_shaders();
	eng.run();

	glfwTerminate();
	return 0;
}