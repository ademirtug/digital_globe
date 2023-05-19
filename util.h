#pragma once

#include <array>
#include <math.h>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/vector_angle.hpp>


double constexpr earth_a = 6.3781370f;
double constexpr earth_b = 6.3567523f;
double constexpr circumference = 2 * glm::pi<double>() * earth_a;

struct box {
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


box path_to_box(const std::string& plate_path);
corner_normals calculate_corner_normals(std::string plate_path, size_t resolution);

//ray casting
glm::vec3 ray_hit(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view);
glm::vec3 ray_hit_to_geo(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view);
glm::vec2 ray_hit_to_merc(glm::vec2 xy, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view);
double ray_hit_to_angle(glm::vec2 xy, glm::vec2 viewport, glm::vec3 camera_pos, glm::mat4 projection, glm::mat4 view);
glm::vec3 cast_ray(glm::vec2 mouse, glm::vec2 viewport, glm::mat4 projection, glm::mat4 view, float dir = -1.0f);
bool solve_quadratic(float a, float b, float c, float& t0, float& t1);
glm::vec3 sphere_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction);


//GIS functions
double N(double phi);
glm::vec3 ecef_to_lla(glm::vec3 ecef);
glm::vec3 lla_to_ecef(double lat_indegrees, double lon_indegrees);
glm::vec3 geo_to_ecef(glm::vec3 geo);

double lon_to_mercator_x(double lon, double mapsize = 1024);
double lat_to_mercator_y(double lat, double mapsize = 1024);

double merc_x_to_lon(double x, double map_size = 1024);
double merc_y_to_lat(double y, double map_size = 1024);

glm::vec2 lla_to_merc(glm::vec3 lla, double map_size);

glm::vec3 merc_to_ecef(glm::vec2 merc, double map_size);
glm::vec2 ecef_to_merc(glm::vec3 ecef, double map_size);

glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3);


//misc util
double get_visible_angle_by_zoom(double zoom);


