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
	eng.load_shaders("debugdepth");
	eng.load_shaders("depth");
	eng.load_shaders("shadowmapping");
	

	pointlight pl("tcube.txt");
	pl.position = { 2.0, 2.0, 2.0 };
	eng.sc->plights.push_back(pl);


	//pointlight pl2("tcube.txt");
	//pl2.position = { 0.0, 2.0, -4.0 };
	//eng.sc->plights.push_back(pl2);

	//colormesh cm("textcube.txt");
	//cm.position = { 0.0, 1.0, -2.0 };
	//eng.sc->meshes.push_back(&cm);
	

	texturemesh tm("textcube.txt");
	tm.position = { 0.0, 1.0, -2.0 };
	tm.tex = new texture("c2.bmp");
	//tm.spec = new texture("c2_spec.bmp");
	eng.sc->meshes.push_back(&tm);


	texturemesh floor("floor.txt");
	floor.tex = new texture("wood.bmp");
	eng.sc->meshes.push_back(&floor);


	//colormesh cf("tcube.txt");
	//cf.position = { 2.0, 7.0, 2.0 };
	//eng.sc->meshes.push_back(&cf);

	eng.run();


	glfwTerminate();
	return 0;
}

