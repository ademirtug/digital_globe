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




#endif //__ISHADER_H__