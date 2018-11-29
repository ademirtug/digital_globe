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

	//tiles.init("");
	//for (size_t i = 0; i < 4; i++)
	//{
	//	shared_ptr<tilerequest> tr(new tilerequest(&tiles.children[i]));
	//	pool.queue(tr);
	//}
}

vector<quadtile*> spheroid::getdisplayedtiles(glm::vec3 cameraPos, int zoomlevel)
{
	vector<quadtile*> dt = tiles.calculatesubtiles(cameraPos, zoomlevel);

	//attach all loaded tile
	unique_lock<std::mutex> lk(mxpreparedtiles);
	for (auto pt : preparedtiles)
	{
		quadtile* t = tiles.gettile(pt->quadkey);
		if (t == nullptr)
			continue;

		t->normals = pt->normals;
		t->uvs = pt->uvs;
		t->vertices = pt->vertices;
		string fname(pt->fname.begin(), pt->fname.end());
		t->tm.reset( new texturemesh(t->vertices, t->normals, t->uvs, fname) );
	}
	preparedtiles.clear();


	return dt;
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