#include "stdafx.h"

#include "threadpool.h"

extern engine eng;

tilerequest::tilerequest()
{
}
tilerequest::tilerequest(quadtile* tile)
{
	__tile = tile;
}
tilerequest::~tilerequest()
{
}
void tilerequest::perform()
{
	__tile->getmap();
}
