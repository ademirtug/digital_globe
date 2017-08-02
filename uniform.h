#ifndef __UNIFORM_H__
#define __UNIFORM_H__

#include "vec.h"

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
	void operator=(mat4<float> v);
};

#endif//__UNIFORM_H__