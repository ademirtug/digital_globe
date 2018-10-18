#ifndef __TEXTURE_H__
#define __TEXTURE_H__


class texture
{
public:
	texture();
	texture(const std::string& filename);
	virtual void load(const std::string& filename);
	virtual void unload();

	GLuint vboid_texture;
};


class bmp : public texture
{
public:
	bmp();
	bmp(const std::string& filename);
	virtual void load(const std::string& filename);
	virtual void load(unsigned char* data, int w, int h, int bitcount=32);
	virtual void unload();
};

class png : public texture
{
public:
	png();
	png(const std::string& filename);
	virtual void load(const std::string& filename);
};



class shadowmap : public texture
{
public:
	shadowmap();
	void init();

	int width, height;
	GLuint fbo_depthmap;
};


class shadowcubemap : public texture
{
public:
	shadowcubemap();
	void init();
	
	int width, height;
	GLuint fbo_depthmap;
};

#endif//__TEXTURE_H__ 