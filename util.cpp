#include "util.h"
#include <algorithm>
#include <string>
#include "spheroid.h"
#include <math.h> 

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




double earth_a = 6.3781370f;
double earth_b = 6.3567523f;
double circumference = 2 * glm::pi<double>() * earth_a;

double N(double phi)
{
	return earth_a * earth_a / sqrt(earth_a * earth_a * cos(phi) * cos(phi) + earth_b * earth_b * sin(phi) * sin(phi));
}

//note: don'tex reinvent the wheel
//https://danceswithcode.net/engineeringnotes/geodetic_to_ecef/geodetic_to_ecef.html
std::array<double, 3> ecef_to_geo(const glm::vec3& ecef) {
	double earth_a = 6378137.0f;
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
	x = ecef.x;
	y = ecef.y;
	z = ecef.z;
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
	rg = earth_a / sqrt(g);
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
	double n = earth_a / sqrt(1 - e2 * sin(lat) * sin(lat));
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
double merc_y_to_lat(double merc_y, double map_size)
{
	auto y = circumference / map_size * (map_size / 2 - merc_y);
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
glm::vec3 lla_to_ecef(double lat_indegrees, double lon_indegrees)
{
	double lat = lat_indegrees * glm::pi<double>() / 180;
	double lon = lon_indegrees * glm::pi<double>() / 180;

	double x = N(lat) * cos(lat) * cos(lon);
	double y = N(lat) * cos(lat) * sin(lon);
	double z = (((earth_b * earth_b) / (earth_a * earth_a)) * N(lat)) * sin(lat);

	return { x, y, z };
}
glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3)
{
	return glm::cross(pt2 - pt1, pt3 - pt1);
}


bool solve_quadratic(float a, float b, float c, float& t0, float& t1) {

	float discriminant = std::pow(b, 2) - 4 * a * c;
	if (discriminant < 0) //no solution
		return false; 

	if (discriminant == 0) { //1 solution
		t0 = -b / 2.0f * a;
	}
	else if (discriminant > 0) {//2 solutions;
	
		t0 = -b - std::sqrtf(discriminant) / 2.0f * a;
		t1 = -b + std::sqrtf(discriminant) / 2.0f * a;
	}



	return true;
}

namespace ecef2blh {

	// 定数
	static double kPi = std::atanf(1.0) * 4.0;
	static double kPi180 = kPi / 180;
	// [ WGS84 座標パラメータ ]
	// a(地球楕円体長半径(赤道面平均半径))
	static constexpr double kA = 6378137.0;
	// 1 / f(地球楕円体扁平率=(a - b) / a)
	static constexpr double k1F = 298.257223563;
	// b(地球楕円体短半径)
	static constexpr double kB = kA * (1.0 - 1.0 / k1F);
	// e^2 = 2 * f - f * f
	//     = (a^2 - b^2) / a^2
	static constexpr double kE2 = (1.0 / k1F) * (2.0 - (1.0 / k1F));
	// e'^2= (a^2 - b^2) / b^2
	static constexpr double kEd2 = kE2 * kA * kA / (kB * kB);

	// 座標
	struct CoordB {
		double b;  // B(Beta)
		double l;  // L(Lambda)
		double h;  // H(Height)
	};
	struct CoordX {
		double x;  // X
		double y;  // Y
		double z;  // Z
	};

	/*
	 * @brief      関数 N
	 *
	 * @param[in]  X (double)
	 * @return     計算結果 (double)
	 */
	double n(double x) {
		double res;

		try {
			res = kA / sqrt(1.0 - kE2 * pow(sin(x * kPi180), 2));
		}
		catch (...) {
			throw;
		}

		return res;
	}

	/*
	 * @brief      ECEF -> BLH
	 *
	 * @param[in]  ECEF 座標 (CoordX)
	 * @return     BLH  座標 (CoordB)
	 */
	CoordB ecef2blh(CoordX c_src) {
		double p;
		double theta;
		CoordB c_res;

		try {
			p = sqrt(c_src.x * c_src.x + c_src.y * c_src.y);
			theta = atan2(c_src.z * kA, p * kB) / kPi180;
			c_res.b = atan2(
				c_src.z + kEd2 * kB * pow(sin(theta * kPi180), 3),
				p - kE2 * kA * pow(cos(theta * kPi180), 3)
			) / kPi180;                                          // Beta(Latitude)
			c_res.l = atan2(c_src.y, c_src.x) / kPi180;          // Lambda(Longitude)
			c_res.h = (p / cos(c_res.b * kPi180)) - n(c_res.b);  // Height
		}
		catch (...) {
			throw;
		}

		return c_res;
	}

}  // namespace ecef2blh

void sphere_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction) {
	double r = earth_a;

	//a = dot of ray origin -> dot(camera pos) or a mouse ray
	//b = ray direction -> 
	//r = radius -> earth_a
	//t = hit distance -> solutions
	glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2, glm::vec3(1, 0, 0));
	ray_origin = glm::vec3(glm::vec4(ray_origin, 1.0f) * rot_mat);
	ray_direction = glm::vec3(glm::vec4(ray_direction, 1.0f) * rot_mat);

	float a = glm::dot(ray_direction, ray_direction);
	float b = glm::dot(ray_origin, ray_direction) * 2.0f;
	float c = glm::dot(ray_origin, ray_origin) - std::pow(r, 2);

	float t0{ 0 }, t1{ 0 };
	if (!solve_quadratic(a, b, c, t0, t1))
		return;

	glm::vec3 hit1 = ray_origin * ray_direction * t0;
	glm::vec3 hit2 = ray_origin * ray_direction * t1;
	auto r1 = ecef_to_geo(hit1);
	auto r2 = ecef_to_geo(hit2);

	auto rx = t0 > 0 ? r1 : r2;
	//ecef2blh::CoordX c_src;
	//c_src.x = hit1.x;
	//c_src.y = hit1.y;
	//c_src.z = hit1.z;

	//ecef2blh::CoordB c_res;
	//c_res = ecef2blh::ecef2blh(c_src);

	
	/*de2::get_instance().set_title("lat: " + std::to_string(r1[0]) + ", lon:  " + std::to_string(r1[1]) + "---" + "lat: " + std::to_string(c_res.b) + ", lon:  " + std::to_string(c_res.l) );*/
	de2::get_instance().set_title("lat: " + std::to_string(rx[0]) + ", lon:  " + std::to_string(rx[1]));
	int stop = 1;

}