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
	if (!std::isfinite(merc.x) || !std::isfinite(merc.y) || !std::isfinite(zoom) || zoom < 0 || zoom > 30) return "";
	size_t level = size_t(zoom) + 1;
	size_t count = size_t(1) << level;
	auto tile = [count](double p) { return size_t(std::clamp(p / 256, 0.0, double(count - 1))); };
	size_t x = tile(merc.x), y = tile(merc.y);
	std::string path;
	for (size_t i = level; i-- > 0;) {
		path += char('a' + ((x >> i) & 1) + 2 * ((y >> i) & 1));
	}
	return path;
}

std::array<glm::vec3, 4> calculate_corners(std::string plate_path, size_t resolution) {
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	square s = path_to_square(plate_path);

	auto p0 = merc_to_ecef({s.x, s.y }, map_size);
	auto p1 = merc_to_ecef({s.x + s.a, s.y}, map_size);
	auto p2 = merc_to_ecef({ s.x, s.y + s.a }, map_size);
	auto p3 = merc_to_ecef({ s.x + s.a, s.y + s.a }, map_size);
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
	return glm::normalize(glm::cross(pt2 - pt1, pt3 - pt1));
}


double N(double phi){
	return earth_a * earth_a / sqrt(earth_a * earth_a * cos(phi) * cos(phi) + earth_b * earth_b * sin(phi) * sin(phi));
}

//note: don't reinvent the wheel
//https://danceswithcode.net/engineeringnotes/geodetic_to_ecef/geodetic_to_ecef.html
glm::vec3 ecef_to_lla(glm::vec3 ecef) {
	if (!std::isfinite(ecef.x) || !std::isfinite(ecef.y) || !std::isfinite(ecef.z) || glm::dot(ecef, ecef) == 0) return glm::vec3(std::numeric_limits<float>::quiet_NaN());

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
	geo.x = glm::degrees(geo.x);
	geo.y = glm::degrees(geo.y);
	return(geo);    //Return Lat, Lon, Altitude in that order
}
glm::dvec3 geo_to_ecef(glm::dvec3 geo) {
	glm::dvec3 ecef;
	double e2 = 1 - earth_b * earth_b / (earth_a * earth_a);

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
double lat_to_mercator_y(double lat, double map_size) {
	// Plate paths use bottom-left origin; OSM XYZ uses top-left origin.
	lat = std::clamp(lat, -85.0511287798066, 85.0511287798066);
	double phi = glm::radians(lat);
	return std::clamp((0.5 + std::log(std::tan(glm::pi<double>() / 4 + phi / 2)) / (2 * glm::pi<double>())) * map_size, 0.0, map_size);
}
glm::vec2 lla_to_merc(glm::vec3 lla, double map_size) {
	return { lon_to_mercator_x(lla.y, map_size), lat_to_mercator_y(lla.x, map_size) };
}

double merc_x_to_lon(double merc_x, double map_size) {
	double lon = (360.0 / map_size * merc_x) - 180;
	return lon;
}
double merc_y_to_lat(double y, double map_size) {
	return glm::degrees(std::atan(std::sinh(2 * glm::pi<double>() * y / map_size - glm::pi<double>())));
}
glm::dvec3 merc_to_ecef(glm::dvec2 merc, double map_size){
	double lat = merc_y_to_lat(merc.y, map_size);
	double lon = merc_x_to_lon(merc.x, map_size);
	return geo_to_ecef({ lat * glm::pi<double>() / 180, lon * glm::pi<double>() / 180, 0 });
}
glm::vec2 ecef_to_merc(glm::vec3 ecef, double map_size) {
	return lla_to_merc(ecef_to_lla(ecef), map_size);
}

//RAY CASTING
bool solve_quadratic(double a, double b, double c, float& t0, float& t1) {
	if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c) || a == 0) return false;
	double discriminant = b * b - 4 * a * c;
	if (discriminant < 0) return false;
	double q = -0.5 * (b + std::copysign(std::sqrt(discriminant), b));
	double first = q == 0 ? -b / (2 * a) : q / a;
	double second = q == 0 ? first : c / q;
	t0 = float(std::min(first, second));
	t1 = float(std::max(first, second));
	return true;
}

glm::dvec3 ellipsoid_intersection(glm::dvec3 ray_origin, glm::dvec3 ray_direction, glm::dvec3 radius) {
	glm::dvec3 origin = glm::dvec3(ray_origin) / glm::dvec3(radius);
	glm::dvec3 direction = glm::dvec3(ray_direction) / glm::dvec3(radius);
	double a = glm::dot(direction, direction), b = glm::dot(direction, origin), c = glm::dot(origin, origin) - 1;
	double discriminant = b * b - a * c;
	if (a > 0 && discriminant >= 0 && std::isfinite(discriminant)) {
		double q = -b - std::copysign(std::sqrt(discriminant), b);
		double t0 = q == 0 ? -b / a : q / a, t1 = q == 0 ? t0 : c / q;
		if (t0 > t1) std::swap(t0, t1);
		double t = t0 >= 0 ? t0 : t1;
		if (t >= 0) return glm::dvec3(ray_origin) + t * glm::dvec3(ray_direction);
	}
	return glm::vec3(std::numeric_limits<float>::quiet_NaN());
}

glm::dvec3 cast_ray(glm::dvec2 mouse, glm::dvec2 viewport, glm::dmat4 projection, glm::dmat4 view, double dir) {
	if (viewport.x <= 0 || viewport.y <= 0) return glm::vec3(std::numeric_limits<float>::quiet_NaN());
	glm::dvec4 clip{ 2.0 * mouse.x / viewport.x - 1, 1 - 2.0 * mouse.y / viewport.y, dir, 1 };
	glm::dvec4 world = glm::inverse(glm::dmat4(projection) * glm::dmat4(view)) * clip;
	return glm::dvec3(world) / world.w;
}

glm::vec3 point_to_world(glm::vec4 pt, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view) {
	return cast_ray(pt, viewport, projection, view, pt.z);
}

glm::dvec3 ray_hit_ellipsoid(glm::dvec2 xy, glm::dvec2 viewport, glm::dmat4 projection, glm::dmat4 view) {
	auto from = cast_ray(xy, viewport, projection, view, -1.0f);
	auto to = cast_ray(xy, viewport, projection, view, 1.0f);
	return ellipsoid_intersection(from, to - from, { earth_a, earth_a, earth_b });
}

glm::vec3 ray_hit_to_lla(glm::dvec2 xy, glm::dvec2 viewport, glm::dmat4 projection, glm::dmat4 view) {
	return ecef_to_lla(ray_hit_ellipsoid(xy, viewport, projection, view));
}

double ray_hit_to_angle(glm::vec2 xy, glm::vec2 viewport, glm::vec3 camera_pos, glm::mat4 projection, glm::mat4 view) {
	auto hit = ray_hit_ellipsoid(xy, viewport, projection, view);
	if (!std::isfinite(hit.x)) return std::numeric_limits<double>::quiet_NaN();
	return std::acos(std::clamp(glm::dot(glm::normalize(glm::dvec3(hit)), glm::normalize(glm::dvec3(camera_pos))), -1.0, 1.0));
}

glm::vec2 ray_hit_to_merc(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view) {
	return lla_to_merc(ray_hit_to_lla(xy, viewport, projection, view), 1024);
}

std::string ray_hit_to_path(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view, size_t zoom) {
	auto lla = ray_hit_to_lla(xy, viewport, projection, view);
	return merc_to_path(lla_to_merc(lla, std::ldexp(256.0, int(zoom) + 1)), double(zoom));
}

//MISC UTIL
plate_bounds calculate_bounds(const std::string& path, double a, double b) {
	auto tile = path_to_square(path);
	double size = std::ldexp(256.0, int(path.size()));
	double south = merc_y_to_lat(double(tile.y), size);
	double north = merc_y_to_lat(double(tile.y + tile.a), size);
	double west = merc_x_to_lon(double(tile.x), size);
	double east = merc_x_to_lon(double(tile.x + tile.a), size);
	std::vector<double> latitudes{ south, north }, longitudes{ west, east };
	if (south < 0 && north > 0) latitudes.push_back(0);
	for (double lon = -180; lon <= 180; lon += 90) {
		if (lon > west && lon < east) longitudes.push_back(lon);
	}
	plate_bounds bounds{ glm::dvec3(std::numeric_limits<double>::max()), glm::dvec3(std::numeric_limits<double>::lowest()) };
	double e2 = 1 - b * b / (a * a);
	for (double lat : latitudes) {
		double phi = glm::radians(lat);
		double n = a / std::sqrt(1 - e2 * std::sin(phi) * std::sin(phi));
		for (double lon : longitudes) {
			double lambda = glm::radians(lon);
			glm::dvec3 p{ n * std::cos(phi) * std::cos(lambda), n * std::cos(phi) * std::sin(lambda), n * (1 - e2) * std::sin(phi) };
			bounds.min = glm::min(bounds.min, p);
			bounds.max = glm::max(bounds.max, p);
		}
	}
	// Leave room for float vertex rounding at Earth scale.
	bounds.min -= glm::dvec3(2);
	bounds.max += glm::dvec3(2);

	double phi = glm::radians((south + north) / 2), lambda = glm::radians((west + east) / 2);
	bounds.axes = {
		glm::dvec3(-std::sin(lambda), std::cos(lambda), 0),
		glm::dvec3(-std::sin(phi) * std::cos(lambda), -std::sin(phi) * std::sin(lambda), std::cos(phi)),
		glm::dvec3(std::cos(phi) * std::cos(lambda), std::cos(phi) * std::sin(lambda), std::sin(phi))
	};
	glm::dvec3 lo, hi;
	// Extrema of dot(axis, ECEF(lat,lon)). For a fixed longitude its latitude
	// derivative is M(lat) * (-k*sin(lat) + axis.z*cos(lat)). Include its zeros
	// and the longitude extrema, not just the corners or the parent's mesh.
	// This encloses the entire curved patch, including every finer descendant.
	for (int i = 0; i < 3; i++) {
		auto axis = bounds.axes[i];
		std::vector<double> lons{ glm::radians(west), glm::radians(east) };
		for (int j = -2; j <= 2; j++) {
			double lon = std::atan2(axis.y, axis.x) + j * glm::pi<double>();
			if (lon > lons[0] && lon < lons[1]) lons.push_back(lon);
		}
		lo[i] = std::numeric_limits<double>::max();
		hi[i] = std::numeric_limits<double>::lowest();
		for (double lon : lons) {
			double k = axis.x * std::cos(lon) + axis.y * std::sin(lon);
			std::vector<double> lats{ glm::radians(south), glm::radians(north) };
			for (int j = -1; j <= 1; j++) {
				double lat = std::atan2(axis.z, k) + j * glm::pi<double>();
				if (lat > lats[0] && lat < lats[1]) lats.push_back(lat);
			}
			for (double lat : lats) {
				double n = a / std::sqrt(1 - e2 * std::sin(lat) * std::sin(lat));
				double d = n * (k * std::cos(lat) + axis.z * (1 - e2) * std::sin(lat));
				lo[i] = std::min(lo[i], d); hi[i] = std::max(hi[i], d);
			}
		}
	}
	bounds.center = bounds.axes * ((lo + hi) * 0.5);
	bounds.extent = (hi - lo) * 0.5 + glm::dvec3(2);
	double closest_lat = std::clamp(0.0, south, north);
	// Upper bound on BOTH orthogonal Mercator tangents over this patch.
	bounds.max_texel_metres = 2 * glm::pi<double>() / 256 * a * a / b * std::cos(glm::radians(closest_lat));
	return bounds;
}

double plate_bounds::support(glm::dvec3 direction) const {
	double value = glm::dot(direction, center);
	for (int i = 0; i < 3; i++) value += extent[i] * std::abs(glm::dot(direction, axes[i]));
	return value;
}

plate_view::plate_view(glm::dvec3 eye, const glm::dmat4& clip, glm::dvec3 radii) : horizon(eye / (radii * radii)) {
	glm::dvec4 rows[4];
	for (int i = 0; i < 4; i++) rows[i] = { clip[0][i], clip[1][i], clip[2][i], clip[3][i] };
	for (int axis = 0; axis < 3; axis++) {
		planes[2 * axis] = rows[3] - rows[axis];
		planes[2 * axis + 1] = rows[3] + rows[axis];
	}
}

bool plate_view::is_visible(const plate_bounds& bounds) const {
	if (bounds.support(horizon) < 1) return false;
	for (const auto& plane : planes) if (bounds.support(glm::dvec3(plane)) + plane.w < 0) return false;
	return true;
}

bool plate_bounds::is_visible(glm::dvec3 eye, const glm::dmat4& clip, glm::dvec3 radii) const {
	return plate_view(eye, clip, radii).is_visible(*this);
}

namespace {
	bool plate_projection(const plate_bounds& bounds, const glm::dmat4& clip, glm::dvec2& lo, glm::dvec2& hi, double& min_w) {
		lo = glm::dvec2(std::numeric_limits<double>::max()); hi = -lo;
		glm::dvec4 w{ clip[0][3], clip[1][3], clip[2][3], clip[3][3] };
		min_w = -bounds.support(-glm::dvec3(w)) + w.w;
		if (min_w <= 0) return false;
		for (int corner = 0; corner < 8; corner++) {
			auto p = bounds.center;
			for (int axis = 0; axis < 3; axis++) p += bounds.axes[axis] * bounds.extent[axis] * ((corner & (1 << axis)) ? 1.0 : -1.0);
			auto q = clip * glm::dvec4(p, 1);
			auto ndc = glm::dvec2(q) / q.w;
			lo = glm::min(lo, ndc); hi = glm::max(hi, ndc);
		}
		return true;
	}
}

double plate_texel_size(const plate_bounds& bounds, const glm::dmat4& clip, glm::dvec2 viewport) {
	glm::dvec2 lo, hi;
	double min_w;
	if (!plate_projection(bounds, clip, lo, hi, min_w)) return std::numeric_limits<double>::infinity();
	// Bound the perspective Jacobian for every point in the box. The difference
	// from its central NDC value is a rank-one matrix, bounded by its norm.
	glm::dvec3 x{ clip[0][0], clip[1][0], clip[2][0] }, y{ clip[0][1], clip[1][1], clip[2][1] }, w{ clip[0][3], clip[1][3], clip[2][3] };
	auto midpoint = (lo + hi) * 0.5;
	auto u = (x - midpoint.x * w) * viewport.x * 0.5;
	auto v = (y - midpoint.y * w) * viewport.y * 0.5;
	double uu = glm::dot(u, u), vv = glm::dot(v, v), uv = glm::dot(u, v);
	double norm = std::sqrt((uu + vv + std::sqrt((uu - vv) * (uu - vv) + 4 * uv * uv)) * 0.5);
	double variation = glm::length((hi - lo) * viewport * 0.25) * glm::length(w);
	return bounds.max_texel_metres * (norm + variation) / min_w;
}

double plate_screen_area(const plate_bounds& bounds, const glm::dmat4& clip, glm::dvec2 viewport) {
	glm::dvec2 lo, hi;
	double min_w;
	if (!plate_projection(bounds, clip, lo, hi, min_w)) return viewport.x * viewport.y;
	auto span = glm::max(glm::dvec2(0), glm::min(hi, glm::dvec2(1)) - glm::max(lo, glm::dvec2(-1)));
	return span.x * span.y * viewport.x * viewport.y / 4;
}

std::vector<plate_request> plan_plate_requests(const std::map<std::string, plate_demand>& demand, const std::set<std::string>& ready) {
	std::map<std::string, plate_request> wanted;
	for (const auto& [leaf, item] : demand) {
		std::string best;
		for (auto path = leaf; !path.empty(); path.pop_back()) if (ready.contains(path)) { best = path; break; }
		if (best.size() >= item.source.size()) continue;
		if (best.empty()) {
			auto root = leaf.substr(0, 1);
			auto& fallback = wanted[root];
			fallback.path = root; fallback.fallback = true; fallback.priority += item.pixels;
		}
		auto& detail = wanted[item.source];
		detail.path = item.source;
		detail.priority += item.pixels * (1 - std::exp2(-double(item.source.size() - best.size())));
	}
	std::vector<plate_request> requests;
	for (const auto& [path, request] : wanted) requests.push_back(request);
	std::sort(requests.begin(), requests.end(), [](const auto& a, const auto& b) {
		if (a.fallback != b.fallback) return a.fallback;
		return a.priority == b.priority ? a.path < b.path : a.priority > b.priority;
	});
	return requests;
}

std::map<std::string, std::string> select_plate_textures(const std::set<std::string>& leaves, const std::set<std::string>& ready) {
	std::map<std::string, std::string> selected;
	for (const auto& path : leaves) {
		for (auto source = path; !source.empty(); source.pop_back()) {
			if (!ready.contains(source)) continue;
			selected.emplace(path, source);
			break;
		}
	}
	return selected;
}

glm::vec4 plate_texture_region(const std::string& path, const std::string& source) {
	if (source.empty() || !path.starts_with(source)) throw std::invalid_argument("texture source must be an ancestor of the plate");
	double count = std::ldexp(1.0, int(path.size() - source.size()));
	auto region = path_to_square(path.substr(source.size()));
	float scale = float(1 / count);
	// x/y are the texture offset; z/w are the scale. Texture V points downward.
	return { region.x / (256 * count), 1 - region.y / (256 * count) - scale, scale, scale };
}

double projected_texel_size(glm::dvec3 point, const glm::dmat4& clip, glm::dvec2 viewport, size_t zoom, double a, double b) {
	double e2 = 1 - b * b / (a * a);
	double longitude = std::atan2(point.y, point.x);
	double latitude = std::atan2(point.z, (1 - e2) * std::hypot(point.x, point.y));
	double sin_lat = std::sin(latitude), cos_lat = std::cos(latitude);
	double factor = 1 - e2 * sin_lat * sin_lat;
	double n = a / std::sqrt(factor);
	double m = a * (1 - e2) / std::pow(factor, 1.5);
	double radians_per_texel = 2 * glm::pi<double>() / std::ldexp(256.0, int(zoom));
	// Tangents of the ellipsoid with respect to Web Mercator image pixels.
	glm::dvec3 du = n * cos_lat * radians_per_texel * glm::dvec3(-std::sin(longitude), std::cos(longitude), 0);
	glm::dvec3 dv = m * cos_lat * radians_per_texel * glm::dvec3(-sin_lat * std::cos(longitude), -sin_lat * std::sin(longitude), cos_lat);
	auto p = clip * glm::dvec4(point, 1);
	if (p.w <= 0 || viewport.x <= 0 || viewport.y <= 0) return 0;
	auto project_tangent = [&](glm::dvec3 tangent) {
		auto d = clip * glm::dvec4(tangent, 0);
		return glm::dvec2(viewport.x, -viewport.y) * (glm::dvec2(d) * p.w - glm::dvec2(p) * d.w) / (2 * p.w * p.w);
	};
	auto u = project_tangent(du), v = project_tangent(dv);
	double trace = glm::dot(u, u) + glm::dot(v, v);
	double determinant = u.x * v.y - u.y * v.x;
	// Largest singular value: the most stretched direction of one source pixel.
	return std::sqrt((trace + std::sqrt(std::max(0.0, trace * trace - 4 * determinant * determinant))) / 2);
}

double view_texel_size(const glm::dmat4& clip, glm::dvec2 viewport, double a, double b) {
	if (viewport.x <= 0 || viewport.y <= 0) return 0;
	auto inverse = glm::inverse(clip);
	glm::dvec3 radii{ a, a, b };
	double largest = 0;
	// Screen samples use framebuffer pixels, so a HiDPI display requests more detail.
	for (int y = 0; y <= 20; y++) for (int x = 0; x <= 32; x++) {
		glm::dvec2 ndc{ 2.0 * x / 32 - 1, 1 - 2.0 * y / 20 };
		auto near_point = inverse * glm::dvec4(ndc, -1, 1);
		auto far_point = inverse * glm::dvec4(ndc, 1, 1);
		glm::dvec3 origin = glm::dvec3(near_point) / near_point.w;
		glm::dvec3 direction = glm::normalize(glm::dvec3(far_point) / far_point.w - origin);
		auto o = origin / radii, d = direction / radii;
		double qa = glm::dot(d, d), qb = glm::dot(o, d), qc = glm::dot(o, o) - 1;
		double discriminant = qb * qb - qa * qc;
		if (discriminant < 0) continue;
		double t = (-qb - std::sqrt(discriminant)) / qa;
		if (!std::isfinite(t) || t < 0) continue;
		auto point = origin + t * direction;
		double latitude = std::atan2(point.z, (b * b / (a * a)) * std::hypot(point.x, point.y));
		if (std::abs(latitude) > glm::radians(85.0511287798066)) continue;
		largest = std::max(largest, projected_texel_size(point, clip, viewport, 0, a, b));
	}
	return largest;
}

size_t select_map_zoom(double texel_pixels, size_t current, size_t maximum) {
	if (!std::isfinite(texel_pixels) || texel_pixels <= 0) return 1;
	// One source pixel should cover at most one framebuffer pixel.
	size_t wanted = size_t(std::clamp(std::ceil(std::log2(texel_pixels)), 1.0, double(maximum)));
	// Keep the finer level near a boundary instead of flickering during rotation.
	if (current > wanted && current <= maximum && std::ldexp(texel_pixels, 1 - int(current)) > 0.75) return current;
	return wanted;
}

double get_visible_angle_by_zoom(double zoom) {
	std::array<double, 19> angles = { 1.70, 1.65, 1.60, 1.30, 0.90, 
		/*5*/0.30, 0.15, 0.07, 0.0350, 0.0150, 
		/*10*/0.0082, 0.0040, 0.0015, 0.0014, 0.00007, 0.00035};
	return angles[zoom];
}

