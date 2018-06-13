// 3d_engine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "camera.h"

using namespace std;
extern engine eng;

int main()
{
	qball_camera *cam = new qball_camera();

	eng.cam = cam;
	eng.init(1024, 768);
	eng.maxfps =750;
	//eng.sc->enable_dirlight = false;


	int lines = 50;
	float r = 3.0f;
	
	glm::vec3** globe = new glm::vec3*[lines];
	for(int i = 0; i < lines; ++i)
		globe[i] = new glm::vec3[lines];

	vector<glm::vec3> vertices;
	pointcloud pc;

	for (size_t i = 0; i < lines; i++)
	{
		float lat = ((2*glm::half_pi<float>() / lines)*i);

		for (size_t j = 0; j< lines; j++)
		{
			float lon = ((2*glm::pi<float>() / lines)*j);

			float x = r * sin(lon) * cos(lat);
			float y = r * sin(lon) * sin(lat);
			float z = r * cos(lon);
			glm::vec3 pt = {x, y, z};
			globe[i][j] = pt;
			pc.addpoint(pt);
			
		}
	}

	for (size_t i = 0; i < lines-1; i++)
	{
		for (size_t j = 0; j< lines - 1; j++)
		{
			glm::vec3 pt1 = globe[i][j];
			glm::vec3 pt2 = globe[i][j+1];
			glm::vec3 pt3 = globe[i+1][j];

			vertices.push_back(pt1);
			vertices.push_back(pt2);
			vertices.push_back(pt3);
		}
	}


	pc.init();
	pc.position = { 0.0, 0.0, 0.0 };
	eng.sc->meshes.push_back(&pc);
	

	eng.sc->generate_shaders();
	eng.run();

	glfwTerminate();
	return 0;
}

