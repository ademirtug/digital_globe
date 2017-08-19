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


class bitmap : public texture
{};


class shadowmap : public texture
{};






#endif//__TEXTURE_H__ 