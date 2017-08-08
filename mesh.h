#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#include "stdafx.h"
#include <sstream>
#include <iterator>

using namespace std;

vector<glm::vec3> arr2vec3(float* data, int size);
void arr2ind(float* data, int size, vector<glm::vec3>& vertices, vector<int>& indices);

template<typename Out> void split(const std::string &s, char delim, Out result);
std::vector<std::string> split(const std::string &s, char delim);


class imesh
{
public:
	imesh();
	virtual ~imesh();
	
	virtual std::string spname() { return ""; };
	virtual void draw() {};
	virtual void loadobj(const std::string& filename) = 0;
	
	
	GLuint vboid_vertices;
	glm::vec3 position;
	vector<glm::vec3> vertices;
	
};


class mesh : public imesh
{
protected:
	virtual void init();
public:
	mesh();
	mesh(const std::string& objfile, const std::string& texfile = "", const std::string& diffusefile = "");

	mesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals = {}, vector<glm::vec2> _uv = {}, vector<glm::vec3> _colors = {});
	virtual ~mesh();

	virtual void draw();
	virtual std::string spname();
	void loadobj(const std::string& filename);


	vector<glm::vec3> normals;
	vector<glm::vec3> colors;
	vector<glm::vec2> uv;

	texture* tex;
	texture* spec;

	GLuint vboid_normals, vboid_colors, vboid_uv;
};

void obj2mesh(std::string filename, mesh* m);

class ilight
{
public:
	virtual void shine() {};
};


class directionallight : public ilight
{
protected:
	virtual void init() {};
public:
};

class pointlight : public ilight, imesh
{
protected:
	virtual void init();
public:

	pointlight();
	pointlight(const std::string& filename);
	pointlight(vector<glm::vec3> _vertices);
	
	void draw();
	virtual std::string spname() { return ""; };
	void loadobj(const std::string& filename);

};


class scene
{
public:
	GLuint vao_mesh_id, vao_lights_id;
	std::vector<imesh*> meshes;
		
	
	//directionallight dirlight;
	std::vector<pointlight> plights;

};


#endif//__BUFFERS_H__