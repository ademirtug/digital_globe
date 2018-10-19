#ifndef __SPHERE_H__
#define __SPHERE_H__


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


class spheroid
{
public:
	int a, b;
	quadtile tiles;

	spheroid(double _a, double _b)
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
					tiles.children[i].children[x].children[y].getmap();
				}
			}
		}
	}
};

#endif//__SPHERE_H__