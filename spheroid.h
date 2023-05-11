#pragma once

#include "de2.h"
#include <iostream>
#include <set>
#include "glm/glm.hpp"
#include "util.h"
#include "map_provider.h"

struct earth {

};

struct plate_name {
	std::string name;
};

class plate : public mesh {
	size_t resolution_{ 0 };
public:
	plate(std::string plate_path, size_t resolution);
	box box_;
	std::string plate_path_;
};


class earth_plate : public texture_model {
public:
	earth_plate(std::string plate_path, size_t resolution = 16);
	~earth_plate();

	static map_quest<disk_store> get_provider() {
		static map_quest<disk_store> provider;
		return provider;
	}
};


class spheroid : public sub_system<size_t> {
public:
	spheroid(double semi_axis_a, double semi_axis_c) : a(semi_axis_a), c(semi_axis_c) {
	}

	void process(ecs_s::registry& world, size_t& level);
private:
	using rtype = decltype(std::declval<de2>().load_model_async<model>());
	double a{ .0f }, c{ .0f };
	std::map<std::string, rtype> requests_made_;
	std::vector<rtype> vertices;
};

