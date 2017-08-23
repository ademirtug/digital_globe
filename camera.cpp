#include "stdafx.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <math.h>

double lrx = 0;

bool rdown = false;
double lastx, lasty;
double rx = 0;
double ry = 0;
double mydiff = 0.000001;
extern engine eng;
glm::quat currot(1.0, 0, 0, 0);

arcball_camera::arcball_camera()
{
	cdist = 5;
}
arcball_camera::~arcball_camera()
{
}
glm::mat4 arcball_camera::getview()
{ 
	cdist += mydiff / 5;
	mydiff = 0;

	glm::mat4 ViewTranslate = glm::translate(
		glm::mat4(1.0f),
		glm::vec3(0, 0, -cdist) );
	
	glm::mat4 ViewRotateX = glm::rotate(
		ViewTranslate,
		(float)ry / 100,
		glm::vec3(-1.0f, 0.0f, 0.0f)
	);
	
	glm::mat4 View = glm::rotate(
		ViewRotateX,
		(float)rx / 100,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	return View;
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
		currot = glm::angleAxis(glm::radians((float)ry / 100), glm::vec3(1.0f, 0.0f, 0.0f));
	}
}
void arcball_camera::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (rdown)
	{
		ry += lasty - ypos;

		bool n = ry < 0 ? true : false;
		ry = abs(ry);


		for (;true;)
		{
			if (ry >= 2 * glm::pi<float>() * 100)
				ry -= 2 * glm::pi<float>() * 100;
			else break;
		}

		//if (ry > PI * 100)
			rx -= lastx - xpos;
		//else
		//	rx -= lastx - xpos;
	
		ry = n ? -ry : ry;

		lastx = xpos;
		lasty = ypos;


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


glm::mat4 qball_camera::getview()
{
	cdist += mydiff / 5;
	mydiff = 0;

	glm::quat xrot = glm::angleAxis(glm::radians((float)ry / 10), glm::vec3(1.0f, -3.0, -cdist));
	glm::mat4 viewm = glm::mat4_cast(xrot);
	viewm = glm::translate(viewm, glm::vec3(0, -3, -cdist));

	//viewm = glm::lookAt(glm::vec3(0, 0, -cdist), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0, 1, 0));


	return viewm;
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
		glfwGetCursorPos(window, &lastx, &lasty);
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
		ry += lasty - ypos;

		bool n = ry < 0 ? true : false;
		ry = abs(ry);


		//for (; true;)
		//{
		//	if (ry >= 2 * glm::pi<float>() * 100)
		//		ry -= 2 * glm::pi<float>() * 100;
		//	else break;
		//}

		rx -= lastx - xpos;
		ry = n ? -ry : ry;

		lastx = xpos;
		lasty = ypos;
	}
}
void qball_camera::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	mydiff = -yoffset;
}