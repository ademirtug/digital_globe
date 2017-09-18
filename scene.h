#ifndef __SCENE_H__
#define __SCENE_H__

#include "stdafx.h"


class scene
{
public:
	scene();
	void generate_shaders();
	void draw();

	GLuint vao_mesh_id, vao_lights_id, fbo_depth_map, vbo_depthcube_map;
	std::vector<imesh*> meshes;
	unsigned int shadow_w, shadow_h;

	directionallight dirlight;
	std::vector<pointlight> plights;
	std::map<std::string, program*> programs;
};



#endif//__SCENE_H__