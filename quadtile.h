#ifndef __QUADTILE_H__
#define __QUADTILE_H__


#include <array>

class spheroid;

float ytolat(float y);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3);
double N(double phi);
glm::vec3 lla2ecef(double lat_indegrees, double lon_indegrees);
std::array<double, 3> ecef_to_geo(std::array<double, 3> ecef);


class quadtile
{
public:
	quadtile* children;
	string quadkey;
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> uvs;
	vector<glm::vec2> borderuvs;
	
	//only accessed from MT
	bool requested = false;

	shared_ptr<texturemesh> tm;
	wstring fname;
	int platebase;
	double lat_center = 0;
	double lon_center = 0;


	quadtile();
	~quadtile();
	void initchildren(string _quadkey = "");

	quadtile* getchild(char c);
	quadtile* gettile(string tile);
	void germinate(string tile);

	void prunetree(string root, set<string>& sprouts);
	vector<quadtile*> calculatesubtiles(glm::vec3 cameraPos, int zoomlevel);
	void invalidate(string tile);
	void buildplates();

};


#endif//__QUADTILE_H__
