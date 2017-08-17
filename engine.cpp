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
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	sc = new scene();
	glGenVertexArrays(1, &sc->vao_mesh_id);
	glGenVertexArrays(1, &sc->vao_lights_id);

	return true;
}
void engine::load_shaders(std::string name)
{
	program* sp = new program();
	
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
		cout << "attach failed";

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
		cout << "attach failed";

	if (sp->link() != GL_NO_ERROR)
		cout << "link failed";

	programs[name] = sp;
}


void engine::run()
{
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glGenFramebuffers(1, &sc->fbo_depth_map);
	glGenTextures(1, &sc->vbo_depth_map);

	glBindTexture(GL_TEXTURE_2D, sc->vbo_depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, sc->shadow_w, sc->shadow_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	
	glBindFramebuffer(GL_FRAMEBUFFER, sc->fbo_depth_map);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sc->vbo_depth_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	glUseProgram(programs["shadowmapping"]->get_id());	
	programs["shadowmapping"]->setuniform("diffuseTexture", 0);
	programs["shadowmapping"]->setuniform("shadowMap", 1);


	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		GLenum err = 0;
		glm::mat4 viewModel = inverse(cam->getview());
		glm::vec3 cameraPos(viewModel[3]);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//// 1. render depth of scene to texture (from light's perspective)
		//// --------------------------------------------------------------
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 7.5f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(sc->plights[0].position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		
		// render scene from light's point of view
		glUseProgram(programs["depth"]->get_id());
		programs["depth"]->setuniform("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, sc->shadow_w, sc->shadow_h);
		glBindFramebuffer(GL_FRAMEBUFFER,sc->fbo_depth_map);
		glClear(GL_DEPTH_BUFFER_BIT);


		/////draw scene meshes
		glBindVertexArray(sc->vao_mesh_id);
		for (auto m : sc->meshes)
		{
			programs["depth"]->setuniform("model", glm::translate(glm::mat4(1.0f), m->position));
			m->draw();
		}
		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, 1024, 768);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// 2. render scene as normal using the generated depth/shadow map  
		// --------------------------------------------------------------

		glBindVertexArray(sc->vao_lights_id);
		for (auto l : sc->plights)
		{
			//draw light sources
			glUseProgram(programs["lightsource"]->get_id());
			programs["lightsource"]->setuniform("model", glm::scale( glm::translate(glm::mat4(), l.position), glm::vec3(0.2f)));
			programs["lightsource"]->setuniform("view", cam->getview());
			programs["lightsource"]->setuniform("projection", Projection);

			l.draw();

			glUseProgram(0);
		}
		glBindVertexArray(0);


		/////draw scene meshes
		glBindVertexArray(sc->vao_mesh_id);
		for (auto m : sc->meshes)
		{
			glUseProgram(programs["shadowmapping"]->get_id());
			programs["shadowmapping"]->setuniform("model", glm::translate(glm::mat4(1.0f), m->position));
			programs["shadowmapping"]->setuniform("view", cam->getview());
			programs["shadowmapping"]->setuniform("projection", Projection);
			
			programs["shadowmapping"]->setuniform("viewPos", cameraPos);
			programs["shadowmapping"]->setuniform("lightPos", sc->plights[0].position );
			programs["shadowmapping"]->setuniform("lightSpaceMatrix", lightSpaceMatrix);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,sc->vbo_depth_map);
			
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