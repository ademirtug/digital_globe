#pragma once

#include <array>
#include <math.h>
#include <cmath>
#include <string>
#include <format>
#include <limits>
#include <set>
#include <map>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/vector_angle.hpp>


double constexpr earth_a = 6378137.0;
double constexpr earth_b = 6356752.314245;
double constexpr circumference = 2 * glm::pi<double>() * earth_a;

struct square {
	size_t x{ 0 }, y{ 0 }, a{ 0 };
	bool hit_test(glm::vec2 p) {
		return (p.x >= x && p.x <= x + a
			&& p.y >= y && p.y <= y + a);
	}
};
struct corner_normals {
	std::array<glm::vec3, 4> data{ glm::vec3{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	glm::vec3& operator[](size_t idx) {
		return data[idx];
	}
};
struct dms {
	dms(double input) {
		if (!std::isfinite(input)) return;
		input = std::abs(input);
		degrees = input / 1;
		double degrees_decimal = input - degrees;
		minutes = degrees_decimal * 60 / 1;
		double  minutes_decimal = degrees_decimal * 60 - minutes;
		seconds = minutes_decimal * 60;
	}
	std::string to_string() {
		return std::format("{:02} {:02}' {:02}\"", degrees, minutes, seconds);
	}
	int degrees{ 0 }, minutes{ 0 }, seconds{ 0 };
};

//ray casting
struct plate_bounds {
	glm::dvec3 min, max;
	glm::dvec3 center, extent;
	glm::dmat3 axes{ 1.0 };
	double max_texel_metres{ 0 };
	double support(glm::dvec3 direction) const;
	bool is_visible(glm::dvec3 eye, const glm::dmat4& clip, glm::dvec3 radii) const;
};

struct plate_view {
	std::array<glm::dvec4, 6> planes;
	glm::dvec3 horizon;
	plate_view(glm::dvec3 eye, const glm::dmat4& clip, glm::dvec3 radii);
	bool is_visible(const plate_bounds& bounds) const;
};

struct plate_demand {
	std::string source;
	double pixels{ 0 };
};
struct plate_request {
	std::string path;
	double priority{ 0 };
	bool fallback{ false };
};
std::vector<plate_request> plan_plate_requests(const std::map<std::string, plate_demand>& demand, const std::set<std::string>& ready);
double plate_texel_size(const plate_bounds& bounds, const glm::dmat4& clip, glm::dvec2 viewport);
double plate_screen_area(const plate_bounds& bounds, const glm::dmat4& clip, glm::dvec2 viewport);

plate_bounds calculate_bounds(const std::string& path, double a = earth_a, double b = earth_b);
std::map<std::string, std::string> select_plate_textures(const std::set<std::string>& leaves, const std::set<std::string>& ready);
glm::vec4 plate_texture_region(const std::string& path, const std::string& source);
double projected_texel_size(glm::dvec3 point, const glm::dmat4& clip, glm::dvec2 viewport, size_t zoom = 0, double a = earth_a, double b = earth_b);
double view_texel_size(const glm::dmat4& clip, glm::dvec2 viewport, double a = earth_a, double b = earth_b);
size_t select_map_zoom(double texel_pixels, size_t current = 0, size_t maximum = 19);

bool solve_quadratic(double a, double b, double c, float& t0, float& t1);
glm::vec3 point_to_world(glm::vec4 pt, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view);
glm::vec2 ray_hit_to_merc(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view);
glm::dvec3 ellipsoid_intersection(glm::dvec3 ray_origin, glm::dvec3 ray_direction, glm::dvec3 radius);
glm::dvec3 ray_hit_ellipsoid(glm::dvec2 xy, glm::dvec2 viewport, glm::dmat4 projection, glm::dmat4 view);
glm::vec3 ray_hit_to_lla(glm::dvec2 xy, glm::dvec2 viewport, glm::dmat4 projection, glm::dmat4 view);
glm::dvec3 cast_ray(glm::dvec2 mouse, glm::dvec2 viewport, glm::dmat4 projection, glm::dmat4 view, double dir = -1.0);
std::string ray_hit_to_path(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view, size_t zoom);
double ray_hit_to_angle(glm::vec2 xy, glm::vec2 viewport, glm::vec3 camera_pos, glm::mat4 projection, glm::mat4 view);


//GIS functions
double N(double phi);
glm::vec3 ecef_to_lla(glm::vec3 ecef);
glm::dvec3 geo_to_ecef(glm::dvec3 geo);
double merc_x_to_lon(double x, double map_size = 1024);
double merc_y_to_lat(double y, double map_size = 1024);
glm::vec2 lla_to_merc(glm::vec3 lla, double map_size);
glm::dvec3 merc_to_ecef(glm::dvec2 merc, double map_size);
glm::vec2 ecef_to_merc(glm::vec3 ecef, double map_size);
double lon_to_mercator_x(double lon, double mapsize = 1024);
double lat_to_mercator_y(double lat, double mapsize = 1024);


//misc util
double get_visible_angle_by_zoom(double zoom);
square path_to_square(const std::string& plate_path);
std::string merc_to_path(glm::vec2 merc, double zoom);
glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3);
corner_normals calculate_corner_normals(std::string plate_path, size_t resolution);
std::array<glm::vec3, 4> calculate_corners(std::string plate_path, size_t resolution);
