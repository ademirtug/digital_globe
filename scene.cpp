#include "stdafx.h"


extern engine eng;

scene::scene()
{
	//bunu texture sýnýfýndan türeme shadow map yap
	enable_dirlight = true;
	width = 1024.0f;
	height = 768.0f;
}


void scene::draw()
{
	GLenum err = 0;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//lock
	eng.sc->mxqueuedmeshes.lock();
	if (queuedmeshes.size() > 0)
	{
		for (auto tile : queuedmeshes)
		{
			tile->tm.reset(new texturemesh(tile->vertices, tile->normals, tile->uvs, string(tile->fname.begin(), tile->fname.end())));
			tile->tm->position = { 0.0, 0.0, 0.0 };
			eng.sc->meshes.push_back(tile->tm);
		}
		queuedmeshes.clear();
	}
	//unlock
	eng.sc->mxqueuedmeshes.unlock();




	if (enable_dirlight)
	{
		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, dirlight.sm.fbo_depthmap);
		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(programs["dirlightshadowdepth"]->get_id());
		programs["dirlightshadowdepth"]->setuniform("DirLightSpaceMatrix", dirlight.space);
		
		for (auto m : meshes)
		{
			programs["dirlightshadowdepth"]->setuniform("model", m->model());
			m->shadowdraw();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	//NORMAL ÇÝZÝM
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(45.0f, width/height, 100.0f, 50000000.0f);
	glm::mat4 view = eng.cam->getview();
	glm::vec3 cameraPos = eng.cam->getpos();
	

	glBindVertexArray(vao_mesh_id);

	for (auto m : meshes)
	{
		glUseProgram(programs[m->spname()]->get_id());
		programs[m->spname()]->setuniform("viewPos", cameraPos);

		if (enable_dirlight)
		{
			programs[m->spname()]->setuniform("dirLight.direction", dirlight.direction);
			programs[m->spname()]->setuniform("dirLight.ambient", dirlight.ambient);
			programs[m->spname()]->setuniform("dirLight.diffuse", dirlight.diffuse);
			programs[m->spname()]->setuniform("dirLight.specular", dirlight.specular);
			programs[m->spname()]->setuniform("DirLightSpaceMatrix", dirlight.space);
			
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, dirlight.sm.vboid_texture);

			programs[m->spname()]->setuniform("far_plane", dirlight.far_plane);
			programs[m->spname()]->setuniform("dldepthMap", 3);
		}
	
		m->draw(view, projection);

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



	//for (auto m : meshes)
	//{
	//	if (programs.find(m->spname()) == programs.end())
	//	{
	//		program* sp = new program();

	//		////geo shader
	//		//shader gsh(GL_GEOMETRY_SHADER);
	//		//if (gsh.compile(m->generate_geoshader(this)) == GL_FALSE)
	//		//	cout << "geo failed";

	//		//if (sp->attach_shader(&gsh) != GL_NO_ERROR)
	//		//	cout << "geo attach failed";


	//		//frag shader
	//		shader fsh(GL_FRAGMENT_SHADER);
	//		if (fsh.compile(m->generate_fragshader(this)) == GL_FALSE)
	//			cout << "frag failed";

	//		if (sp->attach_shader(&fsh) != GL_NO_ERROR)
	//			cout << "frag attach failed";

	//		//vert shader
	//		shader vsh(GL_VERTEX_SHADER);
	//		if (vsh.compile(m->generate_vertshader(this)) == GL_FALSE)
	//			cout << "vertex failed";

	//		if (sp->attach_shader(&vsh) != GL_NO_ERROR)
	//			cout << "vert attach failed";

	//		if (sp->link() != GL_NO_ERROR)
	//			cout << "link failed";

	//		programs[m->spname()] = sp;
	//	}
	//}
}