#include "stdafx.h"
#include "camera.h"
#include <set>


using namespace std;
extern engine eng;

string TileXYToQuadKey(int tileX, int tileY, int levelOfDetail)
{
	string quadkey;
	for (int i = levelOfDetail; i > 0; i--)
	{
		char digit = '0';
		int mask = 1 << (i - 1);
		if ((tileX & mask) != 0)
		{
			digit++;
		}
		if ((tileY & mask) != 0)
		{
			digit++;
			digit++;
		}
		quadkey.push_back(digit);
	}
	return quadkey;
}
vector<int> QuadKeyToTileXY(string quadKey)
{
	int tileX = 0;
	int tileY = 0;
	int levelOfDetail = quadKey.size();


	for (int i = levelOfDetail; i > 0; i--)
	{
		int mask = 1 << (i - 1);
		switch (quadKey[levelOfDetail - i])
		{
		case '0':
			break;

		case '1':
			tileX |= mask;
			break;

		case '2':
			tileY |= mask;
			break;

		case '3':
			tileX |= mask;
			tileY |= mask;
			break;

		default:
			break;
		}
	}

	return { tileX, tileY, levelOfDetail };
}


//double merc_lat(double y) {
//
//	return rad_deg(phi);
//}

float merc_lat(double y) {
	float b = 6356752.3f;
	float a = 6378137.0f;
	float ecc = sqrt(1.0 - ((b / a) * (b / a)));
	float halfecc = ecc * 0.5;
	float ts = exp(-y / b);
	float phi = glm::pi<float>() / 2 - 2 * atan(ts);
	float dphi = 1.0;


	int i;
	for (i = 0; fabs(dphi) > 0.000000001 && i < 15; i++) {
		float con = ecc * sin(phi);
		dphi = glm::pi<float>() / 2 - 2 * atan(ts * pow((1.0 - con) / (1.0 + con), halfecc)) - phi;
		phi += dphi;
	}
	return phi * glm::pi<float>() / 180.0;
}


class tile
{
public:
	string quadkey;
	bmp data;

	vector<glm::vec3> vertices;
	void generate_vertices()
	{
		int level = quadkey.size();
		int size = pow(2, level);
		vector<int> xy = QuadKeyToTileXY(quadkey);
		int x = xy[0];
		int y = xy[1];

		float lon = (360 / size) * x;
		float nextlon = (360 / size) * (x + 1);
		float lat = 0 ;













		float nextlat = 0;

	}
};


class spheroid
{
public:
	int NUMLAT = 180;
	int NUMLONG = 360;

	int a, b;
	std::map<std::string, tile> tiles;

	spheroid(double _a, double _b)
	{
		a = _a;
		b = _b;
		
		for (size_t i = 0; i < 4; i++)
		{
			tile t;
			t.quadkey = to_string(i);
			tiles[t.quadkey] = t;

			//for (size_t x = 0; x < 4; x++)
			//{
			//	tile tx;
			//	tx.quadkey = to_string(i) + to_string(x);
			//	tiles[tx.quadkey] = tx;
			//}
		}

		//for (size_t i = 0; i < num_lat + 1; i++)
		//{
		//	float lat = ((glm::pi<float>() / num_lat)*i) + glm::pi<float>() / 2;
		//	for (size_t j = 0; j<num_long + 1; j++)
		//	{
		//		float lon = ((2 * glm::pi<float>() / num_long)*j);
		//		float x = N(lat, a, b) * cos(lat) * cos(lon);
		//		float y = N(lat, a, b) * cos(lat) * sin(lon);
		//		float z = (1 - e2) * N(lat, a, b) * sin(lat);
		//		glm::vec3 pt = { x, y, z };
		//		globe[i][j] = pt;
		//		pc.addpoint(pt);
		//	}
		//}
		//for (size_t i = 0; i < num_lat; i++)
		//{
		//	for (size_t j = 0; j< num_long; j++)
		//	{
		//		glm::vec3 pt1 = globe[i][j];
		//		glm::vec3 pt2 = globe[i + 1][j];
		//		glm::vec3 pt3 = globe[i][j + 1];
		//		vertices.push_back(pt1);
		//		vertices.push_back(pt2);
		//		vertices.push_back(pt3);
		//		normals.push_back(calc_normal(pt1, pt2, pt3));
		//		normals.push_back(calc_normal(pt2, pt3, pt1));
		//		normals.push_back(calc_normal(pt3, pt1, pt2));
		//		glm::vec3 pt4 = globe[i + 1][j];
		//		glm::vec3 pt5 = globe[i + 1][j + 1];
		//		glm::vec3 pt6 = globe[i][j + 1];
		//		vertices.push_back(pt4);
		//		vertices.push_back(pt5);
		//		vertices.push_back(pt6);
		//		normals.push_back(calc_normal(pt4, pt5, pt6));
		//		normals.push_back(calc_normal(pt5, pt6, pt4));
		//		normals.push_back(calc_normal(pt6, pt4, pt5));
		//	}
		//}
	}


	void generate()
	{
	}
};









glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3)
{
	return glm::cross(pt2 - pt1, pt3 - pt1);
}


vector<unsigned char> getpngdata()
{
	vector<unsigned char> data;
	unsigned long w, h;
	unsigned char* rawdata;
	size_t size = 0;



	return data;
}



float N(float phi, float a, float b)
{
	double e2 = 1 - ((b*b) / (a*a));
	return a / sqrt(1 - e2 * sin(phi)*sin(phi));
}


void ecef2lla(double x, double y, double z) {

	double f = 0.0034;
	double b = 6356752.3f;
	double a = 6378137.0f;
	double e2 = sqrt((a*a - b*b) / b*b);
	double lla[] = { 0, 0, 0 };
	double lat, lon, height, N, theta, p;


	p = sqrt(x*x + y*y);

	theta = atan((z * a) / (p * b));

	lon = atan(y / x);

	lat = atan(((z + e2*e2 * b * pow(sin(theta), 3)) / ((p - pow(e2, 2) * a * pow(cos(theta), 3)))));
	N = a / (sqrt(1 - (pow(e2, 2) * pow(sin(lat), 2))));

	double m = (p / cos(lat));
	height = m - N;


	lon = lon * 180 / glm::pi<double>();
	lat = lat * 180 / glm::pi<double>();
	lla[0] = lat;
	lla[1] = lon;
	lla[2] = height;
}

void lla2ecef()
{
	double b = 6356752.3f;
	double a = 6378137.0f;
	double num_lat = 180;
	double num_long = 360;
	double e2 = 1 - ((b*b) / (a*a));
	double lat = ((glm::pi<double>() / num_lat) * 50);
	double lon = ((2 * glm::pi<double>() / num_long) * 50);

	double x = N(lat, a, b) * cos(lat) * cos(lon);
	double y = N(lat, a, b) * cos(lat) * sin(lon);
	double z = ((b*b) / (a*a)) * N(lat, a, b) * sin(lat);

	ecef2lla(x, y, z);
}

int main()
{
	spheroid earth(6378137.0f, 6356752.3f);
	getpngdata();

	qball_camera *cam = new qball_camera();

	eng.cam = cam;
	eng.init(1024, 768);
	eng.maxfps = 25;

	float r = 6371000.0f; 
	float b = 6356752.3f;
	float a = 6378137.0f;
	float num_lat = 180;
	float num_long = 360;
	float e2 = 1 - ((b*b) / (a*a));

	glm::vec3** globe = new glm::vec3*[num_lat + 1];
	for (int i = 0; i < num_lat + 1; ++i)
		globe[i] = new glm::vec3[num_long + 1];


	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;

	pointcloud pc;

	for (size_t i = 0; i < num_lat + 1; i++)
	{
		float lat = ((glm::pi<float>() / num_lat)*i) + glm::pi<float>() / 2;
		for (size_t j = 0; j<num_long + 1; j++)
		{
			float lon = ((2 * glm::pi<float>() / num_long)*j);

			float x = N(lat, a, b) * cos(lat) * cos(lon);
			float y = N(lat, a, b) * cos(lat) * sin(lon);
			float z = (1 - e2) * N(lat, a, b) * sin(lat);

			glm::vec3 pt = { x, y, z };
			globe[i][j] = pt;
			pc.addpoint(pt);
		}
	}


	for (size_t i = 0; i < num_lat; i++)
	{
		for (size_t j = 0; j< num_long; j++)
		{
			glm::vec3 pt1 = globe[i][j];
			glm::vec3 pt2 = globe[i + 1][j];
			glm::vec3 pt3 = globe[i][j + 1];

			vertices.push_back(pt1);
			vertices.push_back(pt2);
			vertices.push_back(pt3);

			normals.push_back(calc_normal(pt1, pt2, pt3));
			normals.push_back(calc_normal(pt2, pt3, pt1));
			normals.push_back(calc_normal(pt3, pt1, pt2));

			glm::vec3 pt4 = globe[i + 1][j];
			glm::vec3 pt5 = globe[i + 1][j + 1];
			glm::vec3 pt6 = globe[i][j + 1];

			vertices.push_back(pt4);
			vertices.push_back(pt5);
			vertices.push_back(pt6);

			normals.push_back(calc_normal(pt4, pt5, pt6));
			normals.push_back(calc_normal(pt5, pt6, pt4));
			normals.push_back(calc_normal(pt6, pt4, pt5));
		}
	}

	colormesh cm(vertices, normals);
	cm.position = { 0.0, 0.0, 0.0 };
	eng.sc->meshes.push_back(&cm);

	//pc.init();
	//pc.position = { 0.0, 0.0, 0.0 };
	//eng.sc->meshes.push_back(&pc);

	eng.sc->generate_shaders();
	eng.run();

	glfwTerminate();
	return 0;
}

//////sphere
////for (size_t i = 0; i < lines +1; i++)
////{
////	float lon = ((2 * glm::pi<float>() / lines)*i); 
////	for (size_t j = 0; j<lines +1 ; j++)
////	{
////		float lat = ((2 * glm::half_pi<float>() / lines)*j);
////		float x = r * sin(lat) * cos(lon);
////		float y = r * sin(lon) * sin(lat);
////		float z = r * cos(lat);
////		glm::vec3 pt = {x, y, z};
////		globe[j][i] = pt;
////		pc.addpoint(pt);	
////	}
////}
////for (size_t i = 0; i < lines; i++)
////{
////	for (size_t j = 0; j< lines;j++)
////	{
////		glm::vec3 pt1 = globe[i + 1][j];
////		glm::vec3 pt2 = globe[i][j+1];
////		glm::vec3 pt3 = globe[i][j];
////		vertices.push_back(pt1);
////		vertices.push_back(pt2);
////		vertices.push_back(pt3);
////		normals.push_back(calc_normal(pt1, pt2, pt3));
////		normals.push_back(calc_normal(pt2, pt3, pt1));
////		normals.push_back(calc_normal(pt3, pt1, pt2));
////		glm::vec3 pt4 = globe[i][j + 1];
////		glm::vec3 pt5 = globe[i+1][j];
////		glm::vec3 pt6 = globe[i + 1][j + 1];
////		vertices.push_back(pt4);
////		vertices.push_back(pt5);
////		vertices.push_back(pt6);
////		normals.push_back(calc_normal(pt4, pt5, pt6));
////		normals.push_back(calc_normal(pt5, pt6, pt4));
////		normals.push_back(calc_normal(pt6, pt4, pt5));
////	}
////}