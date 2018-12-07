#include "stdafx.h"
#include "quadtile.h"
#include "sphere.h"
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


//long live copy paste codes from internet!
std::array<double, 3> ecef_to_geo(std::array<double,3> ecef) {

	double  a = 6378137.0;              //WGS-84 semi-major axis
	double e2 = 6.6943799901377997e-3;  //WGS-84 first eccentricity squared
	double a1 = 4.2697672707157535e+4;  //a1 = a*e2
	double a2 = 1.8230912546075455e+9;  //a2 = a1*a1
	double a3 = 1.4291722289812413e+2;  //a3 = a1*e2/2
	double a4 = 4.5577281365188637e+9;  //a4 = 2.5*a2
	double a5 = 4.2840589930055659e+4;  //a5 = a1+a3
	double a6 = 9.9330562000986220e-1;  //a6 = 1-e2
	double zp, w2, w, r2, r, s2, c2, s, c, ss;
	double g, rg, rf, u, v, m, f, p, x, y, z;
	double n, lat, lon, alt;
	

	std::array<double,3> geo;   //Results go here (Lat, Lon, Altitude)
	x = ecef[0];
	y = ecef[1];
	z = ecef[2];
	zp = abs(z);
	w2 = x * x + y * y;
	w = sqrt(w2);
	r2 = w2 + z * z;
	r = sqrt(r2);
	geo[1] = atan2(y, x);       //Lon (final)
	s2 = z * z / r2;
	c2 = w2 / r2;
	u = a2 / r;
	v = a3 - a4 / r;
	if (c2 > 0.3) {
		s = (zp / r)*(1.0 + c2 * (a1 + u + s2 * v) / r);
		geo[0] = asin(s);      //Lat
		ss = s * s;
		c = sqrt(1.0 - ss);
	}
	else {
		c = (w / r)*(1.0 - s2 * (a5 - u - c2 * v) / r);
		geo[0] = acos(c);      //Lat
		ss = 1.0 - c * c;
		s = sqrt(ss);
	}
	g = 1.0 - e2 * ss;
	rg = a / sqrt(g);
	rf = a6 * rg;
	u = w - rg * c;
	v = zp - rf * s;
	f = c * u + s * v;
	m = c * v - s * u;
	p = m / (rf / g + f);
	geo[0] = geo[0] + p;      //Lat
	geo[2] = f + m * p / 2.0;     //Altitude
	if (z < 0.0) {
		geo[0] *= -1.0;     //Lat
	}

	geo[0] = geo[0] * 180 / glm::pi<double>();
	geo[1] = geo[1] * 180 / glm::pi<double>();

	return(geo);    //Return Lat, Lon, Altitude in that order
}


double lon2mercx(double lon, double mapsize = 1024)
{
	//lon range is -180 to 180
	return mapsize * ((lon + 180) / 360);
}

double lat2mercy(double lat, double mapsize = 1024)
{
	if (lat < -85.08)
		lat = -85.08;
	
	float a = 6378137.0f;
	float b = 6356752.3f;

	float e2 = 1 - ((b / a) * (b / a));
	float e = sqrt(e2);

	double phi = lat * glm::pi<double>()/180;
	double sinphi = sin(phi);
	double con = e * sinphi;

	con = pow((1.0 - con) / (1.0 + con), e/2);

	double ts = tan(0.5 * (glm::pi<double>() * 0.5 - phi)) / con;

	double distance = 0 - a * log(ts);
	double circumference = 2 * glm::pi<double>() * 6378137.0f;
	//0 to circumference
	distance += circumference / 2;
	//now take the complement.
	distance = circumference - distance;

	distance = (distance / circumference) * mapsize;

	//sanity check
	distance = distance < 0 ? 0 : distance;
	distance = distance > mapsize ? mapsize : distance;

	return distance;
}


double getcornerdistance(string quadkey, double lat, double lon)
{
	double distance = 0;

	if (quadkey.size() < 1)
		return 1;

	double mapsize = 1024;

	double x1 = 0;
	double x2 = mapsize;
	double y1 = 0;
	double y2 = mapsize;
	double mercx = lon2mercx(lon, mapsize);
	double mercy = lat2mercy(lat, mapsize);

	std::array<double, 4> distances;

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

	double a, b;
	
	//topleft
	a = x1 - mercx;
	b = y1 - mercy;
	distance += sqrt(pow(a, 2) + pow(b, 2));

	
	//topright
	a = x2 - mercx;
	b = y1 - mercy;
	distance += sqrt(pow(a, 2) + pow(b, 2));

	
	//bottomleft
	a = x1 - mercx;
	b = y2 - mercy;
	distance += sqrt(pow(a, 2) + pow(b, 2));

	
	//bottomright
	a = x2 - mercx;
	b = y2 - mercy;
	distance += sqrt(pow(a, 2) + pow(b, 2));

	return distance;
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
	///BUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//make this 12 and entire system going nuts at 13rd zoom level
	platebase = 16;
}

quadtile::~quadtile()
{
	invalidate("");

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

double getdelta(int zoomlevel)
{
	//find a formula for this?
	//or handcraft?
	switch (zoomlevel)
	{
	case 1:
		return 2.0;
		break;
	case 2:
		return 2.0;
		break;
	case 3:
		return 2.0;
		break;
	case 4:
		return 1.4;
		break;
	default:
		return 1.0;
		break;
	}
}


vector<quadtile*> quadtile::calculatesubtiles(glm::vec3 cameraPos, int zoomlevel, float delta)
{
	//everytile represented by its own subtiles
	//so root yields four subtile; A B C D
	if (( zoomlevel ) == quadkey.size())
	{
		vector<quadtile*> t;
		t.push_back(this);
		return t;
	}

	float min = 90 * 400;
	int mintile = 0;
	string subtile = "";
	std::array<double, 4> distances;
	

	for (size_t x = 0; x < 4; x++)
	{
		subtile = char(65 + x);

		std::array<double, 3> lla  = ecef_to_geo({ cameraPos.x, cameraPos.y, cameraPos.z });
		float diff = getcornerdistance(quadkey + subtile, lla[0], lla[1]);
		
		distances[x] = diff;

		if (diff < min)
		{
			mintile = x;
			min = diff;
		}
	}

	vector<int> closetiles;
	for (size_t x = 0; x < 4; x++)
	{
		if ((min * delta) >= distances[x])
		{
			//close to mintile or itself
			closetiles.push_back(x);
		}
	}
	
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

		bool found = false;
		for (auto ct : closetiles)
		{
			if (i == ct)
			{
				found = true;
				break;
			}
		}
		if (!found) 
		{
			//destroy its children and add it to list
			children[i].invalidate("");
			t.push_back(&children[i]);
		}
	}

	//now lets go deeper in mintile and qualified closest tiles and figure out which subtiles are needed to be shown
	//if the subchildren are not loaded completely then the tile should displays itself instead, or we get black areas when loading
	for (auto ct : closetiles)
	{
		vector<quadtile*> subtiles = children[ct].calculatesubtiles(cameraPos, zoomlevel, ct == mintile ? delta : 1.00  );
		t.insert(t.end(), subtiles.begin(), subtiles.end());
	}
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

	int platenum = platebase - quadkey.size() > 4 ? platebase - quadkey.size() : 4;

	
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

	if (quadkey == "A")
	{
		int ff = 5;

	}

	double centerx = (x2 + x1) / 2;
	double centery = (y2 + y1) / 2;

	lat_center = ytolat(circumference / mapsize * (mapsize / 2 - centery));
	lon_center = (360.0 / mapsize * centerx) - 180;


	double b = 6356752.3f;
	double a = 6378137.0f;
	double e2 = 1 - ((b*b) / (a*a));
	double e = sqrt(e2);

	double xstep = (x2 - x1) / platenum;
	double ystep = (y2 - y1) / platenum;

	int px = -1;
	int py = -1;


	for (size_t x = 0; x < platenum; x++)
	{
		for (size_t y = 0; y < platenum; y++)
		{
			double mercx1 = x1 + x * xstep;
			double mercx2 = x1 + (x + 1) * xstep;

			double mercy1 = y1 + y * ystep;
			double mercy2 = y1 + (y + 1) * ystep;

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

		
			glm::vec2 uvtopleft = { (x * xstep) / 256.0,  1.0 - (y * ystep) / 256.0 };
			glm::vec2 uvbottomleft = { (x * xstep) / 256.0,  1.0 - (y + 1) * ystep / 256.0 };
			glm::vec2 uvtopright = { (x + 1) * xstep / 256.0, 1.0 - (y * ystep) / 256.0 };

			uvs.push_back(uvtopleft);
			uvs.push_back(uvbottomleft);
			uvs.push_back(uvtopright);

			normals.push_back(calc_normal(topleft, bottomleft, topright));//topleft
			normals.push_back(calc_normal(bottomleft, topright, topleft));//bottomleft
			normals.push_back(calc_normal(topright, topleft, bottomleft));//topright

			vertices.push_back(bottomleft);
			vertices.push_back(bottomright);
			vertices.push_back(topright);

			glm::vec2 uvbottomright = { (x + 1) * xstep / 256.0, 1.0 - (y + 1) * ystep / 256.0 };

			uvs.push_back(uvbottomleft);
			uvs.push_back(uvbottomright);
			uvs.push_back(uvtopright);

			normals.push_back(calc_normal(bottomleft, bottomright, topright));//bottomleft
			normals.push_back(calc_normal(bottomright, topright, bottomleft));//bottomright
			normals.push_back(calc_normal(topright, bottomleft, bottomright));//topright
		}
	}


	////haritalarý yükle
	string req = "https://www.mapquestapi.com/staticmap/v5/map?key=y5i8o8QO8e8AsZ2k2I4VKPifkxkd1lJd&format=png&center=" +
		to_string(lat_center) + "," + to_string(lon_center) +
		"&size=256,256&zoom=" + to_string(quadkey.size());

	fname = L"C:\\mapdata\\" + wstring(quadkey.begin(), quadkey.end()) + L".bmp";

	if (!std::filesystem::exists("C:\\mapdata\\" + quadkey + ".bmp"))
	{
		cout << "requested: " << quadkey << endl;
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