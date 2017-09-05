#include "stdafx.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <math.h>

bool rdown = false;
double lastx, lasty, xs, ys, xe, ye;
double rx = 0;
double ry = 0;
double mydiff = 0.000001;
extern engine eng;


glm::mat4 rot_mat = glm::mat4(1.0f);

arcball_camera::arcball_camera()
{
	cdist = 25;
}
arcball_camera::~arcball_camera()
{
}
glm::mat4 arcball_camera::getview()
{
	cdist += mydiff / 5;
	mydiff = 0;


	glm::mat4 r = glm::mat4(1.0f);

	r = glm::translate(r, glm::vec3(0, 0, -cdist));
	
	r = r*rot_mat;
	return r;
	
}

glm::vec3 arcball_camera::getpos()
{
	return inverse(getview())[3];
}

void arcball_camera::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		rdown = true;
		glfwGetCursorPos(window, &lastx, &lasty);
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		rdown = false;
	}
}
void arcball_camera::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (rdown)
	{
		float newx = xpos;
		float newy = ypos;
		float dx = newx - lastx;
		float dy = newy - lasty;

		glm::mat4 drot_matrix = glm::mat4(1.0f);
		drot_matrix = glm::rotate(drot_matrix, dx / 100, glm::vec3(0.0f, 1.0f, 0.0f));
		drot_matrix = glm::rotate(drot_matrix, dy / 100, glm::vec3(1.0f, 0.0f, 0.0f));

		rot_mat = drot_matrix * rot_mat;

		lastx = newx;
		lasty = newy;
	}
}
void arcball_camera::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	mydiff = -yoffset;
}











qball_camera::qball_camera()
{
	cdist = 5;
}
qball_camera::~qball_camera()
{
}

int ct = 0;

glm::mat4 qball_camera::getview()
{
	cdist += mydiff / 5;
	mydiff = 0;


	glm::vec3 campos(0, -3, -cdist);
	glm::vec3 camright = glm::normalize(glm::cross(glm::normalize(campos), glm::vec3(0, 1, 0)));
	glm::vec3 camup = glm::normalize(glm::cross(glm::normalize(campos), camright));


	glm::mat4 view = glm::translate(glm::mat4(1.0f), campos);

	glm::vec3 mouseaxis = glm::normalize(glm::vec3(xs - xe, ys - ye, 0));
	glm::vec3 rightaxis = glm::normalize(glm::cross(mouseaxis, camup));
	glm::vec3 rotaxis = glm::normalize(glm::cross(mouseaxis, rightaxis));


	if (ct > 1000)
	{
		ct = 0;
		cout << "x:" << rotaxis.x << " y:" << rotaxis.y << " z:" << rotaxis.z << endl;
	}
	else ct++;


	glm::quat xrot = glm::angleAxis(glm::radians(glm::length(glm::vec3(xe - xs, ye - ys, 0))), rotaxis);

	view = glm::mat4_cast(xrot);
	return view;
}

glm::vec3 qball_camera::getpos()
{
	return inverse(getview())[3];
}


void qball_camera::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		rdown = true;
		glfwGetCursorPos(window, &xs, &ys);

	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		rdown = false;
	}
}
void qball_camera::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (rdown)
	{
		glfwGetCursorPos(window, &xe, &ye);
	}
}
void qball_camera::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	mydiff = -yoffset;
}