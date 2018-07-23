#include "stdafx.h"


extern engine eng;

scene::scene()
{
	//bunu texture sýnýfýndan türeme shadow map yap
	enable_dirlight = true;
}


void scene::draw()
{
	GLenum err = 0;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


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

	if (plights.size() > 0)
	{
		glViewport(0, 0, plights[0].sm.width, plights[0].sm.height);
		glBindFramebuffer(GL_FRAMEBUFFER, plights[0].sm.fbo_depthmap);
		glClear(GL_DEPTH_BUFFER_BIT);

		std::vector<glm::mat4> shadowTransforms = plights[0].getshadowtransforms();


		glUseProgram(programs["pointshadowdepth"]->get_id());

		for (unsigned int i = 0; i < 6; ++i)
			programs["pointshadowdepth"]->setuniform("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);

		programs["pointshadowdepth"]->setuniform("far_plane", plights[0].far_plane);
		programs["pointshadowdepth"]->setuniform("lightPos", plights[0].position);


		for (auto m : meshes)
		{
			programs["pointshadowdepth"]->setuniform("model", m->model());
			m->shadowdraw();
		}

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	//NORMAL ÇÝZÝM
	glViewport(0, 0, 1024, 768);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 view = eng.cam->getview();
	glm::vec3 cameraPos = eng.cam->getpos();


	if (plights.size() > 0)
	{
		glBindVertexArray(vao_lights_id);
		for (auto l : plights)
		{
			glUseProgram(programs["pointlight"]->get_id());
			programs["pointlight"]->setuniform("model", l.model());
			programs["pointlight"]->setuniform("view", view);
			programs["pointlight"]->setuniform("projection", projection);

			l.draw();
			glUseProgram(0);
		}
		glBindVertexArray(0);


		plights[0].position.z = sin(glfwGetTime() * 0.5) * 3.0;
	}



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
	
		if (plights.size() > 0)
		{
			int c = 0;
			for (auto l : plights)
			{
				programs[m->spname()]->setuniform("pointLights[" + to_string(c) + "].color", l.color);
				programs[m->spname()]->setuniform("pointLights[" + to_string(c) + "].position", l.position);

				programs[m->spname()]->setuniform("pointLights[" + to_string(c) + "].ambient", l.ambient);
				programs[m->spname()]->setuniform("pointLights[" + to_string(c) + "].diffuse", l.diffuse);
				programs[m->spname()]->setuniform("pointLights[" + to_string(c) + "].specular", l.specular);

				programs[m->spname()]->setuniform("pointLights[" + to_string(c) + "].constant", l.constant);
				programs[m->spname()]->setuniform("pointLights[" + to_string(c) + "].linear", l.linear);
				programs[m->spname()]->setuniform("pointLights[" + to_string(c) + "].quadratic", l.quadratic);

				c++;
			}

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, plights[0].sm.vboid_texture);

			programs[m->spname()]->setuniform("far_plane", plights[0].far_plane);
			programs[m->spname()]->setuniform("pldepthMap", 3);
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
	for (auto m : meshes)
	{
		if (programs.find(m->spname()) == programs.end())
		{
			program* sp = new program();

			////geo shader
			//shader gsh(GL_GEOMETRY_SHADER);
			//if (gsh.compile(m->generate_geoshader(this)) == GL_FALSE)
			//	cout << "geo failed";

			//if (sp->attach_shader(&gsh) != GL_NO_ERROR)
			//	cout << "geo attach failed";


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
	}
}