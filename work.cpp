#include "stdafx.h"
#include "work.h"



iwork::~iwork() {}

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
