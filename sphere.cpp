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
		return corners;

	double platenum = 32;
	double circumference = 2 * glm::pi<double>() * 6378137.0f;
	double mapsize = pow(2, quadkey.size()) * 256;

	double x1 = 0;
	double x2 = mapsize;
	double y1 = 0;
	double y2 = mapsize;


	//2 üzeri seviye kadar en boy parça
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

	double centerx = (x2 + x1) / 2;
	double centery = (y2 + y1) / 2;

	double lat = ytolat(circumference / mapsize * (mapsize / 2 - centery));
	double lon = (360.0 / mapsize * centerx) - 180;


	double test = ytolat(circumference / mapsize * (-256));

	float b = 6356752.3f;
	float a = 6378137.0f;
	float e2 = 1 - ((b*b) / (a*a));
	float e = sqrt(e2);

	double xstep = (x2 - x1) / platenum;
	double ystep = (y2 - y1) / platenum;


	for (size_t x = 0; x < platenum; x++)
	{
		for (size_t i = 0; i < platenum; i++)
		{
			double mercx1 = x1 + i * xstep;
			double mercx2 = x1 + (i + 1) * xstep;

			double mercy1 = y1 + x * ystep;
			double mercy2 = y1 + (x + 1) * ystep;


			//2d to lat long
			double lat1 = ytolat(circumference / mapsize * (mapsize / 2 - mercy1));
			double lon1 = (360.0 / mapsize * mercx1) - 180;

			glm::vec3 ecef = lla2ecef(lat1, lon1);
			glm::vec3 topleft = { ecef.x, ecef.y, ecef.z };


			double lat2 = ytolat(circumference / mapsize * (mapsize / 2 - mercy1));
			double lon2 = (360.0 / mapsize * mercx2) - 180;

			ecef = lla2ecef(lat2, lon2);
			glm::vec3 topright = { ecef.x, ecef.y, ecef.z };


			double lat3 = ytolat(circumference / mapsize * (mapsize / 2 - mercy2));
			double lon3 = (360.0 / mapsize * mercx1) - 180;


			ecef = lla2ecef(lat3, lon3);
			glm::vec3 bottomleft = { ecef.x, ecef.y, ecef.z };

			double lat4 = ytolat(circumference / mapsize * (mapsize / 2 - mercy2));
			double lon4 = (360.0 / mapsize * mercx2) - 180;


			ecef = lla2ecef(lat4, lon4);
			glm::vec3 bottomright = { ecef.x, ecef.y, ecef.z };


			glm::vec2 uvtopleft = { (i * xstep) / 256,  -(x * ystep) / 256 };
			glm::vec2 uvbottomleft = { (i * xstep) / 256,  -((x + 1) * ystep) / 256 };
			glm::vec2 uvtopright = { ((i + 1) * xstep) / 256, -(x * ystep) / 256 };


			glm::vec2 uvbottomright = { ((i + 1) * xstep) / 256, -((x + 1) * ystep) / 256 };


			if (x == 0 && i == 0)
			{
				corners.upperleft = calc_normal(topleft, bottomleft, topright);
			}
			if (x == 0 && i == (platenum - 1))
			{
				corners.bottomleft = calc_normal(bottomleft, topright, topleft);
			}
			if (x == (platenum - 1) && i == 0)
			{
				corners.upperright = calc_normal(topright, topleft, bottomleft);
			}
			if (x == (platenum - 1) && i == (platenum - 1))
			{
				corners.bottomright = calc_normal(bottomright, topright, bottomleft);
			}
		}
	}

	return corners;
}






//
//normalspack spheroid::getcornernormals(string quadkey)
//{
//	normalspack corners;
//	if (quadkey.size() < 1)
//		corners;
//
//	double circumference = 2 * glm::pi<double>() * 6378137.0f;
//	double mapsize = pow(2, quadkey.size()+1) * 256;
//
//	double x1 = 0;
//	double x2 = mapsize;
//	double y1 = 0;
//	double y2 = mapsize;
//
//	double xstep = (x2 - x1) / 32;
//	double ystep = (y2 - y1) / 32;
//
//	for (size_t i = 0; i < quadkey.size(); i++)
//	{
//		if (quadkey[i] == 'A')
//		{
//			y2 = (y2 + y1) / 2;
//			x2 = (x2 + x1) / 2;
//		}
//		else if (quadkey[i] == 'B')
//		{
//			x1 = (x2 + x1) / 2;
//			y2 = (y2 + y1) / 2;
//		}
//		else if (quadkey[i] == 'C')
//		{
//			x2 = (x2 + x1) / 2;
//			y1 = (y2 + y1) / 2;
//		}
//		else if (quadkey[i] == 'D')
//		{
//			x1 = (x2 + x1) / 2;
//			y1 = (y2 + y1) / 2;
//		}
//	}
//
//	//////////////////////////////////////
//	//latitude
//	//////////////////////////////////////
//	//topleft
//	double lat_topleft = ytolat(circumference / mapsize * (mapsize / 2 - y1));
//	double lat_topleftxp = lat_topleft;
//	double lat_topleftyp = ytolat(circumference / mapsize * (mapsize / 2 - y1 + ystep));
//
//
//	//topright
//	double lat_topright = lat_topleft;
//	double lat_toprightxm = lat_topleft;
//	double lat_toprightyp = lat_topleftyp;
//
//
//	//bottomleft
//	double lat_bottomleft = ytolat(circumference / mapsize * (mapsize / 2 - y2));
//	double lat_bottomleftym = ytolat(circumference / mapsize * (mapsize / 2 - y2 + ystep));
//	double lat_bottomleftxp = lat_bottomleft;
//
//
//	//bottomright
//	double lat_bottomright = lat_bottomleft;
//	double lat_bottomrightxm = lat_bottomleft;
//	double lat_bottomrightym = lat_bottomleftym;
//
//
//	/////////////////////////////////////////
//	//longitude
//	/////////////////////////////////////////
//	//topleft
//	double lon_topleft = (360.0 / mapsize * x1) - 180;
//	double lon_topleftxp = (360.0 / mapsize * (x1 + xstep)) - 180;
//	double lon_topleftyp = lon_topleft;
//
//	//topright
//	double lon_topright = (360.0 / mapsize * x2) - 180;
//	double lon_toprightxm = (360.0 / mapsize * (x2 - xstep)) - 180;
//	double lon_toprightyp = lon_topright;
//
//	//bottomleft
//	double lon_bottomleft = lon_topleft;
//	double lon_bottomleftxp = lon_topleftxp;
//	double lon_bottomleftym = lon_topleft;
//
//	//bottomright
//	double lon_bottomright = lon_topright;
//	double lon_bottomrightxm = 1;
//	double lon_bottomrightym = lon_topright;
//
//	corners.upperleft = calc_normal(
//		lla2ecef(lat_topleft, lon_topleft),
//		lla2ecef(lat_topleftyp, lon_topleftyp),
//		lla2ecef(lat_topleftxp, lat_topleftxp));
//
//	
//	corners.bottomleft = calc_normal(
//		lla2ecef(lat_bottomleft, lon_bottomleft),
//		lla2ecef(lat_bottomleftym, lon_bottomleftym),
//		lla2ecef(lat_bottomleftxp, lat_bottomleftxp));
//	
//	
//	corners.upperright = calc_normal(
//		lla2ecef(lat_topright, lon_topright),
//		lla2ecef(lat_toprightxm, lon_toprightxm),
//		lla2ecef(lat_toprightyp, lat_toprightyp));
//
//
//	corners.bottomright = calc_normal(
//		lla2ecef(lat_bottomright, lon_bottomright),
//		lla2ecef(lat_bottomrightym, lon_bottomrightym),
//		lla2ecef(lat_bottomrightxm, lat_bottomrightxm));
//
//	return corners;
//}