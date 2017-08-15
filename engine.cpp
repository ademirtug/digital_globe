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
	
	glUseProgram(programs["shadow"]->get_id());
	programs["shadow"]->setuniform("depthMap", 0);


	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		GLenum err = 0;
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 viewModel = inverse(cam->getview());
		glm::vec3 cameraPos(viewModel[3]);


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
			glUseProgram(programs[m->spname()]->get_id());


			programs[m->spname()]->setuniform("model", glm::translate(glm::mat4(1.0f), m->position));
			programs[m->spname()]->setuniform("view",  cam->getview());
			programs[m->spname()]->setuniform("projection", Projection);
			programs[m->spname()]->setuniform("viewPos", cameraPos);

			programs[m->spname()]->setuniform("dirLight.direction", sc->dirlight.direction);
			programs[m->spname()]->setuniform("dirLight.ambient", sc->dirlight.ambient);
			programs[m->spname()]->setuniform("dirLight.diffuse", sc->dirlight.diffuse);
			programs[m->spname()]->setuniform("dirLight.specular", sc->dirlight.specular);

			if (m->spname() == "uv")
			{
				for (unsigned int i = 0; i < sc->plights.size(); ++i)
				{
					programs[m->spname()]->setuniform("pointLights["+ std::to_string(i) + "].position", sc->plights[i].position);
					programs[m->spname()]->setuniform("pointLights[" + std::to_string(i) + "].ambient", sc->plights[i].ambient);
					programs[m->spname()]->setuniform("pointLights[" + std::to_string(i) + "].diffuse", sc->plights[i].diffuse);
					programs[m->spname()]->setuniform("pointLights[" + std::to_string(i) + "].specular", sc->plights[i].specular);
					programs[m->spname()]->setuniform("pointLights[" + std::to_string(i) + "].constant", sc->plights[i].constant);
					programs[m->spname()]->setuniform("pointLights[" + std::to_string(i) + "].linear", sc->plights[i].linear);
					programs[m->spname()]->setuniform("pointLights[" + std::to_string(i) + "].quadratic", sc->plights[i].quadratic);
				}
			}
			else
			{
				programs[m->spname()]->setuniform("light.position", sc->plights[0].position);
				programs[m->spname()]->setuniform("light.ambient", { 0.1f, 0.1f, 0.1f });
				programs[m->spname()]->setuniform("light.diffuse", { 0.7f, 0.7f, 0.7f });
				programs[m->spname()]->setuniform("light.specular", { 1.0f, 1.0f, 1.0f });
			}

			programs[m->spname()]->setuniform("material.specular", 1.0f);
			programs[m->spname()]->setuniform("material.shininess",32.0f);
			programs[m->spname()]->setuniform("material.diffuse", 0.5f);
			programs[m->spname()]->setuniform("material.color", {1.0f, 0.5f, 0.31f});


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

	
	glGenTextures(1, &vbo_depth_map);
	glBindTexture(GL_TEXTURE_2D, vbo_depth_map);
	
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_w, shadow_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	glGenFramebuffers(1, &fbo_depth_map);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth_map);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, vbo_depth_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}