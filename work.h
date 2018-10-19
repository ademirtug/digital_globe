#pragma once

#include "quadtile.h"

class iwork
{
public:

	virtual void perform() = 0;
	virtual ~iwork();
};


class tilerequest : public iwork
{
	quadtile* __tile;
public:
	tilerequest();
	tilerequest(quadtile* tile);
	~tilerequest();
	virtual void perform();
};