#ifndef __MESH_H__
#define __MESH_H__

#include "stdafx.h"
#include <sstream>
#include <iterator>

using namespace std;
class scene;

vector<glm::vec3> arr2vec3(float* data, int size);
void arr2ind(float* data, int size, vector<glm::vec3>& vertices, vector<int>& indices);

template<typename Out> void split(const std::string &s, char delim, Out result);
std::vector<std::string> split(const std::string &s, char delim);

int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32);

class imesh
{
public:
	imesh();
	virtual ~imesh();
	
	virtual std::string spname() { return ""; };
	virtual void draw(glm::mat4 view, glm::mat4 projection) {};
	virtual void shadowdraw() {};
	virtual std::string generate_geoshader(scene* sc) { return ""; };
	virtual std::string generate_fragshader(scene* sc) { return ""; };
	virtual std::string generate_vertshader(scene* sc) { return ""; };
	

	virtual void loadobj(const std::string& filename) = 0;
	virtual glm::mat4 model();
	
	
	GLuint vboid_vertices;
	glm::vec3 position;
	glm::vec3 scale;
	vector<glm::vec3> vertices;
};



class colormesh : public imesh
{
protected:
	virtual void init();
public:
	colormesh();
	colormesh(const std::string& objfile);
	colormesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals = {});
	virtual ~colormesh();

	void draw(glm::mat4 view, glm::mat4 projection);
	void shadowdraw();
	void loadobj(const std::string& filename);
	std::string generate_geoshader(scene* sc);
	std::string generate_fragshader(scene* sc);
	std::string generate_vertshader(scene* sc);


	std::string spname();

	float shininess, diffuse, specular;

	vector<glm::vec3> normals;
	vector<glm::vec3> colors;

	GLuint vboid_normals, vboid_uv;
};


class pointcloud : public imesh
{

public:
	pointcloud();
	pointcloud(const std::string& objfile);
	pointcloud(vector<glm::vec3> _vertices);
	virtual ~pointcloud();

	virtual void init();
	void draw(glm::mat4 view, glm::mat4 projection);
	void shadowdraw();
	void loadobj(const std::string& filename);
	std::string generate_fragshader(scene* sc);
	std::string generate_vertshader(scene* sc);
	void addpoint(glm::vec3 point);

	std::string spname();

	float shininess, diffuse, specular, pointsize;

	vector<glm::vec3> normals;
};

class texturemesh : public imesh
{
protected:
	virtual void init();
public:
	texturemesh();
	texturemesh(const std::string& objfile, const std::string& texfile = "");
	texturemesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals = {}, vector<glm::vec2> _uv = {});
	texturemesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals, vector<glm::vec2> _uv, const std::string& texfile);
	virtual ~texturemesh();

	void draw(glm::mat4 view, glm::mat4 projection);
	void shadowdraw();
	std::string spname();
	void loadobj(const std::string& filename);
	string generate_fragshader(scene* sc);
	string generate_vertshader(scene* sc);

	
	
	
	float shininess, diffuse;
	glm::vec3 specular;

	vector<glm::vec3> normals;
	vector<glm::vec3> colors;
	vector<glm::vec2> uv;

	texture* tex;

	GLuint vboid_normals, vboid_uv;
};


class diffusetexturemesh : public imesh
{
protected:
	virtual void init();
public:
	diffusetexturemesh();
	diffusetexturemesh(const std::string& objfile, const std::string& texfile = "", const std::string& diffusefile = "");

	diffusetexturemesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals = {}, vector<glm::vec2> _uv = {});
	virtual ~diffusetexturemesh();

	void draw(glm::mat4 view, glm::mat4 projection);
	void shadowdraw();
	std::string spname();
	void loadobj(const std::string& filename);

	std::string generate_fragshader(scene* sc);
	std::string generate_vertshader(scene* sc);

	float shininess, diffuse;

	vector<glm::vec3> normals;
	vector<glm::vec3> colors;
	vector<glm::vec2> uv;

	texture* tex;
	texture* spec;

	GLuint vboid_normals, vboid_uv;
};


void obj2mesh(std::string filename, imesh* m);



#endif//__MESH_H__