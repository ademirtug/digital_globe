
#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "de2.lib")


//https://github.com/ademirtug/de2/
#include "de2.h"
#include "spheroid.h"

using namespace std;
using namespace ecs_s;

int main()
{
	de2::get_instance().init();
	de2::get_instance().programs["c_t_direct"] = std::make_shared<program>("c_t_direct", "shaders/c_t_direct.vert", "shaders/c_t_direct.frag");

	spheroid s(earth_a, earth_b, 8);
	registry world;
	renderer_system renderer;
	//+x meridian
	//+y panama
	//+z south pole
	//approximate sun position
	time_t rawtime;
	time(&rawtime);
	float e2 = 2 * glm::pi<float>() * ((gmtime(&rawtime)->tm_hour) / 24.0);
	auto l = std::make_shared<directional_light>(glm::vec3({ -cos(e2), sin(e2), 0 }));
	l->ambient = { 0.7, 0.7, 0.7 };
	renderer.l = l;
	renderer.z_near = 100;
	renderer.z_far = 30000000;
	//renderer.l = std::make_shared<directional_light>(glm::vec3({ -1, 1, 0 }));


	de2::get_instance().on<pre_render>([&](std::chrono::nanoseconds ns) {
		s.process(world, renderer);
		});
	
	de2::get_instance().on<render>([&](std::chrono::nanoseconds ns) {
		renderer.process(world, ns);	
		});

	de2::get_instance().run();

	return 0;
}