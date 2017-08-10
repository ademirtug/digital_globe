// 3d_engine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "camera.h"

using namespace std;
extern engine eng;



int main()
{
	arcball_camera* cam = new arcball_camera();
	eng.cam = cam;
	eng.init(1024, 768);


	eng.load_shaders("standartlight");
	eng.load_shaders("lightsource");
	eng.load_shaders("uv");
	

	pointlight pl("tcube.txt");
	eng.sc.plights.push_back(pl);

	//colormesh m("nano.txt");
	texturemesh m("textcube.txt");
	m.tex = new texture("c2.bmp");
	m.spec = new texture("c2_spec.bmp");


	eng.sc.meshes.push_back(&m);
	eng.run();


	glfwTerminate();
	return 0;
}

