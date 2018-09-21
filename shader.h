#ifndef __SHADER_H__
#define __SHADER_H__

#include "stdafx.h"


class shader 
{
	GLuint id;
public:
	shader(GLenum shader_type);
	virtual ~shader();


	virtual GLuint compile(const std::string& source);
	operator GLuint() { return id; }
	void operator=(GLuint);
};


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


class uniform
{
protected:
	GLint uni;
	program _sp;
public:
	uniform(program& sp, const std::string& _name);
	virtual ~uniform();

	void operator=(float v);
	void operator=(std::initializer_list<GLfloat> v);
	void operator=(glm::mat4 v);
};

#endif //__SHADER_H__