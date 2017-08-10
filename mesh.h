#ifndef __MESH_H__
#define __MESH_H__

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


class colormesh : public imesh
{
protected:
	virtual void init();
public:
	colormesh();
	colormesh(const std::string& objfile);
	colormesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals = {}, vector<glm::vec3> _colors = {});
	virtual ~colormesh();

	virtual void draw();
	virtual std::string spname();
	void loadobj(const std::string& filename);


	vector<glm::vec3> normals;
	vector<glm::vec3> colors;

	GLuint vboid_normals, vboid_colors;
};



class texturemesh :public imesh
{
protected:
	virtual void init();
public:
	texturemesh();
	texturemesh(const std::string& objfile, const std::string& texfile = "", const std::string& diffusefile = "");

	texturemesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals = {}, vector<glm::vec2> _uv = {});
	virtual ~texturemesh();

	virtual void draw();
	virtual std::string spname();
	void loadobj(const std::string& filename);


	vector<glm::vec3> normals;
	vector<glm::vec3> colors;
	vector<glm::vec2> uv;

	texture* tex;
	texture* spec;

	GLuint vboid_normals, vboid_uv;
};



void obj2mesh(std::string filename, imesh* m);



#endif//__MESH_H__