
//#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib,"winhttp.lib")
//#pragma comment(lib,"wininet.lib")
//#pragma comment (lib, "gdiplus.lib")
#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "de2.lib")


#include <set>
#include <iostream>

//https://github.com/ademirtug/de2/
#include "de2.h"
#include "util.h"
#include "spheroid.h"
#include "map_provider.h"


using namespace std;
using namespace ecs_s;

int main()
{
	auto& eng = de2::get_instance();
	eng.init();
	eng.programs["c_t_direct"] = std::make_shared<program>("c_t_direct", "shaders/c_t_direct.vert", "shaders/c_t_direct.frag");

	spheroid s(earth_a, earth_b);
	registry world;
	renderer_system renderer;
	//+x meridian
	//+y panama
	//+z south pole
	//approximate sun position
	time_t rawtime;
	time(&rawtime);
	float e2 = 2 * glm::pi<float>() * ((gmtime(&rawtime)->tm_hour) / 24.0);
	renderer.l = std::make_shared<directional_light>(glm::vec3({ -cos(e2), sin(e2), 0 }));
	//renderer.l = std::make_shared<directional_light>(glm::vec3({ 0, 0, -1 }));


	de2::get_instance().on<pre_render>([&](std::chrono::nanoseconds ns) {
		s.process(world, renderer);
		});
	
	de2::get_instance().on<render>([&](std::chrono::nanoseconds ns) {
		renderer.process(world, ns);	
		});

	eng.run();

	return 0;
}