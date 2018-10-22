#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "quadtile.h"


class spheroid
{
	
public:
	int a, b;
	quadtile tiles;
	vector<shared_ptr<thread>> threads;

	spheroid(double _a, double _b);
};

#endif//__SPHERE_H__