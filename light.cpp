
#include "stdafx.h"
#include "light.h"

extern engine eng;

pointlight::pointlight()
{
	vboid_vertices = 0;
	position = { 2.0f, 2.0f, 2.0f };
	color = { 1.0f, 1.0f, 1.0f };

	ambient = { 0.1f, 0.1f, 0.1f };
	diffuse = { 1.0f, 1.0f, 1.0f };
	specular = { 1.0f, 1.0f, 1.0f };

	constant = 1.0f;
	linear = 0.009f;
	quadratic = 0.0032f;

	near_plane = 1.0f;
	far_plane = 45.0f;
}

pointlight::pointlight(const std::string& filename) :pointlight()
{
	vboid_vertices = 0;
	loadobj(filename);
	init();
}

pointlight::pointlight(vector<glm::vec3> _vertices) :pointlight()
{
	vboid_vertices = 0;
	vertices = _vertices;

	glBindVertexArray(eng.sc->vao_lights_id);
	glGenBuffers(1, &vboid_vertices);

	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	init();
}

void pointlight::loadobj(const string& filename)
{
	fstream f;
	f.open(filename, std::fstream::in | std::fstream::binary);
	if (!f.is_open())
		return;

	vector<glm::vec3> v;

	string line;
	while (getline(f, line))
	{
		istringstream ss(line);
		string cmd;
		ss >> cmd;

		if (cmd == "v")
		{
			float v1, v2, v3;

			ss >> v1 >> v2 >> v3;
			v.push_back({ v1, v2, v3 });
		}
		else if (cmd == "f")
		{

			while (!ss.eof())
			{
				string sect;
				ss >> sect;

				if (sect.find("/") != string::npos)
				{
					vector<string> rs = split(sect, '/');
					int v1 = atoi(rs[0].c_str());

					vertices.push_back(v[--v1]);
				}
				else
				{
					while (!ss.eof())
					{
						int v1;
						ss >> v1;
						vertices.push_back(v[--v1]);
					}
				}
			}
		}
		else
		{
		}
	}
}

void pointlight::init()
{
	glBindVertexArray(eng.sc->vao_lights_id);
	glGenBuffers(1, &vboid_vertices);

	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);



}

void pointlight::draw()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}


vector<glm::mat4> pointlight::getshadowtransforms()
{
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)1024 / (float)1024, near_plane, far_plane);
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	return shadowTransforms;
}

string pointlight::generate_fragshader(scene* sc)
{
	string shader = "#version 330 core\n";
	shader += "	out vec3 color;\n";
	shader += "void main()\n";
	shader += "{\n";
	shader += "	color = vec3(1, 1, 1);\n";
	shader += "}\n";

	return shader;

}
string pointlight::generate_vertshader(scene* sc)
{
	string shader = "#version 330 core\n";
	shader += "layout(location = 0) in vec3 aPos;\n";

	shader += "uniform mat4 model;\n";
	shader += "uniform mat4 view;\n";
	shader += "uniform mat4 projection;\n";

	shader += "void main() {\n";
	shader += "	gl_Position = projection * view * model * vec4(aPos, 1);\n";
	shader += "}\n";


	return shader;
}



directionallight::directionallight()
{
	direction = { -2.0f, 4.0f, -1.0f };
	ambient = { 0.3f, 0.3f, 0.3f };
	diffuse = { 0.7f, 0.7f, 0.7f };
	specular = { 0.7f, 0.7f, 0.7f };

	near_plane = 1.0f;
	far_plane = 45.0f;

	projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	view = glm::lookAt(direction, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	space = projection * view;
}


string directionallight::generate_fragshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"void main(){}\n";

	return shader;
}
string directionallight::generate_vertshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"

		"uniform mat4 DirLightSpaceMatrix;\n"
		"uniform mat4 model;\n"

		"void main()\n"
		"{\n"
		"	gl_Position = DirLightSpaceMatrix * model * vec4(aPos, 1.0);\n"
		"}\n";

	return shader;
}