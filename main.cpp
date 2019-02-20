#include "stdafx.h"
#include "camera.h"
#include "sphere.h"


using namespace std;
extern engine eng;


int main()
{
	bool status = eng.init(1024, 768);
	eng.maxfps = 30;

	eng.run();

	glfwTerminate();
	return 0;
}