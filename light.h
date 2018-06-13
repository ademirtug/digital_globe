#ifndef __LIGHT_H__
#define __LIGHT_H__


#include "stdafx.h"



class ilight
{
public:
	virtual void shine() {};
	glm::vec3 ambient, diffuse, specular, color;
};

class pointlight : public ilight, public imesh
{
protected:
	virtual void init();
public:

	pointlight();
	pointlight(const std::string& filename);
	pointlight(vector<glm::vec3> _vertices);

	void draw();
	virtual std::string spname() { return "standartlight"; };
	void loadobj(const std::string& filename);
	string generate_fragshader(scene* sc);
	string generate_vertshader(scene* sc);
	std::vector<glm::mat4> getshadowtransforms();

	GLfloat constant, linear, quadratic;
	shadowcubemap sm;
	float near_plane, far_plane;

};


class directionallight : public ilight
{
protected:
	virtual void init() {};
public:
	directionallight();

	string generate_fragshader(scene* sc);
	string generate_vertshader(scene* sc);

	float near_plane, far_plane;
	glm::vec3 direction;
	shadowmap sm;
	glm::mat4 projection, space, view;
};




#endif//__LIGHT_H__
