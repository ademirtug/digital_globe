#include "stdafx.h"
#include "quadtile.h"
#include "sphere.h";
#include <filesystem>


extern engine eng;
using namespace Gdiplus;
using std::min;
using std::max;

float ytolat(float y)
{
	float a = 6378137.0f;
	float b = 6356752.3f;

	float e2 = 1 - ((b / a) * (b / a));
	float e = sqrt(e2);

	float ts = exp(-y / a);
	float phi = (glm::pi<float>() / 2) - 2 * atan(ts);
	float dphi = 1.0;
	int i = 0;

	while ((abs(dphi) > 0.000000001) && (i < 25))
	{
		float con = e * sin(phi);
		dphi = glm::pi<float>() / 2 - 2 * atan(ts * powf((1.0 - con) / (1.0 + con), e / 2)) - phi;
		phi += dphi;
		i++;
	}
	return (phi * 180.0 / glm::pi<float>());
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;
	UINT  size = 0;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3)
{
	return glm::cross(pt2 - pt1, pt3 - pt1);
}

double N(double phi)
{
	double a = 6378137.0f;
	double b = 6356752.3f;
	return a*a / sqrt(a*a * cos(phi)*cos(phi) + b*b*sin(phi)* sin(phi));
}

std::array<double,3> ecef2lla(double x, double y, double z) {

	double b = 6356752.3f;
	double a = 6378137.0f;
	double e2 = 0.00669437999013;
	std::array<double, 3> lla = { 0, 0, 0 };
	double lat, lon, height, N, theta, p;

	lon = 2 * atan((sqrt(x*x + y*y) - x) / y);

	p = sqrt(x*x + y*y);

	theta = atan((z * a) / (p * b));

	lon = atan(y / x);

	lat = atan(((z + e2*e2 * b * pow(sin(theta), 3)) / ((p - pow(e2, 2) * a * pow(cos(theta), 3)))));
	N = a / (sqrt(1 - (pow(e2, 2) * pow(sin(lat), 2))));

	double m = (p / cos(lat));
	height = m - N;


	lon = lon * 180 / glm::pi<double>();
	lat = lat * 180 / glm::pi<double>();
	lla[0] = lat;
	lla[1] = lon;
	lla[2] = height;


	return lla;

}

normalspack getcornernormals(string quadkey)
{
	normalspack corners;
	if (quadkey.size() < 1)
		return corners;

	if (quadkey.size() > 1)
	{
		int x = 5;
	}

	double platenum = 32;
	double circumference = 2 * glm::pi<double>() * 6378137.0f;
	double mapsize = pow(2, quadkey.size()) * 256;

	double x1 = 0;
	double x2 = mapsize;
	double y1 = 0;
	double y2 = mapsize;


	//2 üzeri seviye kadar en boy parça
	for (size_t i = 0; i < quadkey.size(); i++)
	{
		if (quadkey[i] == 'A')
		{
			y2 = (y2 + y1) / 2;
			x2 = (x2 + x1) / 2;
		}
		else if (quadkey[i] == 'B')
		{
			x1 = (x2 + x1) / 2;
			y2 = (y2 + y1) / 2;
		}
		else if (quadkey[i] == 'C')
		{
			x2 = (x2 + x1) / 2;
			y1 = (y2 + y1) / 2;
		}
		else if (quadkey[i] == 'D')
		{
			x1 = (x2 + x1) / 2;
			y1 = (y2 + y1) / 2;
		}
	}

	double centerx = (x2 + x1) / 2;
	double centery = (y2 + y1) / 2;

	double lat = ytolat(circumference / mapsize * (mapsize / 2 - centery));
	double lon = (360.0 / mapsize * centerx) - 180;


	double test = ytolat(circumference / mapsize * (-256));

	float b = 6356752.3f;
	float a = 6378137.0f;
	float e2 = 1 - ((b*b) / (a*a));
	float e = sqrt(e2);

	double xstep = (x2 - x1) / platenum;
	double ystep = (y2 - y1) / platenum;

	//make a clean algo for this;
	for (size_t x = 0; x < platenum; x++)
	{
		for (size_t i = 0; i < platenum; i++)
		{
			if ((x == 0 && i == 0) || (x == 0 && i == (platenum - 1)) || (x == (platenum - 1) && i == 0) || (x == (platenum - 1) && i == (platenum - 1)))
			{
				double mercx1 = x1 + i * xstep;
				double mercx2 = x1 + (i + 1) * xstep;

				double mercy1 = y1 + x * ystep;
				double mercy2 = y1 + (x + 1) * ystep;


				//2d to lat long
				double lat1 = ytolat(circumference / mapsize * (mapsize / 2 - mercy1));
				double lon1 = (360.0 / mapsize * mercx1) - 180;

				glm::vec3 ecef = lla2ecef(lat1, lon1);
				glm::vec3 topleft = { ecef.x, ecef.y, ecef.z };


				double lat2 = ytolat(circumference / mapsize * (mapsize / 2 - mercy1));
				double lon2 = (360.0 / mapsize * mercx2) - 180;

				ecef = lla2ecef(lat2, lon2);
				glm::vec3 topright = { ecef.x, ecef.y, ecef.z };


				double lat3 = ytolat(circumference / mapsize * (mapsize / 2 - mercy2));
				double lon3 = (360.0 / mapsize * mercx1) - 180;


				ecef = lla2ecef(lat3, lon3);
				glm::vec3 bottomleft = { ecef.x, ecef.y, ecef.z };

				double lat4 = ytolat(circumference / mapsize * (mapsize / 2 - mercy2));
				double lon4 = (360.0 / mapsize * mercx2) - 180;


				ecef = lla2ecef(lat4, lon4);
				glm::vec3 bottomright = { ecef.x, ecef.y, ecef.z };


				glm::vec2 uvtopleft = { (i * xstep) / 256,  -(x * ystep) / 256 };
				glm::vec2 uvbottomleft = { (i * xstep) / 256,  -((x + 1) * ystep) / 256 };
				glm::vec2 uvtopright = { ((i + 1) * xstep) / 256, -(x * ystep) / 256 };


				glm::vec2 uvbottomright = { ((i + 1) * xstep) / 256, -((x + 1) * ystep) / 256 };


				if (x == 0 && i == 0)
				{
					corners.upperleft = calc_normal(topleft, bottomleft, topright);
				}
				if (x == 0 && i == (platenum - 1))
				{
					corners.bottomleft = calc_normal(bottomleft, topright, topleft);
				}
				if (x == (platenum - 1) && i == 0)
				{
					corners.upperright = calc_normal(topright, topleft, bottomleft);
				}
				if (x == (platenum - 1) && i == (platenum - 1))
				{
					corners.bottomright = calc_normal(bottomright, topright, bottomleft);
				}
			}
		}
	}
	return corners;
}

glm::vec3 lla2ecef(double lat_indegrees, double lon_indegrees)
{
	double a = 6378137.0f;
	double b = 6356752.3f;

	double lat = lat_indegrees * glm::pi<double>() / 180;
	double lon = lon_indegrees * glm::pi<double>() / 180;

	double x = N(lat) * cos(lat) * cos(lon);
	double y = N(lat) * cos(lat) * sin(lon);
	double z = (((b*b) / (a*a)) * N(lat)) * sin(lat);

	return { x, y, z };
}

quadtile::quadtile()
{
	children = nullptr;	
	platenum = 32;
}

quadtile::~quadtile()
{
	if (children != nullptr)
	{
		delete[] children;
		children = nullptr;
	}
	if (tm != nullptr)
		tm.reset();
}

void quadtile::initchildren(string _quadkey)
{
	if (_quadkey.size() > 0)
		quadkey = _quadkey;

	if (children != nullptr)
		return;

	children = new quadtile[4];
	for (size_t i = 0; i < 4; i++)
	{
		char subkey = 65 + i;
		children[i].quadkey = quadkey + subkey;
	}
}

quadtile* quadtile::getchild(char c)
{
	if (children)
		return &children[c - 65];
	return nullptr;
}

quadtile* quadtile::gettile(string tile)
{
	if (tile.size() == 0)
	{
		return this;
	}

	quadtile* child = getchild(tile.at(0));
	if (child == nullptr)
	{
		return nullptr;
	}

	return child->gettile(tile.substr(1));
}

vector<quadtile*> quadtile::calculatesubtiles(glm::vec3 cameraPos, int zoomlevel)
{
	//everytile represented by its own subtiles
	//so root yields four subtile; A B C D

	if (( zoomlevel ) == quadkey.size())
	{
		vector<quadtile*> t;
		t.push_back(this);
		return t;
	}
	

	float min = 90 * 4;
	int mintile = 0;
	string subtile = "";

	for (size_t x = 0; x < 4; x++)
	{
		subtile = char(65 + x);

		normalspack corners = getcornernormals(quadkey+subtile);
		float diff = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.bottomleft)));
		diff += (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.bottomright)));
		diff += (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.upperleft)));
		diff += (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.upperright)));

		if (diff < min)
		{
			mintile = x;
			min = diff;
		}
	}
	subtile = (char)(65 + mintile);

	//ok we now know that user has zoomed to mintile, now the rest should be invalidated
	//because we don't want them to cumulate and fill up all the memory the computer have
	vector<quadtile*> t;

	if (children == nullptr)
		initchildren(quadkey);


	for (size_t i = 0; i < 4; i++)
	{
		string ct = "";
		ct = ((char)(65 + i));

		if (!children[i].requested)
		{
			eng.sc->earth->pool.queue(shared_ptr<tilerequest>(new tilerequest(quadkey + ct)));
			children[i].requested = true;
		}

		if (i != mintile)
		{
			//destroy its children and add it to list
			children[i].invalidate("");
			t.push_back(&children[i]);
		}
	}


	//now lets go deeper in mintile and figure out which subtiles are needed to be shown
	//if the subchildren are not loaded completely then the tile displays itself instead
	vector<quadtile*> subtiles = children[mintile].calculatesubtiles(cameraPos, zoomlevel);

	t.insert(t.end(), subtiles.begin(), subtiles.end());

	return t;
}

void quadtile::invalidate(string tile)
{
	if (tile.size() == 0)
	{
		if (children != nullptr)
		{
			delete[] children;
			children = nullptr;
		}
		return;
	}


	quadtile* child = getchild(tile.at(0));
	if (child == nullptr)
		return;

	child->invalidate(tile.substr(1));
}

void quadtile::buildplates()
{
	if (quadkey.size() < 1)
		return;

	double circumference = 2 * glm::pi<double>() * 6378137.0f;
	double mapsize = pow(2, quadkey.size()) * 256;

	double x1 = 0;
	double x2 = mapsize;
	double y1 = 0;
	double y2 = mapsize;


	//2 üzeri seviye kadar en boy parça
	for (size_t i = 0; i < quadkey.size(); i++)
	{
		if (quadkey[i] == 'A')
		{
			y2 = (y2 + y1) / 2;
			x2 = (x2 + x1) / 2;
		}
		else if (quadkey[i] == 'B')
		{
			x1 = (x2 + x1) / 2;
			y2 = (y2 + y1) / 2;
		}
		else if (quadkey[i] == 'C')
		{
			x2 = (x2 + x1) / 2;
			y1 = (y2 + y1) / 2;
		}
		else if (quadkey[i] == 'D')
		{
			x1 = (x2 + x1) / 2;
			y1 = (y2 + y1) / 2;
		}
	}

	double centerx = (x2 + x1) / 2;
	double centery = (y2 + y1) / 2;

	lat_center = ytolat(circumference / mapsize * (mapsize / 2 - centery));
	lon_center = (360.0 / mapsize * centerx) - 180;


	float b = 6356752.3f;
	float a = 6378137.0f;
	float e2 = 1 - ((b*b) / (a*a));
	float e = sqrt(e2);

	double xstep = (x2 - x1) / platenum;
	double ystep = (y2 - y1) / platenum;

	int px = -1;
	int py = -1;


	for (size_t x = 0; x < platenum; x++)
	{
		for (size_t i = 0; i < platenum; i++)
		{
			double mercx1 = x1 + i * xstep;
			double mercx2 = x1 + (i + 1) * xstep;

			double mercy1 = y1 + x * ystep;
			double mercy2 = y1 + (x + 1) * ystep;

			//2d to lat long
			double lat1 = ytolat(circumference / mapsize * (mapsize / 2 - mercy1));
			double lon1 = (360.0 / mapsize * mercx1) - 180;

			glm::vec3 ecef = lla2ecef(lat1, lon1);
			glm::vec3 topleft = { ecef.x, ecef.y, ecef.z };

			double lat2 = ytolat(circumference / mapsize * (mapsize / 2 - mercy1));
			double lon2 = (360.0 / mapsize * mercx2) - 180;

			ecef = lla2ecef(lat2, lon2);
			glm::vec3 topright = { ecef.x, ecef.y, ecef.z };

			double lat3 = ytolat(circumference / mapsize * (mapsize / 2 - mercy2));
			double lon3 = (360.0 / mapsize * mercx1) - 180;

			ecef = lla2ecef(lat3, lon3);
			glm::vec3 bottomleft = { ecef.x, ecef.y, ecef.z };

			double lat4 = ytolat(circumference / mapsize * (mapsize / 2 - mercy2));
			double lon4 = (360.0 / mapsize * mercx2) - 180;

			ecef = lla2ecef(lat4, lon4);
			glm::vec3 bottomright = { ecef.x, ecef.y, ecef.z };

			vertices.push_back(topleft);
			vertices.push_back(bottomleft);
			vertices.push_back(topright);

			glm::vec2 uvtopleft = { (i * xstep) / 256,  -(x * ystep) / 256 };
			glm::vec2 uvbottomleft = { (i * xstep) / 256,  -((x + 1) * ystep) / 256 };
			glm::vec2 uvtopright = { ((i + 1) * xstep) / 256, -(x * ystep) / 256 };

			uvs.push_back(uvtopleft);
			uvs.push_back(uvbottomleft);
			uvs.push_back(uvtopright);

			normals.push_back(calc_normal(topleft, bottomleft, topright));//topleft
			normals.push_back(calc_normal(bottomleft, topright, topleft));//bottomleft
			normals.push_back(calc_normal(topright, topleft, bottomleft));//topright

			vertices.push_back(bottomleft);
			vertices.push_back(bottomright);
			vertices.push_back(topright);

			glm::vec2 uvbottomright = { ((i + 1) * xstep) / 256, -((x + 1) * ystep) / 256 };

			uvs.push_back(uvbottomleft);
			uvs.push_back(uvbottomright);
			uvs.push_back(uvtopright);

			normals.push_back(calc_normal(bottomleft, bottomright, topright));//bottomleft
			normals.push_back(calc_normal(bottomright, topright, bottomleft));//bottomright
			normals.push_back(calc_normal(topright, bottomleft, bottomright));//topright

			if (x == 0 && i == 0)
			{
				corners.upperleft = calc_normal(topleft, bottomleft, topright);
			}
			if (x == 0 && i == (platenum - 1))
			{
				corners.bottomleft = calc_normal(bottomleft, topright, topleft);
			}
			if (x == (platenum - 1) && i == 0)
			{
				corners.upperright = calc_normal(topright, topleft, bottomleft);
			}
			if (x == (platenum - 1) && i == (platenum - 1))
			{
				corners.bottomright = calc_normal(bottomright, topright, bottomleft);
			}
		}
	}


	////haritalarý yükle
	string req = "https://www.mapquestapi.com/staticmap/v5/map?key=kAGxoy8TfqxNPPXu1Va54jWMoYMkRCbG&format=png&center=" +
		to_string(lat_center) + "," + to_string(lon_center) +
		"&size=256,256&zoom=" + to_string(quadkey.size());

	fname = L"C:\\mapdata\\" + wstring(quadkey.begin(), quadkey.end()) + L".bmp";

	if (!std::filesystem::exists("C:\\mapdata\\" + quadkey + ".bmp"))
	{
		http_client hc;
		vector<unsigned char> png = hc.get_binary_page(req);
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		CLSID   encoderClsid;
		Status  stat;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, png.size());

		PVOID pMem = GlobalLock(hMem);
		RtlMoveMemory(pMem, &png[0], png.size());
		IStream *pStream = 0;
		HRESULT hr = CreateStreamOnHGlobal(hMem, TRUE, &pStream);

		Image* img = new Gdiplus::Image(pStream);
		GetEncoderClsid(L"image/bmp", &encoderClsid);

		stat = img->Save(fname.c_str(), &encoderClsid, NULL);
		delete img;
		GdiplusShutdown(gdiplusToken);
	}
}
