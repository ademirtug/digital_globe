#pragma once

float ytolat(float y);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3);
double N(double phi);
double* ecef2lla(double x, double y, double z);
glm::vec3 lla2ecef(double lat_indegrees, double lon_indegrees);


class quadtile
{
public:
	quadtile* children;
	string quadkey;

	quadtile();
	~quadtile();
	void init(string _quadkey = "");

	quadtile* getchild(char c);
	quadtile* gettile(string tile);
	void invalidate(string tile);
	BOOL FileExists(LPCTSTR szPath);
	void getmap();
};