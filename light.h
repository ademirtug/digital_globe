#ifndef __LIGHT_H__
#define __LIGHT_H__


#include "stdafx.h"



class ilight
{
public:
	virtual void shine() {};
	glm::vec3 ambient, diffuse, specular;
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
	GLfloat constant, linear, quadratic;
};


class directionallight : public ilight
{
protected:
	virtual void init() {};
public:
	directionallight();
	glm::vec3 direction;

};




#endif//__LIGHT_H__
