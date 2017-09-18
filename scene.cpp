#include "stdafx.h"


extern engine eng;

scene::scene()
{
	//bunu texture sýnýfýndan türeme shadow map yap
	shadow_w = 1024;
	shadow_h = 1024;
}

void scene::generate_shaders()
{
	if (plights.size() > 0)
	{
		program* sp = new program();

		//frag shader
		ifstream frag("./data/pointlight.frag");
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
		ifstream vertex("./data/pointlight.vert");

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

	for (auto m : meshes)
	{
		if (dynamic_cast<texturemesh*>(m) != 0)
		{
			if (programs.find("texturemesh") != programs.end())
				continue;

			program* sp = new program();


			//frag shader
			ifstream frag("./data/texturemesh.frag");
			string source = "";
			string line = "";

			if (frag.is_open())
				while (std::getline(frag, line))
					source += line + "\n";
			else
				cout << "File is not open";

			//source = source.replace(source.find("NR_POINT_LIGHTS"), string("NR_POINT_LIGHTS").size(),
			//string("NR_POINT_LIGHTS ") + to_string(plights.size()));

			shader fsh(GL_FRAGMENT_SHADER);
			if (fsh.compile(source) == GL_FALSE)
				cout << "frag failed";

			source = line = "";

			if (sp->attach_shader(&fsh) != GL_NO_ERROR)
				cout << "frag attach failed";

			//vert shader
			ifstream vertex("./data/texturemesh.vert");

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

			if (sp->link() != GL_NO_ERROR)
				cout << "link failed";

			programs["texturemesh"] = sp;
		}
	}

	glGenFramebuffers(1, &fbo_depth_map);
	glGenTextures(1, &vbo_depthcube_map);

	glBindTexture(GL_TEXTURE_CUBE_MAP, vbo_depthcube_map);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadow_w, shadow_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth_map);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, vbo_depthcube_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void scene::draw()
{
	GLenum err = 0;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	float near_plane = 1.0f;
	float far_plane = 45.0f;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)1024 / (float)1024, near_plane, far_plane);
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj * glm::lookAt(plights[0].position, plights[0].position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(plights[0].position, plights[0].position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(plights[0].position, plights[0].position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(plights[0].position, plights[0].position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(plights[0].position, plights[0].position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(plights[0].position, plights[0].position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));


	glViewport(0, 0, 1024, 1024);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth_map);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(programs["pointshadowdepth"]->get_id());

	for (unsigned int i = 0; i < 6; ++i)
		programs["pointshadowdepth"]->setuniform("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);

	programs["pointshadowdepth"]->setuniform("far_plane", far_plane);
	programs["pointshadowdepth"]->setuniform("lightPos", plights[0].position);


	for (auto m : meshes)
	{
		programs["pointshadowdepth"]->setuniform("model", m->model());
		m->shadowdraw();
	}
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//NORMAL ÇÝZÝM
	glViewport(0, 0, 1024, 768);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 view = eng.cam->getview();
	glm::vec3 cameraPos = eng.cam->getpos();

	glUseProgram(programs["texturemesh"]->get_id());
	programs["texturemesh"]->setuniform("diffuseTexture", 0);
	programs["texturemesh"]->setuniform("depthMap", 1);


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

	glBindVertexArray(vao_mesh_id);
	for (auto m : meshes)
	{
		glUseProgram(programs[m->spname()]->get_id());
		programs[m->spname()]->setuniform("model", glm::translate(glm::mat4(1.0f), m->position));
		programs[m->spname()]->setuniform("view", view);
		programs[m->spname()]->setuniform("projection", projection);
		
		programs[m->spname()]->setuniform("viewPos", cameraPos);
		programs[m->spname()]->setuniform("lightPos", plights[0].position);
		programs[m->spname()]->setuniform("shadows", 1);
		programs[m->spname()]->setuniform("far_plane", far_plane);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, vbo_depthcube_map);
		m->draw(view, projection);

		glUseProgram(0);
	}

	glBindVertexArray(0);


	glfwSwapBuffers(eng.window);
	glfwPollEvents();
	if (err != GL_NO_ERROR)
		cout << gluErrorString(err);
}