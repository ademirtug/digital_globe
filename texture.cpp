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
