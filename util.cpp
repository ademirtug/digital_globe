#include "util.h"
#include <algorithm>
#include <string>
#include "spheroid.h"
#include <math.h>
#include <format>
#include <bitset>

square path_to_square(const std::string& plate_path) {
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	square b = { 0, 0, map_size };
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
std::string merc_to_path(glm::vec2 merc, double zoom) {
	std::string plate_path;
	// |------------------|
	// |  c(10)  | d(11)  | up
	// |------------------|
	// |  a(00)  | b(01)  | down
	// |------------------|
	//   left      right
	// 
	//for zoom = 1, map_size = 1024 x 1024
	//for point {750, 263} = bc
	//x = 750 / 256 = 2 = b10
	//y = 263 / 256 = 1 = b01
	//x determines left or right
	//y determines up or down
	//so path is {y[1], x[1]} + {y[0], x[0]} =  {01} + {10} = bc
	
	//TODO: is x = std::pow(2, zoom) * 256 a singularity? 100?
	merc.x = merc.x == std::pow(2, zoom) * 256 ? merc.x - 1 : merc.x;
	merc.y = merc.y == std::pow(2, zoom) * 256 ? merc.y - 1 : merc.y;

	std::bitset<32> bx = merc.x / 256;
	std::bitset<32> by = merc.y / 256;
	int x = merc.x / 256;
	int y = merc.y / 256;
	int stop = 1;
	for (size_t i = 0; i <= zoom; i++) {
		char c = 0;
		c |= bx[zoom - i] + (by[zoom - i] << 1);
		plate_path += c + 97;
	}
	return plate_path;
}

std::array<glm::vec3, 4> calculate_corners(std::string plate_path, size_t resolution) {
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	square s = path_to_square(plate_path);

	auto p0 = merc_to_ecef({s.x, s.y }, map_size);
	auto p1 = merc_to_ecef({s.x + 256, s.y}, map_size);
	auto p2 = merc_to_ecef({ s.x, s.y + 256 }, map_size);
	auto p3 = merc_to_ecef({ s.x + 256, s.y + 256 }, map_size);
	std::array<glm::vec3, 4> corners{ p0, p1, p2, p3 };
	return corners;
	
}
corner_normals calculate_corner_normals(std::string plate_path, size_t resolution) {
	square b = path_to_square(plate_path);
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	float step = ((float)b.a) / resolution;
	corner_normals cn;

	// resolution_ = 3, plate_path = generic 
	//   |<-----------b.a----------->|
	//	 v7-------v8--------v10------v11  -
	//	 |	 cn2  |		    |  cn3	 |    |
	//	 |        |		    |        |    |
	//	 v6-------|---------|--------v9   |
	//	 |<-step->|		    |		 |   b.a
	//	 |		  |		    |		 |    |
	//	 v2-------|---------|--------v5   |
	//	 |   cn0  |		    |  cn1   |    |
	//	 |        |		    |        |    |
	//	 v0-------v1--------v3-------v4   -
	//(b.x, b.y) 

	//for lower left corner
	glm::vec3 v0 = merc_to_ecef({ b.x,			b.y }, map_size);
	glm::vec3 v1 = merc_to_ecef({ b.x + step,	b.y }, map_size);
	glm::vec3 v2 = merc_to_ecef({ b.x,			b.y + step,}, map_size);

	//for lower right corner
	glm::vec3 v3 = merc_to_ecef({ b.x + b.a - step,	b.y, }, map_size);
	glm::vec3 v4 = merc_to_ecef({ b.x + b.a,		b.y, }, map_size);
	glm::vec3 v5 = merc_to_ecef({ b.x + b.a,		b.y + step, }, map_size);

	//for upper left corner
	glm::vec3 v6 = merc_to_ecef({ b.x,			b.y + b.a - step, }, map_size);
	glm::vec3 v7 = merc_to_ecef({ b.x,			b.y + b.a, }, map_size);
	glm::vec3 v8 = merc_to_ecef({ b.x + step,	b.y + b.a, }, map_size);

	//for upper right corner
	glm::vec3 v9 = merc_to_ecef({ b.x + b.a,			b.y + b.a - step, }, map_size);
	glm::vec3 v10 = merc_to_ecef({ b.x + b.a - step,	b.y + b.a, }, map_size);
	glm::vec3 v11 = merc_to_ecef({ b.x + b.a,			b.y + b.a, }, map_size);

	//anti clock wise
	cn[0] = glm::normalize(calc_normal(v0, v1, v2));
	cn[1] = glm::normalize(calc_normal(v4, v5, v3));
	cn[2] = glm::normalize(calc_normal(v7, v6, v8));
	cn[3] = glm::normalize(calc_normal(v11, v10, v9));

	return cn;
}
glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3) {
	return glm::cross(pt2 - pt1, pt3 - pt1);
}


double N(double phi){
	return earth_a * earth_a / sqrt(earth_a * earth_a * cos(phi) * cos(phi) + earth_b * earth_b * sin(phi) * sin(phi));
}

//note: don't reinvent the wheel
//https://danceswithcode.net/engineeringnotes/geodetic_to_ecef/geodetic_to_ecef.html
glm::vec3 ecef_to_lla(glm::vec3 ecef) {

	glm::vec3 geo{ 0, 0, 0 };			//Results go here (Lat, Lon, Altitude)
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
	//double n, lat, lon, alt;

	x = ecef.x;
	y = ecef.y;
	z = ecef.z;
	zp = std::abs(z);
	w2 = x * x + y * y;
	w = std::sqrt(w2);
	r2 = w2 + z * z;
	r = std::sqrt(r2);
	geo.y = std::atan2(y, x);       //Lon (final)
	s2 = z * z / r2;
	c2 = w2 / r2;
	u = a2 / r;
	v = a3 - a4 / r;
	if (c2 > 0.3) {
		s = (zp / r) * (1.0 + c2 * (a1 + u + s2 * v) / r);
		geo.x = std::asin(s);      //Lat
		ss = s * s;
		c = std::sqrt(1.0 - ss);
	}
	else {
		c = (w / r) * (1.0 - s2 * (a5 - u - c2 * v) / r);
		geo.x = std::acos(c);      //Lat
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
	geo.x = geo.x + p;      //Lat
	geo.z = f + m * p / 2.0;     //Altitude
	if (z < 0.0) {
		geo.x *= -1.0;     //Lat
	}
	//convert to angles
	geo *= 180 / glm::pi<double>();
	return(geo);    //Return Lat, Lon, Altitude in that order
}
glm::vec3 lla_to_ecef(glm::vec3 lla) {
	double lat = lla.x * glm::pi<double>() / 180;
	double lon = lla.y * glm::pi<double>() / 180;

	double x = N(lat) * cos(lat) * cos(lon);
	double y = N(lat) * cos(lat) * sin(lon);
	double z = (((earth_b * earth_b) / (earth_a * earth_a)) * N(lat)) * sin(lat);

	return { -x, -y, -z };
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
	ecef *= -1;

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
	//distance = circumference - distance;
	distance = (distance / circumference) * map_size;
	//sanity check
	distance = distance < 0 ? 0 : distance;
	distance = distance > map_size ? map_size : distance;

	return distance;
}
glm::vec2 lla_to_merc(glm::vec3 lla, double map_size) {
	return { lon_to_mercator_x(lla.y, map_size), lat_to_mercator_y(lla.x, map_size) };
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
		dphi = glm::pi<double>() / 2.0 - 2.0 * atan(ts * pow((1.0 - con) / (1.0 + con), e / 2.0)) - phi;
		phi += dphi;
		i++;
	}
	return (phi * 180.0 / glm::pi<double>());
}
glm::vec3 merc_to_ecef(glm::vec2 merc, double map_size){
	double lat = merc_y_to_lat(merc.y, map_size);
	double lon = merc_x_to_lon(merc.x, map_size);
	return geo_to_ecef({ lat * glm::pi<double>() / 180, lon * glm::pi<double>() / 180, 0 });
	//return lla_to_ecef(glm::vec3{ lat, lon, 0 });
}
glm::vec2 ecef_to_merc(glm::vec3 ecef, double map_size) {
	//auto lla = ecef_to_lla(ecef);
	//return { lon_to_mercator_x(lla.y, map_size), lat_to_mercator_y(lla.x, map_size)};
	return { 0, 0 };
}



//RAY CASTING
bool solve_quadratic(double a, double b, double c, float& t0, float& t1) {
	double discriminant = (b * b) - (4 * a * c);
	if (discriminant < 0) //no solution
		return false;

	if (discriminant == 0) { //1 solution
		t0 = -b / 2.0f * a;
	}
	else if (discriminant > 0) {//2 solutions;
		auto droot = std::sqrt(discriminant);
		t0 = (-b - droot) / (2.0f * a);
		t1 = (-b + droot) / (2.0f * a);
	}
	return true;
}

glm::vec3 sphere_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction) {
	double a = glm::dot(ray_direction, ray_direction);
	double b = glm::dot(ray_origin, ray_direction) * 2.0f;
	double c = glm::dot(ray_origin, ray_origin) - (earth_a * earth_a);

	float t0{ 0 }, t1{ 0 };
	if (!solve_quadratic(a, b, c, t0, t1))
		return glm::vec3(0, 0, 0);

	glm::vec3 hit1 = ray_origin + ray_direction * t0;
	glm::vec3 hit2 = ray_origin + ray_direction * t1;
	//reverse
	hit1 *= -1;

	return t0 > 0 ? hit1 : hit2;
}

glm::vec3 ellipsoid_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 radius)
{
	glm::vec3 ir2 = glm::vec3(1.0) / (radius * radius);
	double a = dot(ray_direction * ray_direction, ir2);
	double b = 2.0 * glm::dot(ray_direction * ray_origin, ir2);
	double c = glm::dot(ir2, ray_origin * ray_origin) - 1.0;

	float t0{ 0 }, t1{ 0 };
	if (!solve_quadratic(a, b, c, t0, t1))
		return glm::vec3(0, 0, 0);

	glm::vec3 hit1 = ray_origin + ray_direction * t0;
	glm::vec3 hit2 = ray_origin + ray_direction * t1;
	//reverse
	hit1 *= -1;

	return t0 > 0 ? hit1 : hit2;
}

glm::vec3 cast_ray(glm::vec2 mouse, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view, float dir) {
	glm::vec2 ndc_mouse{ ((mouse.x * 2) - viewport.x) / viewport.x, -((mouse.y * 2) - viewport.y) / viewport.y};
	glm::vec4 ray_clip{ ndc_mouse, dir, 1.0f };
	glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
	glm::vec4 ray_world = glm::inverse(view) * ray_eye;
	ray_world /= ray_world.w;
	return ray_world;
}
glm::vec3 point_to_world(glm::vec4 pt, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view) {
	glm::vec2 ndc_mouse{ ((pt.x * 2) - viewport.x) / viewport.x, -((pt.y * 2) - viewport.y) / viewport.y };
	glm::vec4 pt_clip{ ndc_mouse, pt.z, pt.w };
	glm::vec4 pt_eye = glm::inverse(projection) * pt_clip;
	glm::vec4 pt_world = glm::inverse(view) * pt_eye;
	pt_world /= pt_world.w;
	return pt_world;
}

glm::vec3 ray_hit_to_lla(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view) {
	auto lla = ecef_to_lla(ray_hit_ellipsoid(xy, viewport, projection, view));
	lla.x *= -1;
	return lla;
}
glm::vec3 ray_hit_ellipsoid(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view) {
	auto from = cast_ray(xy, viewport, projection, view, -1.0f);
	auto to = cast_ray(xy, viewport, projection, view, 1.0f);
	return ellipsoid_intersection(from, to - from, glm::vec3{earth_a, earth_a, earth_b });
}
glm::vec3 ray_hit_to_lla_ellipsoid(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view) {
	auto lla = ecef_to_lla(ray_hit_ellipsoid(xy, viewport, projection, view));
	lla.x *= -1;
	return lla;
}
double ray_hit_to_angle(glm::vec2 xy, glm::vec2 viewport, glm::vec3 camera_pos, glm::mat4 projection, glm::mat4 view) {
	auto edge_hit = ray_hit_ellipsoid(xy, viewport, projection, view);
	glm::vec3 cam = glm::vec4(camera_pos, 0.0) * view;
	return glm::angle(glm::normalize(edge_hit), glm::normalize(cam));
}
glm::vec2 ray_hit_to_merc(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view) {
	return { 0, 0 };
}
std::string ray_hit_to_path(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view, size_t zoom) {
	std::string plate_path;
	auto lla = ray_hit_to_lla(xy, viewport, projection, view);
	auto merc = lla_to_merc(lla, std::pow(2, zoom + 1) * 256);
	plate_path = merc_to_path(merc, zoom);

	return plate_path;
}


//MISC UTIL
double get_visible_angle_by_zoom(double zoom) {
	std::array<double, 19> angles = { 1.70, 1.65, 1.60, 1.30, 0.90, /*5*/0.30, 0.15, 0.07, 0.0350, 0.019, 
		/*10*/0.0069, 0.0030, 0.0015, 0.0014, 0.00007, 0.00035};
	return angles[zoom];
}

