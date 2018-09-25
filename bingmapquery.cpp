#include "stdafx.h"



bingmapquery::bingmapquery()
{
}

bingmapquery::~bingmapquery()
{

}



std::vector<char> bingmapquery::getmap(float slat, float wlong, float nlat, float elong, int width, int height)
{
	http_client cl;
	string url = "https://dev.virtualearth.net/REST/V1/Imagery/Map/road?";
	url += "mapArea=" + to_string(slat) + "," + to_string(wlong) + "," + to_string(nlat) + "," + to_string(elong);
	url += "&ms="+ to_string(width) +"," +to_string(height);
	url += "&key";
	url += "=AuxHX17Q-LDnXLPRCncQ3tEFgpfR4e7r";
	url += "SMCIMH3Faig4GP54xCFGghlyW_oTfu0c";

	return cl.get_binary_page("");
}