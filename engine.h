#pragma once
#include "stdafx.h"
#include "scene.h"



class engine
{
protected:
	
	std::map<GLuint, std::vector<GLuint>> buffers;

public:
	engine();
	virtual ~engine();

	bool init(int width, int height);

	camera* cam;
	scene* sc;
	GLFWwindow* window;
	
	int maxfps;

	void run();
};


