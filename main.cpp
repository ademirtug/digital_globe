#include "stdafx.h"
#include "camera.h"
#include "sphere.h"


using namespace std;
extern engine eng;


int main()
{
	qball_camera *cam = new qball_camera();

	eng.cam = cam;
	eng.init(1024, 768);
	eng.maxfps = 25;

	spheroid earth(6378137.0f, 6356752.3f);

	eng.sc->generate_shaders();
	eng.run();

	glfwTerminate();
	return 0;
}