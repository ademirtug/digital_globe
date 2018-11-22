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

normalspack spheroid::getcornernormals(string quadkey)
{
	normalspack corners;
	if (quadkey.size() < 1)
		corners;

	double circumference = 2 * glm::pi<double>() * 6378137.0f;
	double mapsize = pow(2, quadkey.size()+1) * 256;

	double x1 = 0;
	double x2 = mapsize;
	double y1 = 0;
	double y2 = mapsize;

	for (size_t i = 0; i < quadkey.size(); i++)
	{
		if (quadkey[i] == 'A')
		{
			y2 = (y2 + y1) / 2;
			x2 = (x2 + x1) / 2;
		}
		else if (quadkey[i] == 'B')
		{
			x1 = (x2 + x1) / 2;
			y2 = (y2 + y1) / 2;
		}
		else if (quadkey[i] == 'C')
		{
			x2 = (x2 + x1) / 2;
			y1 = (y2 + y1) / 2;
		}
		else if (quadkey[i] == 'D')
		{
			x1 = (x2 + x1) / 2;
			y1 = (y2 + y1) / 2;
		}
	}

	double lat_topleft = ytolat(circumference / mapsize  * (mapsize / 2 - y1) );
	double lat_topright = lat_topleft;

	double lat_bottomleft = ytolat(circumference / mapsize * (mapsize / 2 - y2));;
	double lat_bottomright = lat_bottomleft;

	double lon_topleft = (360.0 / mapsize * x1) - 180;
	double lon_topright = (360.0 / mapsize * x2) - 180;

	double lon_bottomleft = lon_topleft;
	double lon_bottomright = lon_topright;

	glm::vec3 ecef_topleft = lla2ecef(lat_topleft, lon_topleft);
	glm::vec3 ecef_topright = lla2ecef(lat_topright, lon_topright);
	glm::vec3 ecef_bottomleft = lla2ecef(lat_bottomleft, lon_bottomleft);
	glm::vec3 ecef_bottomright = lla2ecef(lat_bottomright, lon_bottomright);


	
	corners.upperleft = calc_normal(ecef_topleft, ecef_bottomleft, ecef_topright);
	corners.bottomleft = calc_normal(ecef_bottomleft, ecef_topright, ecef_topleft);
	corners.upperright = calc_normal(ecef_topright, ecef_topleft, ecef_bottomleft);
	corners.bottomright = calc_normal(ecef_bottomright, ecef_topright, ecef_bottomleft);

	return corners;
}