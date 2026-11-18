#pragma once

#include "de2.h"
#include <iostream>
#include <set>
#include "glm/glm.hpp"
#include "util.h"
#include "map_provider.h"

struct earth {};

struct plate_name {
	std::string name;
};

class plate : public mesh {
	size_t resolution_{ 0 };
public:
	plate(std::string plate_path, size_t resolution);
	
	square b;
	std::string plate_path_;
	corner_normals cn;
	std::array<glm::vec3, 4> corners;
};

class earth_plate : public texture_model {
public:
	earth_plate(std::string plate_path, size_t resolution);
	~earth_plate();

	static open_street_map<disk_store>& get_provider();
};


class spheroid : public sub_system<renderer_system> {
public:
	spheroid(double semi_axis_a, double semi_axis_c, size_t resolution) : earth_a(semi_axis_a), c(semi_axis_c), resolution_(resolution) {}
	~spheroid() { for (auto& [path, request] : requests_made_) *request.wanted = false; }
	void process(ecs_s::registry& world, renderer_system& renderer);


	corner_normals& get_corner_normals(std::string plate_path);
	std::array<glm::vec3, 4>& get_corners(std::string plate_path);
	size_t resolution_{ 16 };
private:
	uint64_t settings_revision_{ UINT64_MAX };

	bool update_visible_plates(renderer_system& renderer);
	bool evaluate_completed_requests();
	struct image_request {
		std::shared_ptr<std::atomic_bool> wanted;
		std::future<std::shared_ptr<texture>> result;
	};
	double earth_a{ .0f }, c{ .0f };
	std::map<std::string, image_request> requests_made_;
	std::map<std::string, std::chrono::steady_clock::time_point> retry_after_;
	//corner normals cache;
	lru_cache<std::string, corner_normals> cn_cache;
	lru_cache<std::string, std::array<glm::vec3, 4>> c_cache;
	lru_cache<std::string, plate_bounds> bounds_cache;
	lru_cache<std::string, std::shared_ptr<texture>, 512> loaded_plates_;
	// Pin images in use independently of the bounded cache of recent images.
	std::map<std::string, std::shared_ptr<texture>> resident_images_;
	std::map<std::string, plate_demand> visible_plates_;
	std::map<std::string, ecs_s::entity> drawn_plates_;
	glm::dmat4 lod_clip_{ 0.0 };
	glm::dvec2 lod_viewport_{ 0.0 };
	size_t tile_zoom_{ 0 };
	size_t lowest_zoom_{ 0 }, highest_zoom_{ 0 }, visible_count_{ 0 };
	double texel_pixels_{ 0.0 };
};

