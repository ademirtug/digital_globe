#include "stdafx.h"


using namespace std;
engine eng;



void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	eng.cam->mouse_button_callback(window, button, action, mods);
}
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	eng.cam->cursor_pos_callback(window, xpos, ypos);
}
void mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	eng.cam->mouse_wheel_callback(window, xoffset, yoffset);
}

engine::engine()
{
}
engine::~engine()
{
	//programlar listesini delete yap
}

bool engine::init(int width, int height)
{
	if (!glfwInit())
		return false;

	window = glfwCreateWindow(width, height, "3d_engine", NULL, NULL);
	if (window == NULL) 
		return false;

	glfwMakeContextCurrent(window);

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetScrollCallback(window, mouse_wheel_callback);
	
	if (glewInit() != GLEW_OK)
		return false;
	
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	//glEnable(GL_CULL_FACE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	sc = new scene();
	glGenVertexArrays(1, &sc->vao_mesh_id);
	glGenVertexArrays(1, &sc->vao_lights_id);

	maxfps = 25;

	return true;
}
void engine::load_shaders(std::string name)
{
	program* sp = new program();
	

	//frag shader
	ifstream frag("./data/" + name + ".frag");
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
	ifstream vertex("./data/" + name + ".vert");

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
	ifstream geo("./data/" + name + ".geo");
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

	programs[name] = sp;
}


void engine::run()
{
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	
	
	glGenFramebuffers(1, &sc->fbo_depth_map);
	glGenTextures(1, &sc->vbo_depthcube_map);

	glBindTexture(GL_TEXTURE_CUBE_MAP, sc->vbo_depthcube_map);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, sc->shadow_w, sc->shadow_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glBindFramebuffer(GL_FRAMEBUFFER, sc->fbo_depth_map);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sc->vbo_depthcube_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	glUseProgram(programs["pointshadow"]->get_id());
	programs["pointshadow"]->setuniform("diffuseTexture", 0);
	programs["pointshadow"]->setuniform("depthMap", 1);

	double timer = 0;
	double timer2 = glfwGetTime();
	int fps = 0;

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		double diff = glfwGetTime() - timer;

		if (diff > 0 /*(1.0f / maxfps)*/ )
		{
			timer = glfwGetTime();
			fps++;
			if ((glfwGetTime() - timer2) > 1.0)
			{
				std::stringstream ss;
				ss << "3d_engine - fps: " << fps;

				glfwSetWindowTitle(window, ss.str().c_str());
				timer2 = glfwGetTime();
				fps = 0;
			}
		}
		else
			continue;

		GLenum err = 0;
		glm::mat4 view = cam->getview();
		glm::mat4 viewModel = inverse(view);
		glm::vec3 cameraPos(viewModel[3]);
		sc->plights[0].position.z = sin(glfwGetTime() * 0.5) * 3.0;


		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		float near_plane = 1.0f;
		float far_plane = 45.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)1024 / (float)1024, near_plane, far_plane);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(sc->plights[0].position, sc->plights[0].position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(sc->plights[0].position, sc->plights[0].position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(sc->plights[0].position, sc->plights[0].position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(sc->plights[0].position, sc->plights[0].position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(sc->plights[0].position, sc->plights[0].position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(sc->plights[0].position, sc->plights[0].position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));


		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, sc->fbo_depth_map);
		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(programs["pointshadowdepth"]->get_id());
		
		for (unsigned int i = 0; i < 6; ++i)
			programs["pointshadowdepth"]->setuniform("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);

		programs["pointshadowdepth"]->setuniform("far_plane", far_plane);
		programs["pointshadowdepth"]->setuniform("lightPos", sc->plights[0].position);
		programs["pointshadowdepth"]->setuniform("reverse_normals", 0);

		for (auto m : sc->meshes)
		{
			programs["pointshadowdepth"]->setuniform("model", glm::translate(glm::mat4(1.0f), m->position));
			m->draw();
		}
		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glViewport(0, 0, 1024, 768);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glBindVertexArray(sc->vao_lights_id);
		for (auto l : sc->plights)
		{
			glUseProgram(programs["lightsource"]->get_id());
			programs["lightsource"]->setuniform("model", glm::scale(glm::translate(glm::mat4(), l.position), glm::vec3(0.2f)));
			programs["lightsource"]->setuniform("view", view);
			programs["lightsource"]->setuniform("projection", Projection);

			l.draw();

			glUseProgram(0);
		}
		glBindVertexArray(0);


		glBindVertexArray(sc->vao_mesh_id);
		for (auto m : sc->meshes)
		{
			glUseProgram(programs["pointshadow"]->get_id());
			programs["pointshadow"]->setuniform("model", glm::translate(glm::mat4(1.0f), m->position));
			programs["pointshadow"]->setuniform("view", cam->getview());
			programs["pointshadow"]->setuniform("projection", Projection);
			
			programs["pointshadow"]->setuniform("viewPos", cameraPos);
			programs["pointshadow"]->setuniform("lightPos", sc->plights[0].position );
			programs["pointshadow"]->setuniform("shadows", 1);
			programs["pointshadow"]->setuniform("far_plane", far_plane);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, sc->vbo_depthcube_map);
			
			m->draw();

			glUseProgram(0);
		}
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
		if (err != GL_NO_ERROR)
			cout << gluErrorString(err);
	}
	glfwTerminate();
}



scene::scene()
{
	//bunu texture sınıfından türeme shadow map yap
	shadow_w = 1024;
	shadow_h = 1024;
}