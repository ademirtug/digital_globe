#include "spheroid.h"
//http://github.com/ademirtug/ecs_s/
#include "../../ecs_s/ecs_s.hpp"
#include "util.h"
#include "map_provider.h"

void spheroid::process(ecs_s::registry& world, renderer_system& renderer) {
	std::map<std::string, size_t> node_count;
	std::vector<std::string> plates_to_draw;
	std::vector<std::string> plates;
	std::set<std::string> visible_plates;
	std::set<std::string> visible_roots;
	std::set<std::string> in, out;

	//We need to confirm two things...
	//1-is dot(normal, camera.pos()) < 90? 
	//2-is that point within NDC cube?
	//std::array<std::string, 4> pn{ "a", "b", "c", "d" };
	//in.emplace("a");
	//in.emplace("b");
	//in.emplace("c");
	//in.emplace("d");

	//std::string root = "";
	//for (size_t i = 0; i < renderer.cam_->zoom_; i++) {
	//	for (std::string plate : in){
	//		//now check is it facing to us or not?
	//		bool is_facing = false;
	//		corner_normals cn = get_corner_normals(plate);

	//		for (size_t z = 0; z < 4; z++){
	//			if (glm::dot(glm::normalize(renderer.cam_->getpos()), cn[z]) < 90) {
	//				is_facing = true;
	//				break;
	//			}
	//		}
	//		
	//	}
	//	//for (size_t x = 0; x < in.size(); x++) {
	//	//	std::string plate = in.;
	//	//	//now check is it facing to us or not?
	//	//	bool is_facing = false;

	//	//	corner_normals cn = get_corner_normals(plate);
	//	//}	
	//}


	//iterate over all the plates and get a full quad tree
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
	
	//make async requests for required plates
	for (size_t i = 0; i < plates_to_draw.size(); i++){
		if (!de2::get_instance().has_model(plates_to_draw[i]) && requests_made_.find(plates_to_draw[i]) == requests_made_.end()) {
			requests_made_[plates_to_draw[i]] = de2::get_instance().load_model_async<earth_plate>(plates_to_draw[i], plates_to_draw[i]);
		}
	}

	//evaluate any plate request that has been completed
	for (size_t i = 0; i < plates_to_draw.size(); i++) {
		if (requests_made_.find(plates_to_draw[i]) != requests_made_.end()) {
			if (requests_made_[plates_to_draw[i]].wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				requests_made_.erase(plates_to_draw[i]);
				//TODO: object may never appear in the list after all, that means this function will block main thread!!!

				auto m = de2::get_instance().load_model<earth_plate>(plates_to_draw[i], plates_to_draw[i]);
				m->upload();
				m->attach_program(de2::get_instance().programs["c_t_direct"]);
				
				auto ep = std::static_pointer_cast<earth_plate>(m);
				ecs_s::entity e = world.new_entity();
				world.add_component(e, plate_name{ plates_to_draw[i] });
				world.add_component(e, m);

				cn_cache.put(plates_to_draw[i], std::static_pointer_cast<plate>(ep->m)->cn);
			}
		}
	}
};
corner_normals& spheroid::get_corner_normals(std::string plate_path) {
	corner_normals cc;
	return cc;
}
plate::plate(std::string plate_path, size_t resolution) : plate_path_(plate_path), resolution_(resolution) {
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	b = path_to_box(plate_path);

	//  -map_size-
	// |---------|
	// |  c | d  |
	// |---------|
	// |  a | b  |
	// |---------|
	
	// vertices for resolution_ = 2, plate_path=b
	// |-----------------------|
	// | c         |d          |
	// |           |           |
	// |<---b.x--->|           |
	// |-----------6-----7-----8
	// |a    |     |  bc |  bd |
	// |	b.y    3-----4-----5
	// |     |     |  ba |  bb |
	// |-----------0-----1-----2
	for (size_t y = 0; y <= resolution_; y++) {
		for (size_t x = 0; x <= resolution_; x++) {
			double step = b.a / resolution_;
			 
			vertices.push_back({
				merc_to_ecef({ b.x + x * step, b.y + y * step, 0 }, map_size),/*3D position*/
				//{ b.x + x * step, b.y + y * step, 0 },/*2D position*/
				{ 0.0, 0.0, 0.0 },/*normal*/
				{ 1 - (x * step / b.a),  1 - (y * step / b.a)}/*uv*/
				});
		}
	}

	size_t point_per_row = resolution_ + 1;
	for (size_t y = 0; y < resolution_; y++) {
		for (size_t x = 0; x < resolution_; x++) {	
			// v2----v3
			// |     |
			// v0----v1
			//term: 2d array in a 1d pack
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

	//pre calculate corner normals;
	cn = calculate_corner_normals(plate_path_, resolution_, b);
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