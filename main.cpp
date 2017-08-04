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



vector<glm::vec3> gettri(string filename)
{
	vector<glm::vec3> v;

	fstream f;
	f.open(filename, std::fstream::in | std::fstream::binary);
	if (!f.is_open())
		return v;

	string line;
	while (getline(f, line))
	{
		istringstream ss(line);
		string cmd;
		ss >> cmd;

		if (cmd == "v")
		{
			float v1, v2, v3;

			ss >> v1 >> v2 >> v3;
			v.push_back({ v1, v2, v3 });
		}
	}
	return v;
}

int main()
{
	arcball_camera* cam = new arcball_camera();
	eng.cam = cam;
	eng.init(1024, 768);
	eng.load_shaders("standart");
	eng.load_shaders("standartlight");
	eng.load_shaders("lightsource");
	eng.load_shaders("uv");


	//LOAD LIGHT
	mesh m("textcube.txt", "c2.bmp", "c2_spec.bmp");
	mesh m2("textcube.txt", "c2.bmp", "c2_spec.bmp");
	mesh m3("textcube.txt", "c2.bmp", "c2_spec.bmp");

	//mesh m("teapot.txt");
	m.position = { -2, -5, 0 };
	m2.position = { -10, -10, 0 };
	eng.scene.meshes.push_back(&m);
	eng.scene.meshes.push_back(&m2);
	eng.scene.meshes.push_back(&m3);
	
	lightsource l("tcube.txt");

	eng.lights.meshes.push_back(&l);

	eng.run();

	glfwTerminate();
	return 0;
}

