#ifndef __SCENE_H__
#define __SCENE_H__

#include "stdafx.h"


class scene
{
public:
	scene();
	void generate_shaders();
	void draw();
	
	std::mutex mxmeshes;

	GLuint vao_mesh_id, vao_lights_id;
	std::vector<imesh*> meshes;

	directionallight dirlight;
	std::vector<pointlight> plights;
	std::map<std::string, program*> programs;

	bool enable_dirlight;
	float width, height;
};



#endif//__SCENE_H__