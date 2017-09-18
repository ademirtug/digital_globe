
#include "stdafx.h"
#include "light.h"

extern engine eng;

pointlight::pointlight()
{
	vboid_vertices = 0;
	position = { 2.0f, 2.0f, 2.0f };
	ambient = { 0.05f, 0.05f, 0.05f };
	diffuse = { 0.8f, 0.8f, 0.8f };
	specular = { 1.0f, 1.0f, 1.0f };

	constant = 1.0f;
	linear = 0.0009f;
	quadratic = 0.00032f;
}

pointlight::pointlight(const std::string& filename) :pointlight()
{
	vboid_vertices = 0;
	loadobj(filename);
	init();
}

pointlight::pointlight(vector<glm::vec3> _vertices)
{
	vboid_vertices = 0;
	vertices = _vertices;

	glBindVertexArray(eng.sc->vao_lights_id);
	glGenBuffers(1, &vboid_vertices);

	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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



directionallight::directionallight()
{
	direction = { -0.2f, -1.0f, -0.3f };
	ambient = { 0.05f, 0.05f, 0.05f };
	diffuse = { 0.4f, 0.4f, 0.4f };
	specular = { 0.5f, 0.5f, 0.5f };
}
