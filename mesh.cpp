#include "stdafx.h"
#include "mesh.h"

extern engine eng;

vector<glm::vec3> arr2vec3(float* data, int size)
{
	vector<glm::vec3> vt;

	for (int i = 0; i < size; i += 3)
	{
		glm::vec3 v(data[i], data[i + 1], data[i + 2]);
		vt.push_back(v);
	}

	return vt;
}


void arr2ind(float* data, int size, vector<glm::vec3>& vertices, vector<int>& indices)
{
	vector<glm::vec3> tmp = arr2vec3(data, size);

	for (auto vx : tmp)
	{
		bool f = false;
		for (auto vy : vertices)
		{
			if (vx == vy)
			{
				f = true;
				break;
			}
		}
		if (!f) vertices.push_back(vx);
	}

	for (auto vx : tmp)
	{
		for (size_t i = 0; i < vertices.size(); i++)
		{
			if (vx == vertices[i])
			{
				indices.push_back(i);
				break;
			}
		}
	}
}
template<typename Out> void split(const std::string &s, char delim, Out result) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

imesh::imesh()
{
}
imesh::~imesh()
{
	//delete yazmayı unutma
}

colormesh::colormesh()
{
	vboid_vertices = vboid_normals = vboid_colors = 0;
	//position = glm::vec3(0.0f);
}

colormesh::colormesh(const std::string& objfile)
{
	vboid_vertices = vboid_normals = vboid_colors = 0;

	loadobj(objfile);

	init();

}

colormesh::colormesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals, vector<glm::vec3> _colors)
{
	//buaradan
	vboid_vertices = vboid_normals = vboid_colors = 0;
	vertices = _vertices;
	normals = _normals;

	colors = _colors;
	init();
}
colormesh::~colormesh()
{
}


void colormesh::init()
{
	glBindVertexArray(eng.sc->vao_mesh_id);

	glGenBuffers(1, &vboid_vertices);
	glGenBuffers(1, &vboid_normals);


	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	if (normals.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboid_normals);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void colormesh::loadobj(const std::string& filename)
{
	fstream f;
	f.open(filename, std::fstream::in | std::fstream::binary);
	if (!f.is_open())
		return;

	vector<glm::vec3> v;
	vector<glm::vec3> vn;


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
		else if (cmd == "vn")
		{
			float v1, v2, v3;

			ss >> v1 >> v2 >> v3;
			vn.push_back({ v1, v2, v3 });
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

					if (rs.size() > 2 && rs[2].size() > 0)
					{
						//push normal
						v1 = atoi(rs[2].c_str());
						normals.push_back(vn[--v1]);
					}
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
string colormesh::spname()
{
	return "standartlight";
}
void colormesh::draw()
{

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (normals.size() > 0)
	{
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vboid_normals);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}



texturemesh::texturemesh()
{
	vboid_vertices = vboid_normals = vboid_uv = 0;
	tex = 0;
	spec = 0;
}

texturemesh::texturemesh(const std::string& objfile, const std::string& texfile, const std::string& diffusefile) : texturemesh()
{
	loadobj(objfile);

	if (texfile != "")
		tex = new texture(texfile);

	if (diffusefile != "")
		spec = new texture(diffusefile);

	init();

}



texturemesh::texturemesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals, vector<glm::vec2> _uv) : texturemesh()
{
	vertices = _vertices;
	normals = _normals;
	uv = _uv;

	init();
}
texturemesh::~texturemesh()
{
}


void texturemesh::init()
{
	glBindVertexArray(eng.sc->vao_mesh_id);

	glGenBuffers(1, &vboid_vertices);
	glGenBuffers(1, &vboid_normals);
	glGenBuffers(1, &vboid_uv);

	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	if (normals.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboid_normals);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	}


	if (uv.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboid_uv);
		glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), &uv[0], GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void texturemesh::loadobj(const std::string& filename)
{
	fstream f;
	f.open(filename, std::fstream::in | std::fstream::binary);
	if (!f.is_open())
		return;

	vector<glm::vec3> v;
	vector<glm::vec3> vn;
	vector<glm::vec2> vu;


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
		else if (cmd == "vn")
		{
			float v1, v2, v3;

			ss >> v1 >> v2 >> v3;
			vn.push_back({ v1, v2, v3 });
		}
		else if (cmd == "vt")
		{
			float v1, v2;

			ss >> v1 >> v2;
			vu.push_back({ v1, v2 });
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

					if (rs[1].size() > 0)
					{
						//push texture
						v1 = atoi(rs[1].c_str());
						uv.push_back(vu[--v1]);
					}
					if (rs.size() > 2 && rs[2].size() > 0)
					{
						//push normal
						v1 = atoi(rs[2].c_str());
						normals.push_back(vn[--v1]);
					}
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
string texturemesh::spname()
{
	return "uv";
}
void texturemesh::draw()
{
	if (tex != NULL && tex->vboid_texture != NULL)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex->vboid_texture);
	}
	if (spec && spec->vboid_texture != 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, spec->vboid_texture);
	}

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (normals.size() > 0)
	{
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vboid_normals);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	if (uv.size() > 0)
	{
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, vboid_uv);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	}

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}



//lightsource::lightsource()
//{
//	vboid_vertices = 0;
//}
//
//lightsource::lightsource(const std::string& filename)
//{
//	throw("not implemented");
//	vboid_vertices = 0;
//	loadobj(filename);
//	init();
//}
//
//lightsource::lightsource(vector<glm::vec3> _vertices)
//{
//	vboid_vertices = 0;
//	vertices = _vertices;
//
//	glBindVertexArray(eng.sc->vao_lights_id);
//	glGenBuffers(1, &vboid_vertices);
//
//	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
//	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindVertexArray(0);
//}


//void lightsource::loadobj(const string& filename)
//{
//	fstream f;
//	f.open(filename, std::fstream::in | std::fstream::binary);
//	if (!f.is_open())
//		return;
//
//	string line;
//	while (getline(f, line))
//	{
//		istringstream ss(line);
//		string cmd;
//		ss >> cmd;
//
//		if (cmd == "v")
//		{
//			float v1, v2, v3;
//			ss >> v1 >> v2 >> v3;
//			vertices.push_back({ v1, v2, v3 });
//		}
//	}
//}


//void lightsource::init()
//{
//	glBindVertexArray(eng.sc->vao_lights_id);
//	glGenBuffers(1, &vboid_vertices);
//
//	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
//	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindVertexArray(0);
//}
//
//void lightsource::draw()
//{
//	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
//
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	glDrawArrays(GL_TRIANGLES, 0, vertices.size() );
//}
