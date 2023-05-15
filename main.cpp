
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
#include "map_provider.h"


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

	//wgs84
	//-x=meridian
	//+x=anti meridian
	//-y=india
	//+y=panama
	//-z=north pole
	//+z=south pole

	//approximate sun position
	time_t rawtime;
	time(&rawtime);
	float e2 = 2 * glm::pi<float>() * ((gmtime(&rawtime)->tm_hour) / 24.0);
	renderer.l = std::make_shared<directional_light>(glm::vec3({ cos(e2), sin(e2), 0 }));

	de2::get_instance().on<pre_render>([&](std::chrono::nanoseconds ns) {
		s.process(world, renderer.cam_->zoom_);
		});
	
	float fps = 0;
	auto begin = std::chrono::high_resolution_clock::now();
	auto end = begin;
	de2::get_instance().on<render>([&](std::chrono::nanoseconds ns) {

		renderer.process(world, ns);
		
		//Coordinates, based on sphere not WGS84 spheroid!!!!
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view = glm::rotate(renderer.cam_->getview(), glm::pi<float>() / 2, glm::vec3(1.0, 0, 0));
		projection = glm::perspective(glm::radians(45.0f), (float)de2::get_instance().viewport.x / (float)de2::get_instance().viewport.y, 0.1f, 100.0f);
		auto from = cast_ray(renderer.mouse_pos, { de2::get_instance().viewport.x , de2::get_instance().viewport.y }, projection, view, -1.0f);
		auto to = cast_ray(renderer.mouse_pos, { de2::get_instance().viewport.x , de2::get_instance().viewport.y }, projection, view, 1.0f);

		auto m_geo = sphere_intersection(from, to - from);
		std::string s_mgeo = std::format("lat:{:02.2f} lon:{:02.2f}", m_geo[0], m_geo[1]);
		de2::get_instance().set_title(s_mgeo);
		
		});

	eng.run();

	return 0;
}