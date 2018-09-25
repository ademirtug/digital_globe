#ifndef __BINGMAPQUERY_H__
#define __BINGMAPQUERY_H__


class bingmapquery
{

public:
	bingmapquery();
	~bingmapquery();
	std::vector<char> getmap(float slat, float wlong, float nlat, float elong, int width, int height);
};


#endif//__BINGMAPQUERY_H__
