#ifndef __TEXTURE_H__
#define __TEXTURE_H__


class texture
{
public:
	texture();
	texture(const std::string& filename);
	void load(const std::string& filename);


	GLuint vboid_texture;
};







#endif//__TEXTURE_H__ 