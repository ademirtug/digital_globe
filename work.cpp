#include "stdafx.h"
#include "work.h"

tilerequest::tilerequest()
{
}
tilerequest::tilerequest(quadtile* tile)
{
	__tile = tile;
}
void tilerequest::dojob()
{
	__tile->getmap();
}
