#include "stdafx.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <math.h>

double lrx = 0;
int counter = 1;

bool rdown = false;
double lastx, lasty;
double rx = 0;
double ry = 0;
double mydiff = 0;
extern engine eng;

arcball_camera::arcball_camera()
{
	cdist = 5;
}

arcball_camera::~arcball_camera()
{}


glm::mat4 arcball_camera::getview()
{ 
	cdist += mydiff / 5;
	mydiff = 0;

	counter++;
	//if (counter % 1000 == 0)
	//	cout << "pitch:" << ry/100 << endl << "yaw: " << rx/100<<endl;

	//return glm::lookAt(eng.sc->plights[0].position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	return glm::translate(glm::mat4(), glm::vec3(0, -2, -cdist)) * glm::eulerAngleXY(-(float)ry/100, (float)rx/100);
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