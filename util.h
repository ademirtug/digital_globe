#pragma once

#include <array>
#include <math.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

double N(double phi);
glm::vec3 lla_to_ecef(double lat_indegrees, double lon_indegrees);
std::array<double, 3> ecef_to_geo(const std::array<double, 3>& ecef);

double lon_to_mercator_x(double lon, double mapsize = 1024);
double lat_to_mercator_y(double lat, double mapsize = 1024);

double merc_x_to_lon(double x, double map_size = 1024);
double merc_y_to_lat(double y, double map_size = 1024);

glm::vec3 lla_to_ecef(double lat_indegrees, double lon_indegrees);
glm::vec3 merc_to_ecef(glm::vec3 input, double map_size);

glm::vec3 calc_normal(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3);