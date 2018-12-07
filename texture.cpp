 #include "stdafx.h"
#include "texture.h"
#include <wingdi.h>

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

void texture::unload()
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
bmp::~bmp()
{
	unload();
}

void bmp::load(const std::string& filename)
{
	BITMAPFILEHEADER* hdr;
	BITMAPINFOHEADER* info;

	unsigned char* data;
	unsigned char* uchdr = new unsigned char[54+1];

	fstream f;
	f.open(filename, std::fstream::in | std::fstream::binary);
	if (!f.is_open())
		return;

	f.read((char*)uchdr, 54);
	hdr = (BITMAPFILEHEADER*)uchdr;
	info = (BITMAPINFOHEADER*)(uchdr + sizeof(BITMAPFILEHEADER));

	if (uchdr[0] != 'B' || uchdr[1] != 'M')
		return;

	data = new unsigned char[hdr->bfSize-54];

	f.read((char*)data, hdr->bfSize - 54);

	f.close();

	load(data, info->biWidth, info->biHeight, info->biBitCount);

	delete[] uchdr;
	delete[] data;

}

void bmp::load(unsigned char* data, int w, int h, int bitcount)
{
	glGenTextures(1, &vboid_texture);
	glBindTexture(GL_TEXTURE_2D, vboid_texture);

	unsigned int type = bitcount == 32 ? GL_BGRA : GL_BGR;

	//ekran kartına yükleyelim.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, type, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

void bmp::unload()
{
	glDeleteTextures(1, &vboid_texture);
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
	char* data = new char[imgsize];

	glGenTextures(1, &vboid_texture);
	glBindTexture(GL_TEXTURE_2D, vboid_texture);

	//ekran kartına yükleyelim.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	//yükleme tamam ram üzerindekini silelim.
	delete[] data;

	//trilinear filtreleme standart komut grubu
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
