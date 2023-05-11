#include "stdafx.h"

#include <string>
//#include <glm/gtx/vector_angle.hpp>



extern engine eng;
using namespace std;

void search(string base, quadtile* tile)
{
	for (size_t i = 0; i < 4; i++)
	{
		string ch = base + (char)(65 + i);
	}
}

int zl = 2;
string currentfocus = "";

scene::scene()
{
	//bunu texture s�n�f�ndan t�reme shadow map yap
	enable_dirlight = true;
	width = 1024.0f;
	height = 768.0f;
}


void scene::draw()
{
	if (cam == nullptr)
	{
		cam.reset(new qball_camera());
		//cam.reset(new euler_angle_orbit());
		earth.reset(new spheroid(6378137.0f, 6356752.3f));
	}
	GLenum err = 0;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::vec3 cameraPos = eng.sc->cam->getpos();

	if (zl != cam->getzoomlevel())
	{
		cout << "" << endl;
		zl = cam->getzoomlevel();
	}

	//this part collects all plates that needs to be shown
	vector<quadtile*> tiles = earth->getdisplayedtiles(cameraPos, cam->getzoomlevel());

	if (enable_dirlight)
	{
		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, dirlight.sm.fbo_depthmap);
		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(programs["dirlightshadowdepth"]->get_id());
		programs["dirlightshadowdepth"]->setuniform("DirLightSpaceMatrix", dirlight.space);
		
		for (auto m : tiles)
		{
			if (m->tm == nullptr)
				continue;
			programs["dirlightshadowdepth"]->setuniform("model", m->tm->model());
			m->tm->shadowdraw();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	//NORMAL ��Z�M
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(45.0f, width/height, 100.0f, 50000000.0f);

	std::array<double, 3> lla = ecef_to_geo({ cameraPos.x, cameraPos.y, cameraPos.z });

	glm::mat4 view = eng.sc->cam->getview(lla[0], lla[1]);

	glBindVertexArray(vao_mesh_id);

	for (auto m : tiles)
	{
		if (m->tm == nullptr)
			continue;

		glUseProgram(programs[m->tm->spname()]->get_id());
		programs[m->tm->spname()]->setuniform("viewPos", cameraPos);

		if (enable_dirlight)
		{
			programs[m->tm->spname()]->setuniform("dirLight.direction", dirlight.direction);
			programs[m->tm->spname()]->setuniform("dirLight.ambient", dirlight.ambient);
			programs[m->tm->spname()]->setuniform("dirLight.diffuse", dirlight.diffuse);
			programs[m->tm->spname()]->setuniform("dirLight.specular", dirlight.specular);
			programs[m->tm->spname()]->setuniform("DirLightSpaceMatrix", dirlight.space);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, dirlight.sm.vboid_texture);

			programs[m->tm->spname()]->setuniform("far_plane", dirlight.far_plane);
			programs[m->tm->spname()]->setuniform("dldepthMap", 3);
		}

		m->tm->draw(view, projection);

		glUseProgram(0);
	}

	glBindVertexArray(0);


	glfwSwapBuffers(eng.window);
	glfwPollEvents();
	if (err != GL_NO_ERROR)
		cout << gluErrorString(err);
}
void scene::generate_shaders()
{
	programs.clear();

	if (plights.size() > 0)
	{
		program* sp = new program();

		//frag shader
		shader fsh(GL_FRAGMENT_SHADER);
		if (fsh.compile(plights[0].generate_fragshader(this)) == GL_FALSE)
			cout << "frag failed";

		if (sp->attach_shader(&fsh) != GL_NO_ERROR)
			cout << "frag attach failed";

		//vert shader
		shader vsh(GL_VERTEX_SHADER);
		if (vsh.compile(plights[0].generate_vertshader(this)) == GL_FALSE)
			cout << "vertex failed";

		if (sp->attach_shader(&vsh) != GL_NO_ERROR)
			cout << "vert attach failed";

		if (sp->link() != GL_NO_ERROR)
			cout << "link failed";

		programs["pointlight"] = sp;
	}
	if (plights.size() > 0)
	{
		program* sp = new program();


		//frag shader
		ifstream frag("./data/pointshadowdepth.frag");
		string source = "";
		string line = "";

		if (frag.is_open())
			while (std::getline(frag, line))
				source += line + "\n";
		else
			cout << "File is not open";

		shader fsh(GL_FRAGMENT_SHADER);
		if (fsh.compile(source) == GL_FALSE)
			cout << "frag failed";

		source = line = "";

		if (sp->attach_shader(&fsh) != GL_NO_ERROR)
			cout << "frag attach failed";

		//vert shader
		ifstream vertex("./data/pointshadowdepth.vert");

		if (vertex.is_open())
			while (std::getline(vertex, line))
				source += line + "\n";
		else
			cout << "File is not open";

		shader vsh(GL_VERTEX_SHADER);
		if (vsh.compile(source) == GL_FALSE)
			cout << "vertex failed";

		source = line = "";

		if (sp->attach_shader(&vsh) != GL_NO_ERROR)
			cout << "vert attach failed";


		//geo shader
		ifstream geo("./data/pointshadowdepth.geo");
		if (geo.is_open())
		{
			while (std::getline(geo, line))
				source += line + "\n";


			shader gsh(GL_GEOMETRY_SHADER);
			if (gsh.compile(source) == GL_FALSE)
				cout << "geo failed";

			source = line = "";

			if (sp->attach_shader(&gsh) != GL_NO_ERROR)
				cout << "geo attach failed";
		}


		if (sp->link() != GL_NO_ERROR)
			cout << "link failed";

		programs["pointshadowdepth"] = sp;
	}
	if (enable_dirlight)
	{
		program* sp = new program();

		//frag shader
		shader fsh(GL_FRAGMENT_SHADER);
		if (fsh.compile(dirlight.generate_fragshader(this)) == GL_FALSE)
			cout << "frag failed";

		if (sp->attach_shader(&fsh) != GL_NO_ERROR)
			cout << "frag attach failed";

		//vert shader
		shader vsh(GL_VERTEX_SHADER);
		if (vsh.compile(dirlight.generate_vertshader(this)) == GL_FALSE)
			cout << "vertex failed";

		if (sp->attach_shader(&vsh) != GL_NO_ERROR)
			cout << "vert attach failed";


		if (sp->link() != GL_NO_ERROR)
			cout << "link failed";

		programs["dirlightshadowdepth"] = sp;
	}


	program* sp = new program();

	////geo shader
	//shader gsh(GL_GEOMETRY_SHADER);
	//if (gsh.compile(m->generate_geoshader(this)) == GL_FALSE)
	//	cout << "geo failed";

	//if (sp->attach_shader(&gsh) != GL_NO_ERROR)
	//	cout << "geo attach failed";

	shared_ptr<texturemesh> m;
	m.reset(new texturemesh());

	//frag shader
	shader fsh(GL_FRAGMENT_SHADER);
	if (fsh.compile(m->generate_fragshader(this)) == GL_FALSE)
		cout << "frag failed";

	if (sp->attach_shader(&fsh) != GL_NO_ERROR)
		cout << "frag attach failed";

	//vert shader
	shader vsh(GL_VERTEX_SHADER);
	if (vsh.compile(m->generate_vertshader(this)) == GL_FALSE)
		cout << "vertex failed";

	if (sp->attach_shader(&vsh) != GL_NO_ERROR)
		cout << "vert attach failed";

	if (sp->link() != GL_NO_ERROR)
		cout << "link failed";

	programs[m->spname()] = sp;
}