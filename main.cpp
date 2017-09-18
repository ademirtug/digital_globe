// 3d_engine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "camera.h"

using namespace std;
extern engine eng;

int main()
{
	//qball_camera* cam = new qball_camera();
	//arcball_camera* cam = new arcball_camera();
	eu_orbit_camera* cam = new eu_orbit_camera();


	eng.cam = cam;
	eng.init(1024, 768);
	eng.maxfps = 500;

	pointlight pl("tcube.txt");
	
	pl.position = { 2.0f, 2.5f, 2.0f };
	pl.scale = glm::vec3(0.2f);
	

	eng.sc->plights.push_back(pl);

	texturemesh tm("textcube.txt");
	tm.position = { 0.0, 1.0, 0.0 };
	tm.tex = new texture("c2.bmp");
	tm.spec = new texture("c2_spec.bmp");
	eng.sc->meshes.push_back(&tm);


	texturemesh floor("plane.txt");
	floor.tex = new texture("wood.bmp");
	eng.sc->meshes.push_back(&floor);

	eng.sc->generate_shaders();
	eng.run();


	glfwTerminate();
	return 0;
}

