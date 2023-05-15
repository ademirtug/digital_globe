#include "spheroid.h"
#include "../../ecs_s/ecs_s.hpp"
#include "util.h"
#include "map_provider.h"

void spheroid::process(ecs_s::registry& world, size_t& level) {
	std::map<std::string, size_t> node_count;
	std::vector<std::string> plates_to_draw;
	std::vector<std::string> plates;

	world.view<plate_name>([&node_count](ecs_s::entity& e, plate_name& pn) {
		std::string plate_root = pn.name.substr(0, pn.name.size() - 1);
		if (node_count.find(plate_root) == node_count.end())
			node_count[plate_root] = 0;

		node_count[pn.name] = 1;
		node_count[plate_root]++;
		});

	for (auto kv : node_count) {
		if (kv.second < 4 && node_count[kv.first.substr(0, kv.first.size() - 1)] > 3) {
			plates_to_draw.push_back(kv.first);
		}
	}
	if (plates_to_draw.size() == 0) {
		plates_to_draw.push_back("a");
		plates_to_draw.push_back("b");
		plates_to_draw.push_back("c");
		plates_to_draw.push_back("d");
	}
	
	for (size_t i = 0; i < plates_to_draw.size(); i++){
		if (!de2::get_instance().has_model(plates_to_draw[i]) && requests_made_.find(plates_to_draw[i]) == requests_made_.end()) {
			requests_made_[plates_to_draw[i]] = de2::get_instance().load_model_async<earth_plate>(plates_to_draw[i], plates_to_draw[i]);
		}
	}

	for (size_t i = 0; i < plates_to_draw.size(); i++) {
		if (requests_made_.find(plates_to_draw[i]) != requests_made_.end()) {
			if (requests_made_[plates_to_draw[i]].wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				requests_made_.erase(plates_to_draw[i]);
				//TODO: object may never appear in the list after all, that means this function will block thread
				ecs_s::entity e = world.new_entity();

				auto m = de2::get_instance().load_model<earth_plate>(plates_to_draw[i], plates_to_draw[i]);
				m->upload();
				m->attach_program(de2::get_instance().programs["c_t_direct"]);
				world.add_component(e, plate_name{ plates_to_draw[i] });
				world.add_component(e, m);
			}
		}
	}
};

plate::plate(std::string plate_path, size_t resolution) : plate_path_(plate_path), resolution_(resolution) {
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	box_ = path_to_box(plate_path);

	//  -map_size-
	// |---------|
	// |  c | d  |
	// |---------|
	// |  earth_a | earth_b  |
	// |---------|
	
	// vertices for resolution_ = 2, plate_path=earth_b
	// |-----------------------|
	// |           |     |     |
	// |     c     |   box_.y  |
	// |           |     |     |
	// |-----------6-----7-----8
	// |           |  bc |  bd |
	// |---box_.x--3-----4-----5
	// |           |  ba |  bb |
	// |-----------0-----1-----2
	for (size_t y = 0; y <= resolution_; y++) {
		for (size_t x = 0; x <= resolution_; x++) {
			double step = box_.earth_a / resolution_;
			 
			vertices.push_back({
				merc_to_ecef({ box_.x + x * step, box_.y + y * step, 0 }, map_size),/*3D position*/
				//{ box_.x + x * step, box_.y + y * step, 0 },/*2D position*/
				{ 0.0, 0.0, 0.0 },/*normal*/
				{ 1 - (x * step / box_.earth_a),  1 - (y * step / box_.earth_a)}/*uv*/
				});
		}
	}

	size_t point_per_row = resolution_ + 1;
	for (size_t y = 0; y < resolution_; y++) {
		for (size_t x = 0; x < resolution_; x++) {	
			// v2----v3
			// |     |
			// v0----v1
			//term: 2d array in earth_a 1d pack
			int v0 = x + (point_per_row * y);
			int v1 = x + 1 + (point_per_row * y);
			int v2 = x + (point_per_row * (y+1));
			int v3 = x + 1 + (point_per_row * (y + 1));

			//lower left triangle - anti-clockwise
			indices.insert(indices.end(), { v0, v1, v2});
			//upper right triangle - anti-clockwise
			indices.insert(indices.end(), { v1, v3 ,v2 });

			//TODO: there is something wrong with specular lighting
			auto n1 = calc_normal(vertices[v0].position, vertices[v1].position, vertices[v2].position);
			auto n2 = calc_normal(vertices[v1].position, vertices[v3].position, vertices[v0].position);
			auto n3 = calc_normal(vertices[v2].position, vertices[v0].position, vertices[v3].position);
			auto n4 = calc_normal(vertices[v3].position, vertices[v2].position, vertices[v1].position);

			vertices[v0].normal = n1;
			vertices[v1].normal = n2;
			vertices[v2].normal = n3;
			vertices[v3].normal = n4;
		}
	}
	size_of_indices = indices.size();
}

//EARTH PLATE
earth_plate::earth_plate(std::string plate_path, size_t resolution) {
	m = std::make_shared<plate>(plate_path, resolution);
	tex = earth_plate::get_provider().get(plate_path);
	path_ = plate_path;
}
earth_plate::~earth_plate() {
	glDeleteVertexArrays(1, &vao);
}

map_quest<disk_store>& earth_plate::get_provider() {
	static map_quest<disk_store> provider;
	return provider;
}