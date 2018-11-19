#include "stdafx.h"
#include "sphere.h"
#include "threadpool.h"
#include <filesystem>

spheroid::spheroid(double _a, double _b)
{
	a = _a;
	b = _b;

	if (!std::filesystem::exists("c:\\mapdata\\"))
		std::filesystem::create_directory("c:\\mapdata\\");

	tiles.init("");
	for (size_t i = 0; i < 4; i++)
	{
		shared_ptr<tilerequest> tr(new tilerequest(&tiles.children[i]));
		pool.queue(tr);


		//tiles.children[i].init();
		//for (size_t x = 0; x < 4; x++)
		//{
		//	tiles.children[i].children[x].init();
		//	for (size_t y = 0; y < 4; y++)
		//	{
		//		shared_ptr<tilerequest> tr(new tilerequest(&tiles.children[i].children[x].children[y]));
		//		pool.queue(tr);
		//	}
		//}
	}
}