#include "stdafx.h"
#include "camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"
#include <math.h>

bool rdown = false;
double lastx, lasty;
double rx = 0;
double ry = 0;
double mydiff = 0.000001;
extern engine eng;


glm::mat4 rot_mat = glm::mat4(1.0f);

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

	return glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -cdist)) *rot_mat;
}
glm::vec3 arcball_camera::getpos()
{
	return inverse(getview())[3];
}
void arcball_camera::mouse_button_callback(GLFWwindow* window, int button,	int action, int mods)
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
		drot_matrix = glm::rotate(drot_matrix, dx / 300, glm::vec3(0.0f, 1.0f, 0.0f));
		drot_matrix = glm::rotate(drot_matrix, dy / 300, glm::vec3(1.0f, 0.0f, 0.0f));

		rot_mat = drot_matrix * rot_mat;

		lastx = newx;
		lasty = newy;
	}
}
void arcball_camera::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	mydiff = -yoffset;
}

float toRadians(float degree)
{
	return degree * (glm::pi<float>() / 180);
}
float getAltitude(float mapzoom) {
	//this equation is a transformation of the angular size equation solving for D. See: http://en.wikipedia.org/wiki/Forced_perspective
	float googleearthaltitude;
	float firstPartOfEq = (float)(.05 * ((591657550.5 / (pow(2, (mapzoom -2)))) / 2));//amount displayed is .05 meters and map scale =591657550.5/(Math.pow(2,(mapzoom-1))))
																					   //this bit ^ essentially gets the h value in the angular size eq then divides it by 2
	googleearthaltitude = (firstPartOfEq) * ((float)(cos(toRadians(85.362 / 2))) / (float)(sin(toRadians(85.362 / 2))));//85.362 is angle which google maps displays on a 5cm wide screen
	return googleearthaltitude;
}


qball_camera::qball_camera()
{
	zoomlevel = 3;
	cdist = getAltitude(zoomlevel);
}
qball_camera::~qball_camera()
{

}

glm::quat currot(1, 0, 0, 0);
glm::mat4 qball_camera::getview()
{
	cdist = 6378137.0f + getAltitude(zoomlevel);

	return (glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -cdist)) * glm::mat4_cast(currot));
}

glm::vec3 qball_camera::getpos()
{
	return inverse(getview())[3];
}


void qball_camera::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		rdown = true;
		glfwGetCursorPos(window, &lastx, &lasty);

	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		rdown = false;
	}
}
void qball_camera::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (rdown)
	{
		float dx = xpos - lastx;
		float dy = ypos - lasty;

		if (dx == 0 && dy == 0)
			return;
		
		glm::vec3 mouseaxis = glm::normalize(glm::vec3(-dx, dy, 0));
		glm::vec3 up = mouseaxis;

		glm::mat4 temp_rotmat(1);
		temp_rotmat = glm::rotate(temp_rotmat, 45.0f, glm::vec3(0.0, 0.0, 1.0));
		up = glm::vec3(temp_rotmat * glm::vec4(up, 1.0f));

		glm::vec3 rightaxis = glm::normalize(glm::cross(mouseaxis, up));
		glm::vec3 rotaxis = glm::normalize(glm::cross(mouseaxis, rightaxis));

		glm::quat newrot = glm::angleAxis(glm::radians(glm::length(glm::vec3(dx, dy, 0))/((10*log2f(zoomlevel)))), rotaxis);
		currot = newrot * currot;

		lastx = xpos;
		lasty = ypos;
	}
}
void qball_camera::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoomlevel += yoffset;
	zoomlevel = zoomlevel < 2 ? 2 : zoomlevel;
	zoomlevel = zoomlevel > 19 ? 19 : zoomlevel;

	
}



eu_orbit_camera::eu_orbit_camera()
{
	cdist = 15;
}
eu_orbit_camera::~eu_orbit_camera()
{}
glm::mat4 eu_orbit_camera::getview()
{
	cdist += mydiff / 5;
	mydiff = 0;
	return glm::translate(glm::mat4(), glm::vec3(0, 0, -cdist)) * glm::eulerAngleXY(-(float)ry / 100, (float)rx / 100);
}
glm::vec3 eu_orbit_camera::getpos()
{
	return inverse(getview())[3];
}
void eu_orbit_camera::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
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
void eu_orbit_camera::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (rdown)
	{
		ry += lasty - ypos;

		bool n = ry < 0 ? true : false;
		ry = abs(ry);

		for (; true;)
		{
			if (ry >= 2 * glm::pi<float>() * 100)
				ry -= 2 * glm::pi<float>() * 100;
			else break;
		}
		rx -= lastx - xpos;
		ry = n ? -ry : ry;

		lastx = xpos;
		lasty = ypos;
	}
}
void eu_orbit_camera::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	mydiff = 10 * -yoffset;
}