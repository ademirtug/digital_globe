#ifndef __SHADER_PROGRAM_H__
#define __SHADER_PROGRAM_H__

#include "stdafx.h"

class program
{
protected:
	GLuint id;
public:
	program(void);
	virtual ~program(void);

	GLenum attach_shader(shader* shader);
	GLenum link();
	GLuint get_id();

	void setuniform(std::string name, GLfloat v);
	void setuniform(std::string name, GLint v);
	void setuniform(std::string name, glm::vec3);
	void setuniform(std::string name, glm::mat4 v);

	operator GLuint() { return id; }
	void operator=(GLuint);
};


#endif//__SHADER_PROGRAM_H__
