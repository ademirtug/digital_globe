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

	eng.sc->mxqueuedmeshes.lock();

	eng.sc->queuedmeshes.push_back(__tile);

	//unlock
	eng.sc->mxqueuedmeshes.unlock();
}
