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

	maxfps = 25;

	return true;
}

void engine::run()
{
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


	//glUseProgram(sc->programs["pointshadow"]->get_id());
	//sc->programs["pointshadow"]->setuniform("diffuseTexture", 0);
	//sc->programs["pointshadow"]->setuniform("depthMap", 1);

	double timer = 0;
	double timer2 = glfwGetTime();
	int fps = 0;

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		double diff = glfwGetTime() - timer;

		if (diff > (1.0f / maxfps) )
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

		sc->draw();
	}
	glfwTerminate();
}


