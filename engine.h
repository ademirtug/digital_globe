#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


class engine
{
protected:
	GLFWwindow* window;
	std::map<GLuint, std::vector<GLuint> > buffers;

public:
	engine();
	virtual ~engine();

	bool init(int width, int height);
	void load_shaders(std::string name);


	vao scene;
	vao lights;
	camera* cam;

	std::map<std::string, program*> programs;

	void run();
};

