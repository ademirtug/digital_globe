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

	glGenVertexArrays(1, &sc.vao_mesh_id);
	glGenVertexArrays(1, &sc.vao_lights_id);

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
	

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		GLenum err = 0;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 viewModel = inverse(cam->getview());
		glm::vec3 cameraPos(viewModel[3]);


		glBindVertexArray(sc.vao_lights_id);
		for (auto l : sc.plights)
		{
			//draw light sources
			glm::mat4 LModel = glm::mat4();
			LModel = glm::translate(LModel, l.position);
			LModel = glm::scale(LModel, glm::vec3(0.2f));


			glUseProgram(programs["lightsource"]->get_id());
			programs["lightsource"]->setuniform("model", LModel);
			programs["lightsource"]->setuniform("view", cam->getview());
			programs["lightsource"]->setuniform("projection", Projection);

			l.draw();

			glUseProgram(0);
		}
		glBindVertexArray(0);


		/////draw scene meshes
		glBindVertexArray(sc.vao_mesh_id);
		for (auto m : sc.meshes)
		{
			glUseProgram(programs[m->spname()]->get_id());
			
			programs[m->spname()]->setuniform("model", glm::translate(glm::mat4(1.0f), m->position));
			programs[m->spname()]->setuniform("view", cam->getview());
			programs[m->spname()]->setuniform("projection", Projection);
			programs[m->spname()]->setuniform("viewPos", cameraPos);


			programs[m->spname()]->setuniform("dirLight.direction", sc.dirlight.direction);
			programs[m->spname()]->setuniform("dirLight.ambient", sc.dirlight.ambient);
			programs[m->spname()]->setuniform("dirLight.diffuse", sc.dirlight.diffuse);
			programs[m->spname()]->setuniform("dirLight.specular", sc.dirlight.specular);

			for (auto l : sc.plights)
			{
				programs[m->spname()]->setuniform("pointLights[0].position", l.position);
				programs[m->spname()]->setuniform("pointLights[0].ambient",  l.ambient);
				programs[m->spname()]->setuniform("pointLights[0].diffuse", l.diffuse);
				programs[m->spname()]->setuniform("pointLights[0].specular", l.specular);
				programs[m->spname()]->setuniform("pointLights[0].constant", l.constant);
				programs[m->spname()]->setuniform("pointLights[0].linear", l.linear);
				programs[m->spname()]->setuniform("pointLights[0].quadratic", l.quadratic);
			}

			programs[m->spname()]->setuniform("material.specular", 1);
			programs[m->spname()]->setuniform("material.shininess",64.0f);
			programs[m->spname()]->setuniform("material.diffuse", 0);
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