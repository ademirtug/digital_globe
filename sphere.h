#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "quadtile.h"
#include "threadpool.h"



class spheroid
{
	
public:
	int a, b;
	quadtile tiles;
	
	std::mutex mxpreparedtiles;
	std::vector<shared_ptr<quadtile>> preparedtiles;

	threadpool pool;
	spheroid(double _a, double _b);
	
	vector<quadtile*> getdisplayedtiles(glm::vec3 cameraPos, int zoomlevel);
	set<string> rtiles;
	
};

#endif//__SPHERE_H__