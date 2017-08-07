#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "camera.h"



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
	scene sc;

	std::map<std::string, program*> programs;

	void run();
};