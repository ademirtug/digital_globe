#include "stdafx.h"
#include "buffers.h"

mesh::mesh()
{
}

mesh::mesh(const float* _data, GLuint sizei)
{
	//memcpy(data, _data, 12);
	size = sizei;
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, sizei, _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLenum err = 0;;
	if (err != GL_NO_ERROR)
		std::cout << gluErrorString(err);

}

mesh::~mesh()
{
	if (data)
		delete data;
}

