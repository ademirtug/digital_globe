#include "stdafx.h"
#include "program.h"

#include <GL\GL.h>


program::program(void)
{
	id = glCreateProgram();
}


program::~program(void)
{
}


GLenum program::attach_shader(shader* shader)
{
	glAttachShader(id, *shader);

	char eb[500];
	int rl = 0;
	glGetProgramInfoLog(id, 500, &rl, eb);


	return glGetError();
}

GLenum program::link()
{
	glLinkProgram(id);

	char eb[500];
	int rl = 0;
	glGetProgramInfoLog(id, 500, &rl, eb);

	return glGetError();
}

GLuint program::get_id()
{
	return id;
}

void program::operator=(GLuint val)
{
	id = val;
}


void program::setuniform(std::string name, GLint v)
{
	GLuint uniformid = glGetUniformLocation(id, name.c_str());
	glUniform1i(uniformid, v);
}

void program::setuniform(std::string name, GLfloat v)
{
	GLuint uniformid = glGetUniformLocation(id, name.c_str());
	glUniform1f(uniformid, v);
}

void program::setuniform(std::string name, glm::vec3 v)
{
	GLuint uniformid = glGetUniformLocation(id, name.c_str());
	glUniform3fv(uniformid, 1, &v[0]);
}

void program::setuniform(std::string name, glm::mat4 v)
{
	GLuint uniformid = glGetUniformLocation(id, name.c_str());
	glUniformMatrix4fv(uniformid, 1,GL_FALSE,  &v[0][0]);
}