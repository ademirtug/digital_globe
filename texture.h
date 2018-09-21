<<<<<<< HEAD
#ifndef __TEXTURE_H__
#define __TEXTURE_H__


class texture
{
public:
	texture();
	texture(const std::string& filename);
	virtual void load(const std::string& filename);

	GLuint vboid_texture;
};


class bmp : public texture
{
public:
	bmp();
	bmp(const std::string& filename);
	virtual void load(const std::string& filename);
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

=======
#ifndef __TEXTURE_H__
#define __TEXTURE_H__


class texture
{
public:
	texture();
	texture(const std::string& filename);
	virtual void load(const std::string& filename);

	GLuint vboid_texture;
};


class bmp : public texture
{
public:
	bmp();
	bmp(const std::string& filename);
	virtual void load(const std::string& filename);
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

>>>>>>> f2830dff220a5c8305bb801d8123cc6495fb6be2
#endif//__TEXTURE_H__ 