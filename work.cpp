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
	//texturemesh* tm = new texturemesh(vertices, normals, uvs, string(fname.begin(), fname.end()));
	//tm->position = { 0.0, 0.0, 0.0 };
	//eng.sc->meshes.push_back(tm);

	//unlock
	eng.sc->mxqueuedmeshes.unlock();
}
