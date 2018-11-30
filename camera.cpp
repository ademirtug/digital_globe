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
float getAltitude(int mapzoom) {
	float height = 0;
	switch (mapzoom)
	{
	case 1:
		height = 18000000;
		break;
	case 2:
		height = 8000000;
		break;
	case 3:
		height = 4000000;
		break;
	case 4:
		height = 3000000;
		break;
	case 5:
		height = 2000000;
		break;
	case 6:
		height = 1000000;
		break;
	case 7:
		height = 500000;
		break;
	case 8:
		height = 250000;
		break;
	case 9:
		height = 100000;
		break;
	case 10:
		height = 50000;
		break;
	case 11:
		height = 25000;
		break;
	case 12:
		height = 13000;
		break;
	case 13:
		height = 6500;
		break;
	case 14:
		height = 3750;
		break;
	case 15:
		height = 1900;
		break;
	case 16:
		height = 800;
		break;
	case 17:
		height = 400;
		break;
	case 18:
		height = 200;
		break;
	case 19:
		height = 10;
		break;
	default:
		height = 50000000;
		break;
	}

	return height;
}


qball_camera::qball_camera()
{
	zoomlevel = 2;
	cameralevel = 2;
	currot.w = 1;
	currot.x = 0;
	currot.y = 0;
	currot.z = 0;
	cdist = getAltitude(zoomlevel);
}
qball_camera::~qball_camera()
{
}

glm::mat4 qball_camera::getview(double lat = 0, double lon = 0)
{
	glm::vec3 ecef = lla2ecef(lat, lon);
	double groundlevel = sqrt(pow(ecef.x, 2) + pow(ecef.y, 2) + pow(ecef.z, 2));
	cdist = groundlevel + getAltitude(zoomlevel);

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

		glm::quat newrot = glm::angleAxis(glm::radians(glm::length(glm::vec3(dx, dy, 0)) / (float)(2.1 * pow(2, zoomlevel - 1)) ), rotaxis);
		currot = newrot * currot;

		lastx = xpos;
		lasty = ypos;
	}
}
void qball_camera::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoomlevel += yoffset;
	zoomlevel = zoomlevel < 1 ? 1 : zoomlevel;
	zoomlevel = zoomlevel > 17 ? 17 : zoomlevel;
}
int qball_camera::getzoomlevel()
{
	return zoomlevel;
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