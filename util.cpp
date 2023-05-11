#include "util.h"
#include <algorithm>
#include <string>
#include "spheroid.h"


box path_to_box(const std::string& plate_path) {
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	box b = { 0, 0, map_size };
	//  -map_size-
	// |---------|
	// |  c | d  |
	// |---------|
	// |  a | b  |
	// |---------|
	for (size_t i = 0; i < plate_path.size(); i++) {
		char c = plate_path[i] - 97;
		b = {
			b.x + (c & 1) * b.a / 2,
			b.y + (c & 2) * b.a / 4,
			b.a / 2
		};
	}
	return b;
}




double a = 6.3781370f;
double b = 6.3567523f;
double circumference = 2 * glm::pi<double>() * a;

double N(double phi)
{
	return a * a / sqrt(a * a * cos(phi) * cos(phi) + b * b * sin(phi) * sin(phi));
}

//note: don'tex reinvent the wheel
//https://danceswithcode.net/engineeringnotes/geodetic_to_ecef/geodetic_to_ecef.html
std::array<double, 3> ecef_to_geo(const std::array<double, 3>& ecef) {
	double a = 6378137.0f;
	double e2 = 6.6943799901377997e-3;
	double a1 = 4.2697672707157535e+4;
	double a2 = 1.8230912546075455e+9;
	double a3 = 1.4291722289812413e+2;
	double a4 = 4.5577281365188637e+9;
	double a5 = 4.2840589930055659e+4;
	double a6 = 9.9330562000986220e-1;
	double zp, w2, w, r2, r, s2, c2, s, c, ss;
	double g, rg, rf, u, v, m, f, p, x, y, z;
	double n;

	std::array<double, 3> geo;
	x = ecef[0];
	y = ecef[1];
	z = ecef[2];
	zp = abs(z);
	w2 = x * x + y * y;
	w = sqrt(w2);
	r2 = w2 + z * z;
	r = sqrt(r2);
	geo[1] = atan2(y, x);
	s2 = z * z / r2;
	c2 = w2 / r2;
	u = a2 / r;
	v = a3 - a4 / r;
	if (c2 > 0.3) {
		s = (zp / r) * (1.0 + c2 * (a1 + u + s2 * v) / r);
		geo[0] = asin(s);
		ss = s * s;
		c = sqrt(1.0 - ss);
	}
	else {
		c = (w / r) * (1.0 - s2 * (a5 - u - c2 * v) / r);
		geo[0] = acos(c);
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
	geo[0] = geo[0] + p;
	geo[2] = f + m * p / 2.0;
	if (z < 0.0) {
		geo[0] *= -1.0;
	}

	geo[0] = geo[0] * 180 / glm::pi<double>();
	geo[1] = geo[1] * 180 / glm::pi<double>();

	return(geo);
}
glm::vec3 geo_to_ecef(glm::vec3 geo) {
	glm::vec3 ecef;
	double e2 = 6.6943799901377997e-3;

	double lat = geo.x;
	double lon = geo.y;
	double alt = geo.z;
	double n = a / sqrt(1 - e2 * sin(lat) * sin(lat));
	ecef.x = (n + alt) * cos(lat) * cos(lon);    //ECEF x
	ecef.y = (n + alt) * cos(lat) * sin(lon);    //ECEF y
	ecef.z = (n * (1 - e2) + alt) * sin(lat);    //ECEF z
	return ecef;
}


double lon_to_mercator_x(double lon, double map_size)
{
	//lon range is -180 to 180
	return map_size * ((lon + 180) / 360);
}


double lat_to_mercator_y(double lat, double map_size)
{
	//merc projection range
	lat = std::clamp(lat, -85.08, 85.08);

	double e2 = 1 - ((b / a) * (b / a));
	double e = sqrt(e2);
	double phi = lat * glm::pi<double>() / 180;
	double sinphi = sin(phi);
	double con = e * sinphi;
	con = pow((1.0 - con) / (1.0 + con), e / 2);
	double ts = tan(0.5 * (glm::pi<double>() * 0.5 - phi)) / con;
	double distance = 0 - a * log(ts);
	//0 to circumference
	distance += circumference / 2;
	//now take the complement.
	distance = circumference - distance;
	distance = (distance / circumference) * map_size;
	//sanity check
	distance = distance < 0 ? 0 : distance;
	distance = distance > map_size ? map_size : distance;

	return distance;
}

double merc_x_to_lon(double merc_x, double map_size) {
	double lon = (360.0 / map_size * merc_x) - 180;
	return lon;
}
double merc_y_to_lat(double merc_y, double map_size)
{
	auto y = circumference / map_size * (map_size / 2 - merc_y);
	double e2 = 1 - ((b / a) * (b / a));
	double e = sqrt(e2);
	double ts = exp(-y / a);
	double phi = (glm::pi<double>() / 2) - 2 * atan(ts);
	double dphi = 1.0;
	int i = 0;

	while ((abs(dphi) > 0.000000001) && (i < 25)) {
		double con = e * sin(phi);
		dphi = glm::pi<double>() / 2.0 - 2.0 * atan(ts * powf((1.0 - con) / (1.0 + con), e / 2.0)) - phi;
		phi += dphi;
		i++;
	}
	return (phi * 180.0 / glm::pi<double>());
}


glm::vec3 merc_to_ecef(glm::vec3 input, double map_size){
	double lat = merc_y_to_lat(input.y, map_size);
	double lon = merc_x_to_lon(input.x, map_size);
	return lla_to_ecef(lat, lon);
}
glm::vec3 lla_to_ecef(double lat_indegrees, double lon_indegrees)
{
	double lat = lat_indegrees * glm::pi<double>() / 180;
	double lon = lon_indegrees * glm::pi<double>() / 180;

	double x = N(lat) * cos(lat) * cos(lon);
	double y = N(lat) * cos(lat) * sin(lon);
	double z = (((b * b) / (a * a)) * N(lat)) * sin(lat);

	return { x, y, z };
}


glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3)
{
	return glm::cross(pt2 - pt1, pt3 - pt1);
}