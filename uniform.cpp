#include "stdafx.h"
#include "uniform.h"
#include "vec.h"

uniform::uniform(program& sp, const std::string& name)
{
	_sp = sp;
	uni = glGetUniformLocation(_sp, name.c_str());
}

void uniform::operator=(float v)
{

	glUniform1f(uni, v);

}

//void uniform::operator=(mat4* mat)
//{
//	glUniformMatrix4fv(uni, 1, GL_FALSE/*column major*/, mat->mat);
//}


void uniform::operator=(std::initializer_list<GLfloat> v)
{
	if (v.size() == 2)
		glUniform2f(uni, *v.begin(), *(v.begin() + 1));
}

void uniform::operator=(mat4<float> v)
{
	glUniformMatrix4fv(uni, 1, GL_FALSE, (GLfloat*)&v.data);
}

uniform::~uniform()
{
}


