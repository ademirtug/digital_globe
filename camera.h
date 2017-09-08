#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace std;

class camera
{
public:
	virtual glm::mat4 getview() = 0;
	virtual glm::vec3 getpos() = 0;
	virtual void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) = 0;
	virtual void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) = 0;
	virtual void mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset) = 0;

};



class arcball_camera : public camera
{
public:
	double cdist;
	arcball_camera();
	~arcball_camera();
	
	glm::mat4 getview();
	glm::vec3 getpos();
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
	void mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset);
};

class qball_camera : public camera
{
public:
	double cdist;
	qball_camera();
	~qball_camera();

	glm::mat4 getview();
	glm::vec3 getpos();
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
	void mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset);
};


class eu_orbit_camera : public camera
{
public:
	double cdist;
	eu_orbit_camera();
	~eu_orbit_camera();

	glm::mat4 getview();
	glm::vec3 getpos();
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
	void mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset);
};



#endif//__CAMERA_H__
