#ifndef __SCENE_H__
#define __SCENE_H__

#include "stdafx.h"
#include "quadtile.h"
#include "camera.h"
#include "sphere.h"
class scene
{
public:
	scene();
	void generate_shaders();
	void draw();
	
	shared_ptr<spheroid> earth;
	shared_ptr<camera> cam;
	GLuint vao_mesh_id, vao_lights_id;
	std::vector<shared_ptr<imesh>> meshes;
	
	directionallight dirlight;
	std::vector<pointlight> plights;
	std::map<std::string, program*> programs;

	bool enable_dirlight;
	float width, height;
};



#endif//__SCENE_H__