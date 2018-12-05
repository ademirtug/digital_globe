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
}

vector<quadtile*> spheroid::getdisplayedtiles(glm::vec3 cameraPos, int zoomlevel)
{
	vector<quadtile*> dt = tiles.calculatesubtiles(cameraPos, zoomlevel);

	//attach all loaded tiles
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
