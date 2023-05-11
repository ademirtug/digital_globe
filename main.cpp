
//#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib,"winhttp.lib")
//#pragma comment(lib,"wininet.lib")
//#pragma comment (lib, "gdiplus.lib")
#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "de2.lib")


//https://github.com/ademirtug/de2/
#include "de2.h"
#include <iostream>
#include <set>
#include "spheroid.h"
#include "util.h"

using namespace std;
using namespace ecs_s;

int main()
{
	auto& eng = de2::get_instance();
	eng.init();
	eng.programs["c_t_point"] = std::make_shared<program>("c_t_point", "shaders/c_t_point.vert", "shaders/c_t_point.frag");
	eng.programs["c_t_direct"] = std::make_shared<program>("c_t_direct", "shaders/c_t_direct.vert", "shaders/c_t_direct.frag");

	spheroid s(6378137.0f, 6356752.3f);
	registry world;
	size_t level = 3;
	renderer_system renderer;

	//approximate sun position
	time_t rawtime;
	time(&rawtime);
	float e2 = 2 * glm::pi<float>() * ((gmtime(&rawtime)->tm_hour ) / 24.0);

	float x = cos(e2);
	float y = sin(e2);
	float z = 0;
	//float x = 1.0;
	//float y = 0;
	//float z = 0;

	renderer.l = std::make_shared<directional_light>(glm::vec3({ x, y, z }));

	float fps = 0;
	auto begin = std::chrono::high_resolution_clock::now();
	auto end = begin;
	de2::get_instance().on<pre_render>([&](std::chrono::nanoseconds ns) {
		end = std::chrono::high_resolution_clock::now();

		fps++;
		if (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() > 10000000) {
			eng.set_title("Frame Time(msec): " + std::to_string(1000 / fps));
			begin = end;
			fps = 0;
		}
		s.process(world, level);
		});

	de2::get_instance().on<render>([&world, &renderer](std::chrono::nanoseconds ns) {
		renderer.process(world, ns);
		});

	//util test
	double lat = 0;
	double lon = 0;
	double mercx = 0;
	double mercy = 0;

	std::vector<glm::vec3> mx;
	std::vector<glm::vec3> lx;
	std::vector<glm::vec3> m_y_lat;
	std::vector<float> m_x_lon;
	float step = 128;
	for (size_t i = 0; i <= 8; i++) {
		std::cout << "merc_x: (" << i * step << ") ->" << merc_x_to_lon(i * step, 1024) << std::endl;
	}
	std::cout << std::endl;

	for (size_t i = 0; i <= 8; i++) {
		std::cout << "merc_y: (" << i * step << ") ->" << merc_y_to_lat(i * step, 1024) << std::endl;
	}

	glm::vec3 ecef = merc_to_ecef({ 512, 512, 0 }, 1024);

	//for (size_t i = 0; i <= 8; i++) {
	//	std::cout << "merc_x: (" << i * step << ") ->" << merc_y_to_lat(i * step, 1024) << std::endl;

	//}
	//eng.enable_wireframe_mode();
	//eng.enable_point_mode();
	eng.run();

	return 0;
}