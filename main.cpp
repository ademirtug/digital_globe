// 3d_engine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "camera.h"

using namespace std;
extern engine eng;

float planeVertices[] = {
	10.0f, -0.5f,  10.0f,   
	-10.0f, -0.5f,  10.0f,   
	-10.0f, -0.5f, -10.0f,    

	10.0f, -0.5f,  10.0f,    
	-10.0f, -0.5f, -10.0f,     
	10.0f, -0.5f, -10.0f  
};


float planeNormals[] = { 
	0.0f, 1.0f, 0.0f,
0.0f, 1.0f, 0.0f,
0.0f, 1.0f, 0.0f,
0.0f, 1.0f, 0.0f, 
0.0f, 1.0f, 0.0f, 
0.0f, 1.0f, 0.0f };



vector<glm::vec2> planeCoords = { 
	{10.0f,  0.0f},
	{0.0f,  0.0f},
	{0.0f, 10.0f},
	{10.0f,  0.0f},
	{0.0f, 10.0f},
	{10.0f, 10.0f} };


int main()
{
	arcball_camera* cam = new arcball_camera();
	eng.cam = cam;
	eng.init(1024, 768);


	eng.load_shaders("standartlight");
	eng.load_shaders("lightsource");
	eng.load_shaders("uv");
	

	pointlight pl("tcube.txt");
	pl.position = { 2.0, 2.0, 2.0 };
	eng.sc.plights.push_back(pl);

	//colormesh m("nano.txt");
	//texturemesh m("textcube.txt");
	//m.tex = new texture("c2.bmp");
	//m.spec = new texture("c2_spec.bmp");


	//vector<glm::vec3> vfloor = arr2vec3(planeVertices, sizeof(planeVertices));
	//vector<glm::vec3> vnormals = arr2vec3(planeNormals, sizeof(planeNormals));

	//texturemesh floor(vfloor, vnormals, planeCoords);
	//floor.tex = new texture("wood.bmp");


	//eng.sc.meshes.push_back(&floor);


	colormesh cf("tcube.txt");
	cf.position = { 2.0, 7.0, 2.0 };
	eng.sc.meshes.push_back(&cf);

	eng.run();


	glfwTerminate();
	return 0;
}

