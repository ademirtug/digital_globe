#ifndef __BUFFERS_H__
#define __BUFFERS_H__

class mesh
{
public:
	mesh();
	mesh(const float* _data, GLuint sizei);

	~mesh();
	
	
	float* data;
	GLuint id, size;
};

class vao
{
public:
	GLuint id;
	std::vector<mesh*> meshes;
};



#endif//__BUFFERS_H__