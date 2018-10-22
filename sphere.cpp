#include "stdafx.h"
#include "sphere.h"
#include "work.h"

spheroid::spheroid(double _a, double _b)
{
	a = _a;
	b = _b;
	tiles.init("");
	for (size_t i = 0; i < 4; i++)
	{
		tiles.children[i].init();
		for (size_t x = 0; x < 4; x++)
		{
			tiles.children[i].children[x].init();
			for (size_t y = 0; y < 4; y++)
			{
				shared_ptr<tilerequest> tr(new tilerequest(&tiles.children[i].children[x].children[y]));
				//pool.queue(tr);
				shared_ptr<thread> t(new thread(&tilerequest::perform, tr));
				t->detach();
				threads.push_back(t);
				


				//std::thread t(&quadtile::getmap, tiles.children[i].children[x].children[y]);
				//t.join();
				//std::thread t(&tiles.children[i].children[x].children[y].getmap(), this);
			}
		}
	}
}