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

void uniform::operator=(glm::mat4 v)
{
	glUniformMatrix4fv(uni, 1, GL_FALSE, (GLfloat*)&v[0]);
}

uniform::~uniform()
{
}


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
	glUniformMatrix4fv(uniformid, 1, GL_FALSE, &v[0][0]);
}