#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "quadtile.h"
#include "threadpool.h"



class spheroid
{
	
public:
	int a, b;
	quadtile tiles;
	vector<shared_ptr<thread>> threads;
	threadpool pool;
	spheroid(double _a, double _b);
	
	normalspack getcornernormals(string quadkey);
	vector<quadtile*> getdisplayedtitles(glm::vec3 camerapos, int zoomlevel);
	
};

#endif//__SPHERE_H__