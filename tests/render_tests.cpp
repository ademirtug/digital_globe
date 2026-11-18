#include "test_context.h"
#include "util.h"
#include "sky.h"
#include <cstdlib>

void check(bool condition, const char* message) {
	if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}

int main() {
	test_context context;
	{
		auto prg = std::make_shared<program>("test", "shaders/c_t_direct.vert", "shaders/c_t_direct.frag");
		prg->use();
		prg->setuniform("view", glm::mat4(1));
		prg->setuniform("projection", glm::mat4(1));
		auto bytes = std::make_shared<std::vector<unsigned char>>();
		std::string header = "P6\n8 8\n255\n";
		bytes->insert(bytes->end(), header.begin(), header.end());
		const std::array<glm::u8vec3, 4> colors{ glm::u8vec3(255, 0, 0), { 0, 255, 0 }, { 0, 0, 255 }, { 255, 255, 0 } };
		for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) {
			auto c = colors[(x >= 4 ? 1 : 0) + (y >= 4 ? 2 : 0)];
			bytes->insert(bytes->end(), { c.r, c.g, c.b });
		}
		auto image = std::make_shared<texture>(bytes);
		auto quad = std::make_shared<mesh>();
		quad->vertices = { { {-1,-1,0}, {0,0,1}, {0,0} }, { {1,-1,0}, {0,0,1}, {1,0} },
			{ {-1,1,0}, {0,0,1}, {0,1} }, { {1,1,0}, {0,0,1}, {1,1} } };
		quad->indices = { 0, 1, 2, 1, 3, 2 };
		model tile;
		tile.m = quad;
		tile.tex = image;
		tile.attach_program(prg);
		tile.upload();
		glViewport(0, 0, 32, 32);
		std::map<std::string, size_t> quadrants{ { "aa", 2 }, { "ab", 3 }, { "ac", 0 }, { "ad", 1 } };
		for (const auto& [path, index] : quadrants) {
			auto region = plate_texture_region(path, "a");
			tile.texture_offset = { region.x, region.y };
			tile.texture_scale = { region.z, region.w };
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			tile.draw();
			unsigned char pixel[4]{};
			glReadPixels(16, 16, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
			for (int channel = 0; channel < 3; channel++) check(std::abs(int(pixel[channel]) - int(colors[index][channel])) < 3, "actual shader samples the correct cropped ancestor image");
			check(glGetError() == GL_NO_ERROR, "fallback texture rendering produces no OpenGL errors");
		}

		sky_background sky;
		glViewport(0, 0, 256, 128);
		auto projection = glm::perspective(glm::radians(55.0f), 2.0f, 1.0f, 100.0f);
		auto view = glm::lookAt(glm::dvec3(3,0,0), glm::dvec3(0), glm::dvec3(0,0,1));
		auto render_sky = [&](const glm::dmat4& matrix) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			sky.draw(matrix, projection);
			std::vector<unsigned char> pixels(256 * 128 * 4);
			glReadPixels(0,0,256,128,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
			return pixels;
		};
		glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE); glDepthMask(GL_TRUE);
		auto stars = render_sky(view);
		GLboolean write;
		glGetBooleanv(GL_DEPTH_WRITEMASK,&write);
		check(glIsEnabled(GL_DEPTH_TEST) && glIsEnabled(GL_CULL_FACE) && write, "sky rendering restores globe depth and culling state");
		size_t light = 0, dark = 0;
		for(size_t i=0;i<stars.size();i+=4) {
			if(stars[i]+stars[i+1]+stars[i+2]>40) light++;
			if(stars[i]+stars[i+1]+stars[i+2]<10) dark++;
		}
		check(light>10 && dark>100, "the real sky image contains visible stars against dark space");
		auto moved = render_sky(glm::lookAt(glm::dvec3(30,0,0),glm::dvec3(0),glm::dvec3(0,0,1)));
		check(stars == moved, "the distant sky does not move when the camera changes distance");
		auto turned = render_sky(glm::lookAt(glm::dvec3(0,3,0),glm::dvec3(0),glm::dvec3(0,0,1)));
		check(stars != turned, "camera rotation reveals a different part of the sky");
		prg->use();
		prg->setuniform("view",glm::mat4(1));
		prg->setuniform("projection",glm::mat4(1));
		tile.texture_offset={0.5f,0.5f}; tile.texture_scale={0.5f,0.5f};
		tile.draw();
		unsigned char foreground[4]{};
		glReadPixels(128,64,1,1,GL_RGBA,GL_UNSIGNED_BYTE,foreground);
		check(foreground[0]>200 && foreground[1]>200 && foreground[2]<10, "foreground map geometry covers the star background");
		check(glGetError()==GL_NO_ERROR,"sky and globe drawing produce no OpenGL errors");
	}
	std::cout << "Passed: ancestor texture quadrants, real sky rendering, rotation, distance invariance, and foreground occlusion.\n";
}
