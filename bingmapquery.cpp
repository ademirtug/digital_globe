#include "stdafx.h"




bingmapquery::bingmapquery()
{
}

bingmapquery::~bingmapquery()
{

}



std::vector<unsigned char> bingmapquery::getmap(float slat, float wlong, float nlat, float elong, int width, int height)
{
	//bunun ötesi mercator da yok
	slat = slat > 85.05112878 ? 85.05112878 : slat;
	nlat = nlat > 85.05112878 ? 85.05112878 : nlat;

	http_client cl;
	string url = "https://dev.virtualearth.net/REST/V1/Imagery/Map/road?";
	url += "mapArea=" + to_string(slat) + "," + to_string(wlong) + "," + to_string(nlat) + "," + to_string(elong);
	url += "&ms=" + to_string(width) + "," + to_string(height);
	url += "&k";
	url += "e";
	url += "y";
	url += "=AuxHX17Q-LDnXLPRCncQ3tEFgpfR4e7rSMCIMH3Faig4GP54xCFGghlyW_oTfu0c";



	return cl.get_binary_page("");
}