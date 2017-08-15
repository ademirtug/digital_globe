#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "camera.h"
#include "light.h"

class scene
{
public:
	scene();
	GLuint vao_mesh_id, vao_lights_id, fbo_depth_map,vbo_depth_map;
	std::vector<imesh*> meshes;
	unsigned int shadow_w, shadow_h;

	directionallight dirlight;
	std::vector<pointlight> plights;
};

class engine
{
protected:
	GLFWwindow* window;
	std::map<GLuint, std::vector<GLuint>> buffers;

public:
	engine();
	virtual ~engine();

	bool init(int width, int height);
	void load_shaders(std::string name);

	camera* cam;
	scene* sc;

	std::map<std::string, program*> programs;

	void run();
};


