# digital_globe
3D simplistic globe application.

## Requirements
- [msvc](https://visualstudio.microsoft.com/) **>= 2022**

## How to display your own data.
- You need to feed the system with your own provider which takes plate paths as an argument and return required map data.
```cpp
template<typename T>
class my_map_provider {
public:
	std::shared_ptr<texture> get(const std::string& plate_path) {
		//return map data
	}
};
```

## Basic Usage

```cpp
//https://github.com/ademirtug/de2/
#include "de2.h"
#include "spheroid.h"

using namespace std;
using namespace ecs_s;
double constexpr earth_a = 6378137.0f;
double constexpr earth_b = 6356752.3f;

int main()
{
	auto& eng = de2::get_instance();
	eng.init();
	eng.programs["c_t_direct"] = std::make_shared<program>("c_t_direct", "shaders/c_t_direct.vert", "shaders/c_t_direct.frag");

	spheroid s(earth_a, earth_b);
	registry world;
	renderer_system renderer;
	renderer.l = std::make_shared<directional_light>(glm::vec3({ 1, 0, 0 }));

	de2::get_instance().on<pre_render>([&](std::chrono::nanoseconds ns) {
		s.process(world, renderer);
		});
	
	de2::get_instance().on<render>([&](std::chrono::nanoseconds ns) {
		renderer.process(world, ns);	
		});

	eng.run();

	return 0;
}
```

![Screen Shot 1](s1.png?raw=true "screen shot 1")

![Screen Shot 2](s2.png?raw=true "screen shot 2")
