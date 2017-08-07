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
		glm::vec3 lightPos(2.0f, 2.0f, -2.0f);
		glm::mat4 viewModel = inverse(cam->getview());
		glm::vec3 cameraPos(viewModel[3]);

		/////draw scene meshes
		glBindVertexArray(sc.vao_mesh_id);
		for (auto m : sc.meshes)
		{
			glUseProgram(programs[m->spname()]->get_id());
			

			programs[m->spname()]->setuniform("model", glm::translate(glm::mat4(1.0f), m->position));
			programs[m->spname()]->setuniform("view", cam->getview());
			programs[m->spname()]->setuniform("projection", Projection);

			programs[m->spname()]->setuniform("light.position", lightPos);
			programs[m->spname()]->setuniform("light.ambient", {0.1f, 0.1f, 0.1f});
			programs[m->spname()]->setuniform("light.diffuse", { 0.7f, 0.7f, 0.7f });
			programs[m->spname()]->setuniform("light.specular", { 1.0f, 1.0f, 1.0f });


			programs[m->spname()]->setuniform("light.constant", 1.0f);
			programs[m->spname()]->setuniform("light.linear", 0.09f);
			programs[m->spname()]->setuniform("light.quadratic", 0.032f);


			programs[m->spname()]->setuniform("material.specular", 1);
			programs[m->spname()]->setuniform("material.shininess",64.0f);
			programs[m->spname()]->setuniform("material.diffuse", 0);
			programs[m->spname()]->setuniform("material.color", {1.0f, 0.5f, 0.31f});

			programs[m->spname()]->setuniform("viewPos", cameraPos);

			m->draw();

			glUseProgram(0);
		}
		glBindVertexArray(0);

		////////////draw light sources
		//////////glm::mat4 LModel = glm::mat4();
		//////////LModel = glm::translate(LModel, lightPos);
		//////////LModel = glm::scale(LModel, glm::vec3(0.2f));

		//////////glBindVertexArray(sc.vao_lights_id);
		//////////for (auto l : lights.meshes)
		//////////{
		//////////	glUseProgram(programs[l->spname()]->get_id());
		//////////	programs[l->spname()]->setuniform("model", LModel);
		//////////	programs[l->spname()]->setuniform("view", cam->getview());
		//////////	programs[l->spname()]->setuniform("projection", Projection);

		//////////	l->draw();

		//////////	glUseProgram(0);
		//////////}
		//////////glBindVertexArray(0);


		glfwSwapBuffers(window);
		glfwPollEvents();
		if (err != GL_NO_ERROR)
			cout << gluErrorString(err);
	}
	glfwTerminate();


	//lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	//lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	//lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	//lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
	//// point light 1
	//lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
	//lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	//lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	//lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	//lightingShader.setFloat("pointLights[0].constant", 1.0f);
	//lightingShader.setFloat("pointLights[0].linear", 0.09);
	//lightingShader.setFloat("pointLights[0].quadratic", 0.032);
	//// point light 2
	//lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
	//lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	//lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	//lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	//lightingShader.setFloat("pointLights[1].constant", 1.0f);
	//lightingShader.setFloat("pointLights[1].linear", 0.09);
	//lightingShader.setFloat("pointLights[1].quadratic", 0.032);
	//// point light 3
	//lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
	//lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	//lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	//lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	//lightingShader.setFloat("pointLights[2].constant", 1.0f);
	//lightingShader.setFloat("pointLights[2].linear", 0.09);
	//lightingShader.setFloat("pointLights[2].quadratic", 0.032);
	//// point light 4
	//lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
	//lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	//lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	//lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	//lightingShader.setFloat("pointLights[3].constant", 1.0f);
	//lightingShader.setFloat("pointLights[3].linear", 0.09);
	//lightingShader.setFloat("pointLights[3].quadratic", 0.032);
	//// spotLight
	//lightingShader.setVec3("spotLight.position", camera.Position);
	//lightingShader.setVec3("spotLight.direction", camera.Front);
	//lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	//lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	//lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	//lightingShader.setFloat("spotLight.constant", 1.0f);
	//lightingShader.setFloat("spotLight.linear", 0.09);
	//lightingShader.setFloat("spotLight.quadratic", 0.032);
	//lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	//lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}