 #include "stdafx.h"
#include "texture.h"


texture::texture()
{
	vboid_texture = 0;
}
texture::texture(const std::string& filename)
{
	vboid_texture = 0;
	load(filename);
}
void texture::load(const std::string& filename)
{
}

bmp::bmp()
{
	vboid_texture = 0;
}
bmp::bmp(const std::string& filename)
{
	vboid_texture = 0;
	load(filename);
}
void bmp::load(const std::string& filename)
{
	unsigned int w, h, datastart, imgsize;
	char* data;
	char* hdr = new char[54 + 1];

	fstream f;
	f.open(filename, std::fstream::in | std::fstream::binary);
	if (!f.is_open())
		return;

	f.read(hdr, 54);

	//sanity check yapalım
	if (hdr[0] != 'B' || hdr[1] != 'M')
		return;


	datastart = *((int*)&hdr[0x0A]);
	imgsize = *((int*)&hdr[0x22]);
	w = *((int*)&hdr[0x12]);
	h = *((int*)&hdr[0x16]);

	//sanity check devam
	imgsize = imgsize == 0 ? w* h * 3 : imgsize;
	datastart = datastart == 0 ? 54 : datastart;

	data = new char[imgsize];
	f.read(data, imgsize);

	f.close();


	glGenTextures(1, &vboid_texture);
	glBindTexture(GL_TEXTURE_2D, vboid_texture);

	//ekran kartına yükleyelim.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	//yükleme tamam ram üzerindekini silelim.
	delete[] data;

	//trilinear filtreleme standart komut grubu
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}




png::png()
{
	vboid_texture = 0;
}
png::png(const std::string& filename)
{
	vboid_texture = 0;
	load(filename);
}
void png::load(const std::string& filename)
{
	unsigned int w, h, datastart, imgsize;
	char* data;

	data = new char[imgsize];





	glGenTextures(1, &vboid_texture);
	glBindTexture(GL_TEXTURE_2D, vboid_texture);

	//ekran kartına yükleyelim.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	//yükleme tamam ram üzerindekini silelim.
	delete[] data;

	//trilinear filtreleme standart komut grubu
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}


//shadowmap
shadowmap::shadowmap()
{
	width = 1024;
	height = 1024;
	vboid_texture = 0;
	fbo_depthmap = 0;
	init();
}
void shadowmap::init()
{
	glGenFramebuffers(1, &fbo_depthmap);

	glGenTextures(1, &vboid_texture);

	glBindTexture(GL_TEXTURE_2D, vboid_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo_depthmap);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, vboid_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}




//shadowcubemap
shadowcubemap::shadowcubemap()
{
	width = 1024;
	height = 1024;
	vboid_texture = 0;
	fbo_depthmap = 0;
	init();
}

void shadowcubemap::init()
{
	glGenFramebuffers(1, &fbo_depthmap);

	glGenTextures(1, &vboid_texture);

	glBindTexture(GL_TEXTURE_CUBE_MAP, vboid_texture);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glBindFramebuffer(GL_FRAMEBUFFER, fbo_depthmap);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, vboid_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
