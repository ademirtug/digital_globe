#include "stdafx.h"
#include "camera.h"

using namespace std;
extern engine eng;

class ellipsoid
{
public:
	float num_lat, num_long;
	glm::vec3 radii;


	ellipsoid(double _x, double _y, double _z)
	{
		radii.x = _x;
		radii.y = _y;
		radii.z = _z;
		float num_lat = 9 * 3;
		float num_long = 18 * 3;


	}
	ellipsoid(glm::vec3 xyz) : ellipsoid(xyz.x, xyz.y, xyz.z)
	{
	}
	vector<glm::vec3> generate_vertices()
	{
		glm::vec3** globe = new glm::vec3*[num_lat + 1];
		for (int i = 0; i < num_lat + 1; ++i)
			globe[i] = new glm::vec3[num_long + 1];


	}
};


glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3)
{
	return glm::cross(pt2 - pt1, pt3 - pt1);
}


float N(float phi, float a, float b)
{
	return (a*a) / sqrt((a*a)*cos(phi)*cos(phi) + (b*b)*sin(phi)*sin(phi));
}

int main()
{
	qball_camera *cam = new qball_camera();

	eng.cam = cam;
	eng.init(1024, 768);
	eng.maxfps = 25;
	//eng.sc->enable_dirlight = false;
	//cam->cdist = 7000000;



	float r = 6371000.0f; 
	float a = 6356752.3f;
	float b = 6378137.0f;
	float h = 0.0f;
	float num_lat = 9 * 3;
	float num_long = 18 * 3;

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

			float x = (N(lat, a, b) + h) * cos(lat) * cos(lon);

			float y = (N(lat, a, b) + h) * cos(lat) * sin(lon);

			float z = (((b*b) / (a*a)) * N(lon, a, b) + h) * sin(lat);

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
			glm::vec3 pt2 = globe[i+1][j];
			glm::vec3 pt3 = globe[i][j+1];

			vertices.push_back(pt1);
			vertices.push_back(pt2);
			vertices.push_back(pt3);

			normals.push_back(calc_normal(pt1, pt2, pt3));
			normals.push_back(calc_normal(pt2, pt3, pt1));
			normals.push_back(calc_normal(pt3, pt1, pt2));

			glm::vec3 pt4 = globe[i+1][j];
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
