#include "util.h"
#include <algorithm>
#include <string>
#include "spheroid.h"
#include <math.h>
#include <format>


double constexpr earth_a = 6.3781370f;
double constexpr earth_b = 6.3567523f;
double constexpr circumference = 2 * glm::pi<double>() * earth_a;


box path_to_box(const std::string& plate_path) {
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	box earth_b = { 0, 0, map_size };
	//  -map_size-
	// |---------|
	// |  c | d  |
	// |---------|
	// |  earth_a | earth_b  |
	// |---------|
	for (size_t i = 0; i < plate_path.size(); i++) {
		char c = plate_path[i] - 97;
		earth_b = {
			earth_b.x + (c & 1) * earth_b.earth_a / 2,
			earth_b.y + (c & 2) * earth_b.earth_a / 4,
			earth_b.earth_a / 2
		};
	}
	return earth_b;
}

double N(double phi){
	return earth_a * earth_a / sqrt(earth_a * earth_a * cos(phi) * cos(phi) + earth_b * earth_b * sin(phi) * sin(phi));
}

//note: don't reinvent the wheel
//https://danceswithcode.net/engineeringnotes/geodetic_to_ecef/geodetic_to_ecef.html
//Convert Earth-Centered-Earth-Fixed (ECEF) to lat, Lon, Altitude
//Input is a three element array containing x, y, z in meters
//Returned array contains lat and lon in radians, and altitude in meters
std::array<double, 3> ecef_to_geo(std::array<double, 3> ecef) {

	std::array<double, 3> geo{ 0, 0, 0 };   //Results go here (Lat, Lon, Altitude)
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

	x = ecef[0] * 1000000 ;
	y = ecef[1] * 1000000;
	z = ecef[2] * 1000000;
	zp = std::abs(z);
	w2 = x * x + y * y;
	w = std::sqrt(w2);
	r2 = w2 + z * z;
	r = std::sqrt(r2);
	geo[1] = std::atan2(y, x);       //Lon (final)
	s2 = z * z / r2;
	c2 = w2 / r2;
	u = a2 / r;
	v = a3 - a4 / r;
	if (c2 > 0.3) {
		s = (zp / r) * (1.0 + c2 * (a1 + u + s2 * v) / r);
		geo[0] = std::asin(s);      //Lat
		ss = s * s;
		c = std::sqrt(1.0 - ss);
	}
	else {
		c = (w / r) * (1.0 - s2 * (a5 - u - c2 * v) / r);
		geo[0] = std::acos(c);      //Lat
		ss = 1.0 - c * c;
		s = std::sqrt(ss);
	}
	g = 1.0 - e2 * ss;
	rg = a / std::sqrt(g);
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
	//convert to angles
	geo[0] = geo[0] * 180 / glm::pi<double>();
	geo[1] = geo[1] * 180 / glm::pi<double>();
	return(geo);    //Return Lat, Lon, Altitude in that order
}
glm::vec3 geo_to_ecef(glm::vec3 geo) {
	glm::vec3 ecef;
	double e2 = 6.6943799901377997e-3;

	double lat = geo.x;
	double lon = geo.y;
	double alt = geo.z;
	double n = earth_a / sqrt(1 - e2 * sin(lat) * sin(lat));
	ecef.x = (n + alt) * cos(lat) * cos(lon);    //ECEF x
	ecef.y = (n + alt) * cos(lat) * sin(lon);    //ECEF y
	ecef.z = (n * (1 - e2) + alt) * sin(lat);    //ECEF z
	return ecef;
}


double lon_to_mercator_x(double lon, double map_size){
	//lon range is -180 to 180 -> 0 to 360
	return map_size * ((lon + 180) / 360);
}
double lat_to_mercator_y(double lat, double map_size){
	//merc projection range
	lat = std::clamp(lat, -85.08, 85.08);

	double e2 = 1 - ((earth_b / earth_a) * (earth_b / earth_a));
	double e = sqrt(e2);
	double phi = lat * glm::pi<double>() / 180;
	double sinphi = sin(phi);
	double con = e * sinphi;
	con = pow((1.0 - con) / (1.0 + con), e / 2);
	double ts = tan(0.5 * (glm::pi<double>() * 0.5 - phi)) / con;
	double distance = 0 - earth_a * log(ts);
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
double merc_y_to_lat(double merc_y, double map_size){
	double y = circumference / map_size * (map_size / 2 - merc_y);
	double e2 = 1 - ((earth_b / earth_a) * (earth_b / earth_a));
	double e = sqrt(e2);
	double ts = exp(-y / earth_a);
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
glm::vec3 lla_to_ecef(double lat_indegrees, double lon_indegrees){
	double lat = lat_indegrees * glm::pi<double>() / 180;
	double lon = lon_indegrees * glm::pi<double>() / 180;

	double x = N(lat) * cos(lat) * cos(lon);
	double y = N(lat) * cos(lat) * sin(lon);
	double z = (((earth_b * earth_b) / (earth_a * earth_a)) * N(lat)) * sin(lat);

	return { x, y, z };
}
glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3){
	return glm::cross(pt2 - pt1, pt3 - pt1);
}

bool solve_quadratic(float a, float b, float c, float& t0, float& t1) {
	float discriminant = (b * b) - (4 * a * c);
	if (discriminant < 0) //no solution
		return false; 

	if (discriminant == 0) { //1 solution
		t0 = -b / 2.0f * a;
	}
	else if (discriminant > 0) {//2 solutions;
		auto droot = std::sqrtf(discriminant);
		t0 = (- b - droot) / (2.0f * a);
		t1 = (- b + droot) / (2.0f * a);
	}
	return true;
}

//TODO: WGS84 is a spheroid not a sphere!
glm::vec3 sphere_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction) {
	//a = dot of ray origin -> dot(camera pos) or a mouse ray
	//b = ray direction -> 
	//r = radius -> earth_a
	//t = hit distance -> solutions
	float a = glm::dot(ray_direction, ray_direction);
	float b = glm::dot(ray_origin, ray_direction) * 2.0f;
	float c = glm::dot(ray_origin, ray_origin) - (earth_a * earth_a);

	float t0{ 0 }, t1{ 0 };
	if (!solve_quadratic(a, b, c, t0, t1))
		return glm::vec3(0, 0, 0);

	glm::vec3 hit1 = ray_origin + ray_direction * t0;
	glm::vec3 hit2 = ray_origin + ray_direction * t1;
	//TODO: find the cause of this left hand - right hand difference, probably in inverse transformations.
	auto r1 = ecef_to_geo({ -hit1.x, hit1.y, -hit1.z });
	auto r2 = ecef_to_geo({ -hit2.x, hit2.y, -hit2.z });

	auto rx = t0 < 0 ? r2 : r1;
	return glm::vec3(rx[0], rx[1], rx[2]);
}

glm::vec3 cast_ray(glm::vec2 mouse, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view, float dir) {
	glm::vec2 ndc_mouse{ ((mouse.x * 2) - viewport.x) / viewport.x, -((mouse.y * 2) - viewport.y) / viewport.y};
	glm::vec4 ray_clip{ ndc_mouse, dir, 1.0f };
	glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
	glm::vec4 ray_world = glm::inverse(view) * ray_eye;
	ray_world /= ray_world.w;
	return ray_world;
}