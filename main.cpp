// 3d_engine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "camera.h"

using namespace std;
extern engine eng;

int main()
{
	qball_camera* cam = new qball_camera();
	//arcball_camera* cam = new arcball_camera();
	eng.cam = cam;
	eng.init(1024, 768);
	eng.maxfps = 25;


	eng.load_shaders("lightsource");
	eng.load_shaders("uv");
	eng.load_shaders("pointshadow");
	eng.load_shaders("pointshadowdepth");
	eng.load_shaders("standartlight");
	

	pointlight pl("tcube.txt");
	pl.position = { 2.0f, 3.0f, 2.0f };
	eng.sc->plights.push_back(pl);




	//texturemesh tm("textcube.txt");
	//tm.position = { 0.0, 1.0, 0.0 };
	//tm.tex = new texture("c2.bmp");
	//tm.spec = new texture("c2_spec.bmp");
	//eng.sc->meshes.push_back(&tm);


	//texturemesh floor("plane.txt");
	//floor.tex = new texture("wood.bmp");
	//eng.sc->meshes.push_back(&floor);


	//colormesh cf("tcube.txt");
	//cf.position = { 2.0, 7.0, 2.0 };
	//eng.sc->meshes.push_back(&cf);

	texturemesh tm("earth.txt");
	tm.tex = new texture("earth.bmp");
	eng.sc->meshes.push_back(&tm);

	eng.run();


	glfwTerminate();
	return 0;
}

