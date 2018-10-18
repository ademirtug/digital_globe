#include "stdafx.h"
#include "mesh.h"
#include "scene.h"
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
	scale = glm::vec4(1.0f);
}
imesh::~imesh()
{
	//delete yazmayı unutma
}
glm::mat4 imesh::model()
{
	return glm::scale(glm::translate(glm::mat4(), position), scale);
}


//colormesh
colormesh::colormesh() : imesh()
{
	vboid_vertices = vboid_normals = vboid_uv = 0;
	shininess = 32.0f;
	diffuse = 0;
	specular = 1;
}
colormesh::colormesh(const std::string& objfile) : colormesh()
{
	loadobj(objfile);
	init();
}
colormesh::colormesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals) : colormesh()
{
	vertices = _vertices;
	normals = _normals;

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
	glGenBuffers(1, &vboid_uv);

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
	return "colormesh";
}
void colormesh::draw(glm::mat4 view, glm::mat4 projection)
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

	glm::mat4 m = model();
	glUseProgram(eng.sc->programs["colormesh"]->get_id());
	eng.sc->programs["colormesh"]->setuniform("model", model());
	eng.sc->programs["colormesh"]->setuniform("view", view);
	eng.sc->programs["colormesh"]->setuniform("projection", projection);

	eng.sc->programs["colormesh"]->setuniform("material.shininess", shininess);
	eng.sc->programs["colormesh"]->setuniform("material.diffuse", diffuse);
	eng.sc->programs["colormesh"]->setuniform("material.specular", specular);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}
void colormesh::shadowdraw()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

string colormesh::generate_geoshader(scene* sc)
{
	string shader = ""
		"#version 330\n"
		"layout(triangles) in;\n"
		"layout(line_strip, max_vertices = 8) out;\n"
		"float normal_length = 0.2;\n"
		"uniform mat4 gxl3d_ModelViewProjectionMatrix;\n "

		//"in vec3 gsFragPos;\n"
		//"in vec3 gsNormal;\n"
		//"in vec2 gsTexCoords;\n"
		//"out vec3 FragPos;\n"
		//"out vec3 Normal;\n"
		//"out vec2 TexCoords;\n"
		""
		""
		""
		"void main()\n"
		"{\n"
		//"	FragPos = gsFragPos;\n"
		//"	Normal = gsNormal;\n"
		//"	TexCoords = gsTexCoords;\n"
		"}\n"
		
		
		"";



	return shader;
}

string colormesh::generate_fragshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"in vec3 FragPos;\n"
		"in vec3 Normal;\n"
		"in vec2 TexCoords;\n"
		"out vec4 FragColor;\n"


		"struct Material {\n"
		"		sampler2D diffuse;\n"
		"		sampler2D specular;\n"
		"		float shininess;\n"
		"};\n"
		"uniform vec3 viewPos;\n"
		"uniform Material material;\n";

	if (sc->enable_dirlight)
	{
		shader += "struct DirLight {\n"
			"	vec3 direction;\n"
			"	vec3 ambient;\n"
			"	vec3 diffuse;\n"
			"	vec3 specular;\n"
			"};\n"
			"uniform DirLight dirLight;\n"
			"in vec4 DirLightSpaceFragPos;\n"
			"uniform sampler2D dldepthMap;\n"

			"float DirLightShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)\n"
			"{\n"
			"	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
			"	projCoords = projCoords * 0.5 + 0.5;\n"
			"	float closestDepth = texture(dldepthMap, projCoords.xy).r;\n"

			"	float currentDepth = projCoords.z;\n"
			"	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);\n"
			"	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;\n"
				
			"	return 0.0;\n"
			"}\n"

			"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
			"{\n"
			"	vec3 lightDir = normalize(light.direction);"
			"	vec3 color = vec3(1.0, 0.5, 0.31);\n"

			"	vec3 ambient = light.ambient * color;\n"

			"	float diff = max(dot(lightDir, normal), 0.0);\n"
			"	vec3 diffuse = diff * light.diffuse * color;\n"

			"	vec3 reflectDir = reflect(-lightDir, normal);\n"
			"	float spec = 0.0;\n"
			"	vec3 halfwayDir = normalize(lightDir + viewDir);\n"
			"	spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
			"	vec3 specular = spec * light.specular * color;\n"

			"	float shadow = DirLightShadowCalculation(DirLightSpaceFragPos, normal, lightDir);\n"
			"	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));\n"
			"	return lighting;"
			"}\n";
	}

	shader += "void main()\n"
		"{\n"
		"	vec3 norm = normalize(Normal);\n"
		"	vec3 viewDir = normalize(viewPos - FragPos);\n"
		"	vec3 result = vec3(0, 0, 0);\n";
	int i = 0;
	if (sc->enable_dirlight)
	{
		shader += "	result += CalcDirLight(dirLight, norm, FragPos, viewDir);\n";
	}
	shader += "	FragColor = vec4(result, 1.0); }\n";

	return shader;
}


string colormesh::generate_vertshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoords;\n"

		"out vec3 FragPos;\n"
		"out vec3 Normal;\n"
		"out vec2 TexCoords;\n"

		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n";


	if (sc->enable_dirlight)
	{
		shader += "out vec4 DirLightSpaceFragPos;\n"
			"uniform mat4 DirLightSpaceMatrix;\n";
	}

	shader += "void main()\n"
		"{\n"
		"		FragPos = vec3(model * vec4(aPos, 1.0));\n"
		"		Normal = transpose(inverse(mat3(model))) * aNormal;\n"
		"		TexCoords = aTexCoords;\n";
	

	if (sc->enable_dirlight)
	{
		shader += "DirLightSpaceFragPos = DirLightSpaceMatrix * vec4(FragPos, 1.0);\n";
	}

	shader += "	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}\n";

	return shader;
}



//pointcloud
pointcloud::pointcloud() : imesh()
{
	vboid_vertices = 0;
	shininess = 32.0f;
	diffuse = 0;
	specular = 1;
	pointsize = 5;
}
pointcloud::pointcloud(const std::string& objfile) : pointcloud()
{
	loadobj(objfile);
	init();
}
pointcloud::pointcloud(vector<glm::vec3> _vertices) : pointcloud()
{
	vertices = _vertices;

	init();
}
pointcloud::~pointcloud()
{
}
void pointcloud::addpoint(glm::vec3 point)
{
	vertices.push_back(point);
}
void pointcloud::init()
{
	glBindVertexArray(eng.sc->vao_mesh_id);

	glGenBuffers(1, &vboid_vertices);

	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void pointcloud::loadobj(const std::string& filename)
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
string pointcloud::spname()
{
	return "pointcloud";
}
void pointcloud::draw(glm::mat4 view, glm::mat4 projection)
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


	glm::mat4 m = model();
	glUseProgram(eng.sc->programs["pointcloud"]->get_id());
	eng.sc->programs["pointcloud"]->setuniform("model", model());
	eng.sc->programs["pointcloud"]->setuniform("view", view);
	eng.sc->programs["pointcloud"]->setuniform("projection", projection);

	eng.sc->programs["pointcloud"]->setuniform("material.shininess", shininess);
	eng.sc->programs["pointcloud"]->setuniform("material.diffuse", diffuse);
	eng.sc->programs["pointcloud"]->setuniform("material.specular", specular);
	glEnable(GL_PROGRAM_POINT_SIZE);
	
	glDrawArrays(GL_POINTS, 0, vertices.size());
}
void pointcloud::shadowdraw()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_POINTS, 0, vertices.size());
}
string pointcloud::generate_fragshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"in vec3 FragPos;\n"
		"in vec3 Normal;\n"
		"in vec2 TexCoords;\n"
		"out vec4 FragColor;\n"


		"struct Material {\n"
		"		sampler2D diffuse;\n"
		"		sampler2D specular;\n"
		"		float shininess;\n"
		"};\n"
		"uniform vec3 viewPos;\n"
		"uniform Material material;\n";

	if (sc->enable_dirlight)
	{
		shader += "struct DirLight {\n"
			"	vec3 direction;\n"
			"	vec3 ambient;\n"
			"	vec3 diffuse;\n"
			"	vec3 specular;\n"
			"};\n"
			"uniform DirLight dirLight;\n"
			"in vec4 DirLightSpaceFragPos;\n"
			"uniform sampler2D dldepthMap;\n"

			"float DirLightShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)\n"
			"{\n"
			"	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
			"	projCoords = projCoords * 0.5 + 0.5;\n"
			"	float closestDepth = texture(dldepthMap, projCoords.xy).r;\n"

			"	float currentDepth = projCoords.z;\n"
			"	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);\n"
			"	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;\n"

			"	return shadow;\n"
			"}\n"

			"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
			"{\n"
			"	vec3 lightDir = normalize(light.direction);"
			"	vec3 color = vec3(1.0, 0.5, 0.31);\n"

			"	vec3 ambient = light.ambient * color;\n"

			"	float diff = max(dot(lightDir, normal), 0.0);\n"
			"	vec3 diffuse = diff * light.diffuse * color;\n"

			"	vec3 reflectDir = reflect(-lightDir, normal);\n"
			"	float spec = 0.0;\n"
			"	vec3 halfwayDir = normalize(lightDir + viewDir);\n"
			"	spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
			"	vec3 specular = spec * light.specular * color;\n"

			"	float shadow = DirLightShadowCalculation(DirLightSpaceFragPos, normal, lightDir);\n"
			"	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));\n"
			"	return lighting;"
			"}\n";
	}

	shader += "void main()\n"
		"{\n"
		"	vec3 norm = normalize(Normal);\n"
		"	vec3 viewDir = normalize(viewPos - FragPos);\n"
		"	vec3 result = vec3(0, 0, 0);\n";
	if (sc->enable_dirlight)
	{
		shader += "	result += CalcDirLight(dirLight, norm, FragPos, viewDir);\n";
	}
	shader += "	FragColor = vec4(result, 1.0); }\n";

	return shader;

}
string pointcloud::generate_vertshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoords;\n"

		"out vec3 FragPos;\n"
		"out vec3 Normal;\n"
		"out vec2 TexCoords;\n"

		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n";


	if (sc->enable_dirlight)
	{
		shader += "out vec4 DirLightSpaceFragPos;\n"
			"uniform mat4 DirLightSpaceMatrix;\n";
	}

	shader += "void main()\n"
		"{\n"
		"		FragPos = vec3(model * vec4(aPos, 1.0));\n"
		"		Normal = transpose(inverse(mat3(model))) * aNormal;\n"
		"		TexCoords = aTexCoords;\n"
		"		gl_PointSize = "+ to_string(pointsize) +";\n";


	if (sc->enable_dirlight)
	{
		shader += "DirLightSpaceFragPos = DirLightSpaceMatrix * vec4(FragPos, 1.0);\n";
	}

	shader += "	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}\n";

	return shader;
}



//texturemesh
texturemesh::texturemesh() : imesh()
{
	vboid_vertices = vboid_normals = vboid_uv = 0;
	tex = 0;
	shininess = 32.0f;
	diffuse = 0;
	specular = { 0.7, 0.7 , 0.7 };
}
texturemesh::texturemesh(const std::string& objfile, const std::string& texfile) : texturemesh()
{
	loadobj(objfile);

	if (texfile != "")
		tex = new bmp(texfile);

	init();

}
texturemesh::texturemesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals, vector<glm::vec2> _uv) : texturemesh()
{
	vertices = _vertices;
	normals = _normals;
	uv = _uv;

	init();
}
texturemesh::texturemesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals, vector<glm::vec2> _uv, const std::string& texfile) : texturemesh()
{
	vertices = _vertices;
	normals = _normals;
	uv = _uv;

	if (texfile != "")
		tex = new bmp(texfile);

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
	return "texturemesh";
}
void texturemesh::draw(glm::mat4 view, glm::mat4 projection)
{
	if (tex != NULL && tex->vboid_texture != NULL)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex->vboid_texture);
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

	glm::mat4 m = model();
	glUseProgram(eng.sc->programs["texturemesh"]->get_id());
	eng.sc->programs["texturemesh"]->setuniform("model", model());
	eng.sc->programs["texturemesh"]->setuniform("view", view);
	eng.sc->programs["texturemesh"]->setuniform("projection", projection);

	eng.sc->programs["texturemesh"]->setuniform("material.shininess", shininess);
	eng.sc->programs["texturemesh"]->setuniform("material.diffuse", diffuse);
	eng.sc->programs["texturemesh"]->setuniform("material.specular", specular);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	

	if (tex != NULL && tex->vboid_texture != NULL)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}
void texturemesh::shadowdraw()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}
string texturemesh::generate_fragshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"in vec3 FragPos;\n"
		"in vec3 Normal;\n"
		"in vec2 TexCoords;\n"
		"out vec4 FragColor;\n"

		"\n"
		"struct Material {\n"
		"		sampler2D diffuse;\n"
		"		vec3 specular;\n"
		"		float shininess;\n"
		"};\n"

		"uniform vec3 viewPos;\n"
		"uniform Material material;\n";

	if (sc->enable_dirlight)
	{
		shader += "struct DirLight {\n"
			"	vec3 direction;\n"
			"	vec3 ambient;\n"
			"	vec3 diffuse;\n"
			"	vec3 specular;\n"
			"};\n"
			"uniform DirLight dirLight;\n"
			"uniform sampler2D dldepthMap;\n"
			"in vec4 DirLightSpaceFragPos;\n"

			"float DirLightShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)\n"
			"{\n"
			"	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
			"	projCoords = projCoords * 0.5 + 0.5;\n"
			"	float closestDepth = texture(dldepthMap, projCoords.xy).r;\n"

			"	float currentDepth = projCoords.z;\n"
			"	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);\n"
			"	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;\n"

			"	return shadow;\n"
			"}\n"

			"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
			"{\n"
			"	vec3 lightDir = normalize(light.direction);"
			"	vec3 color = texture(material.diffuse, TexCoords).rgb;\n"
			"	vec3 ambient = light.ambient * color;\n"

			"	float diff = max(dot(lightDir, normal), 0.0);\n"
			"	vec3 diffuse = diff * light.diffuse * color;\n"

			"	vec3 reflectDir = reflect(-lightDir, normal);\n"
			"	float spec = 0.0;\n"
			"	vec3 halfwayDir = normalize(lightDir + viewDir);\n"
			"	spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
			"	vec3 specular = spec * light.specular * color;\n"

			"	//final\n"
			"	float shadow = DirLightShadowCalculation(DirLightSpaceFragPos, normal, lightDir);\n"
			"	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));\n"
			"	return lighting;"
			"}\n";

	}
	shader += "void main()\n"
		"{\n"
		"	vec3 norm = normalize(Normal);\n"
		"	vec3 viewDir = normalize(viewPos - FragPos);\n"
		"	vec3 result = vec3(0, 0, 0);\n";

	if (sc->enable_dirlight)
	{
		shader += "	result += CalcDirLight(dirLight, norm, FragPos, viewDir);\n";
	}

	shader += "	FragColor = vec4(result, 1.0);\n"
		"}\n";

	return shader;

}
string texturemesh::generate_vertshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoords;\n"

		"out vec3 FragPos;\n"
		"out vec3 Normal;\n"
		"out vec2 TexCoords;\n"

		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n";

	if (sc->enable_dirlight)
	{
		shader += "out vec4 DirLightSpaceFragPos;\n"
			"uniform mat4 DirLightSpaceMatrix;\n";
	}

	shader += "void main()\n"
		"{\n"
		"	FragPos = vec3(model * vec4(aPos, 1.0));\n"
		"	Normal = transpose(inverse(mat3(model))) * aNormal;\n"
		"	TexCoords = aTexCoords;\n";

	if (sc->enable_dirlight)
	{
		shader += "	DirLightSpaceFragPos = DirLightSpaceMatrix * vec4(FragPos, 1.0);\n";
	}

	shader += "	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}\n";

	return shader;
}



//diffusetexturemesh
diffusetexturemesh::diffusetexturemesh() : imesh()
{
	vboid_vertices = vboid_normals = vboid_uv = 0;
	tex = 0;
	spec = 0;
	shininess = 64.0f;
	diffuse = 0;
}
diffusetexturemesh::diffusetexturemesh(const std::string& objfile, const std::string& texfile, const std::string& diffusefile) : diffusetexturemesh()
{
	loadobj(objfile);

	if (texfile != "")
		tex = new texture(texfile);

	if (diffusefile != "")
		spec = new texture(diffusefile);

	init();

}
diffusetexturemesh::diffusetexturemesh(vector<glm::vec3> _vertices, vector<glm::vec3> _normals, vector<glm::vec2> _uv) : diffusetexturemesh()
{
	vertices = _vertices;
	normals = _normals;
	uv = _uv;

	init();
}
diffusetexturemesh::~diffusetexturemesh()
{
}
void diffusetexturemesh::init()
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
void diffusetexturemesh::loadobj(const std::string& filename)
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
string diffusetexturemesh::spname()
{
	return "diffusetexturemesh";
}
void diffusetexturemesh::draw(glm::mat4 view, glm::mat4 projection)
{
	if (tex != NULL && tex->vboid_texture != NULL)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex->vboid_texture);
	}

	if (spec && spec->vboid_texture != 0)
	{
		glActiveTexture(GL_TEXTURE1);
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

	glm::mat4 m = model();
	glUseProgram(eng.sc->programs["diffusetexturemesh"]->get_id());
	eng.sc->programs["diffusetexturemesh"]->setuniform("model", model());
	eng.sc->programs["diffusetexturemesh"]->setuniform("view", view);
	eng.sc->programs["diffusetexturemesh"]->setuniform("projection", projection);

	eng.sc->programs["diffusetexturemesh"]->setuniform("material.shininess", shininess);
	eng.sc->programs["diffusetexturemesh"]->setuniform("material.diffuse", 0);
	eng.sc->programs["diffusetexturemesh"]->setuniform("material.specular", 1);//FARK 1

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());


	if (tex != NULL && tex->vboid_texture != NULL)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (spec && spec->vboid_texture != 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
void diffusetexturemesh::shadowdraw()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboid_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

string diffusetexturemesh::generate_fragshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"in vec3 FragPos;\n"
		"in vec3 Normal;\n"
		"in vec2 TexCoords;\n"
		"out vec4 FragColor;\n"
		"\n"
		"struct Material {\n"
		"		sampler2D diffuse;\n"
		"		sampler2D specular;\n"
		"		float shininess;\n"
		"};\n"
		"uniform vec3 viewPos;\n"
		"uniform Material material;\n";
			
	if (sc->enable_dirlight)
	{
		shader += "struct DirLight {\n"
			"	vec3 direction;\n"
			"	vec3 ambient;\n"
			"	vec3 diffuse;\n"
			"	vec3 specular;\n"
			"};\n"
			"uniform DirLight dirLight;\n"
			"uniform sampler2D dldepthMap;\n"
			"in vec4 DirLightSpaceFragPos;\n"

			"float DirLightShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)\n"
			"{\n"

			"	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
			"	projCoords = projCoords * 0.5 + 0.5;\n"
			"	float closestDepth = texture(dldepthMap, projCoords.xy).r;\n"
			"	float currentDepth = projCoords.z;\n"
			"	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);\n"

			"	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0; "
			"	return shadow;\n"
			"}\n"


			"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
			"{\n"
			"	vec3 lightDir = normalize(light.direction);"
			"	vec3 color = texture(material.diffuse, TexCoords).rgb;\n"
			"	vec3 ambient = light.ambient * color;\n"

			"	float diff = max(dot(lightDir, normal), 0.0);\n"
			"	vec3 diffuse = diff * light.diffuse * color;\n"

			"	vec3 reflectDir = reflect(-lightDir, normal);\n"
			"	float spec = 0.0;\n"
			"	vec3 halfwayDir = normalize(lightDir + viewDir);\n"
			"	spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
			"	vec3 specular = spec * light.specular * texture(material.specular, TexCoords).rgb;\n"

			"	float shadow = DirLightShadowCalculation(DirLightSpaceFragPos, normal, lightDir);\n"
			"	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));\n"
			"	return lighting;"
			"}\n";
	}
	shader += "void main()\n"
		"{\n"
		"		// properties\n"
		"		vec3 norm = normalize(Normal);\n"
		"		vec3 viewDir = normalize(viewPos - FragPos);\n"
		"		vec3 result = vec3(0, 0, 0);\n";

	if (sc->enable_dirlight)
	{
		shader += "	result += CalcDirLight(dirLight, norm, FragPos, viewDir);\n";
	}

	shader += "		FragColor = vec4(result, 1.0); \n } \n";

	return shader;
}
string diffusetexturemesh::generate_vertshader(scene* sc)
{
	string shader = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoords;\n"

		"out vec3 FragPos;\n"
		"out vec3 Normal;\n"
		"out vec2 TexCoords;\n"

		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n";

		if (sc->enable_dirlight)
		{
			shader += "out vec4 DirLightSpaceFragPos;\n"
				"uniform mat4 DirLightSpaceMatrix;\n";
		}

		shader += "void main()\n"
			"{\n"
			"		FragPos = vec3(model * vec4(aPos, 1.0));\n"
			"		Normal = transpose(inverse(mat3(model))) * aNormal;\n"
			"		TexCoords = aTexCoords;\n";

		if (sc->enable_dirlight)
		{
			shader += "DirLightSpaceFragPos = DirLightSpaceMatrix * vec4(FragPos, 1.0);\n";
		}
		
		shader += "	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}\n";


	return shader;
}