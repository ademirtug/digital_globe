// 3d_engine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "camera.h"



using namespace std;
extern engine eng;
//vector<glm::vec3> arr2vec3(float* data, int size);
//void arr2ind(float* data, int size, vector<glm::vec3>& vertices, vector<int>& indices);
//
//template<typename Out> void split(const std::string &s, char delim, Out result);
//std::vector<std::string> split(const std::string &s, char delim);
//
//void obj2mesh(std::string filename, vector<glm::vec3>& vertices, vector<glm::vec3>& normals);
//bool loadOBJ(const char * path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals);



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

