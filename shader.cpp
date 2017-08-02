#include "stdafx.h"
#include "shader.h"


shader::shader(GLenum shader_type)
{
	id =  glCreateShader(shader_type);
}


shader::~shader()
{
	//if (id > 0)//createshader is executed
	//	glDeleteShader(id);
}



GLuint shader::compile(const std::string& source = "")
{
	if (source.length() < 1)
		return GL_FALSE;

	const std::string ssrc = source.c_str();
	const GLchar *src = static_cast<const GLchar*>(ssrc.c_str());


	glShaderSource(id, 1, &src, NULL);

	glCompileShader(id);

	GLint bSuccess = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &bSuccess);

	if (bSuccess == GL_FALSE)
	{
		char eb[500];
		int rl = 0;
		glGetShaderInfoLog(id, 500, &rl, eb);

		std::string se(eb, rl);
		throw se.c_str();


		glDeleteShader(id);
		id = 0;
	}

	return bSuccess;
}


void shader::operator=(GLuint val)
{
	id = val;
}