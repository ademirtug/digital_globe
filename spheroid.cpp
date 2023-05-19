#include "spheroid.h"
//http://github.com/ademirtug/ecs_s/
#include "../../ecs_s/ecs_s.hpp"
#include "util.h"
#include "map_provider.h"
#include <glm/gtx/vector_angle.hpp>

void spheroid::process(ecs_s::registry& world, renderer_system& renderer) {
	//task: find visible plates
	//perfect solution to this question is probably 
	//sphere - sphere intersection
	
	//
	//|---------------------------------|
	//|l16            l17            l20|
	//|l14,l15                   l18,l19|
	//|                                 |
	//|   l10                    l13    |
	//|l8,l9                     l11,l12|
	//|                                 |
	//|   l3                       l7   |
	//|l1,l2          l4           l5,l6|
	//|---------------------------------|


	//default value is 60 degrees, only infinite distance 
	//can get 90, like in the directional lighting.
	constexpr float max_visible_angle = glm::pi<float>() / 2.7;
	auto vp = de2::get_instance().viewport;
	glm::vec2 l1{ 0, 0 }, l4{ vp.x / 2, 0 }, l6{ vp.x, 0 }, l8{ 0, vp.y / 2 }, l12{ vp.x, vp.y / 2 };
	glm::vec2 l16{ 0, vp.y }, l17{ vp.x / 2, vp.y }, l20{ vp.x, vp.y };

	std::vector<glm::vec2> corner_points;
	corner_points.insert(corner_points.end(), { l1, l6, l16, l20 });

	//we are still using sphere not spheroid
	//so -> hit_normal = glm::normalize(ecef)
	auto from = cast_ray(l8, { vp.x , vp.y }, renderer.get_projection(), renderer.get_view(), -1.0f);
	auto to = cast_ray(l8, { vp.x , vp.y }, renderer.get_projection(), renderer.get_view(), 1.0f);
	auto edge_hit = sphere_intersection(from, to - from);
	glm::vec3 cam = glm::vec4(renderer.cam_->get_world_pos(), 0.0) * renderer.get_view();
	float hit_angle = glm::angle(glm::normalize(edge_hit), glm::normalize(cam));
	hit_angle = std::isnan(hit_angle) ? max_visible_angle : hit_angle;
	auto hit_geo = ecef_to_geo({ edge_hit.x, edge_hit.y, edge_hit.z });


	//coords under mouse cursor
	auto mfrom = cast_ray(renderer.mouse_pos, de2::get_instance().viewport, renderer.get_projection(), renderer.get_view(), -1.0f);
	auto mto = cast_ray(renderer.mouse_pos, de2::get_instance().viewport, renderer.get_projection(), renderer.get_view(), 1.0f);
	auto mouse_hit = sphere_intersection(mfrom, mto - mfrom);
	auto mouse_geo = ecef_to_geo({ mouse_hit.x, mouse_hit.y, mouse_hit.z });



	//////////////////////////////////////////////////
	std::set<std::string> visible_hierarchy;
	std::queue<std::string> plates_to_check;
	//seed
	plates_to_check.push("a");
	plates_to_check.push("b");
	plates_to_check.push("c");
	plates_to_check.push("d");

	std::string sv = "(";

	while (!plates_to_check.empty()) {
		std::string plate_path = plates_to_check.front();
		plates_to_check.pop();

		auto cn = get_corner_normals(plate_path);
		bool is_visible = false;
		for (size_t i = 0; i < 4; i++){
			auto ca = glm::angle(cn[i], glm::normalize(glm::vec3{-cam.x, -cam.y, -cam.z }));
			if (plate_path == "da") {
				int b = 1;
			}
			if (ca < ( hit_angle * 1.50)) {
				is_visible = true;
				sv += plate_path + ":" + std::format("{:02.2f} ", ca);
				break;
			}
		}

		if (is_visible) {
			visible_hierarchy.emplace(plate_path);
			if (plate_path.size() < renderer.cam_->zoom_) {
				plates_to_check.push(plate_path + "a");
				plates_to_check.push(plate_path + "b");
				plates_to_check.push(plate_path + "c");
				plates_to_check.push(plate_path + "d");
			}
		}
	}
	sv += ")";

	//| m_hit -> ({:02.2f},{:02.2f},{:02.2f}) mouse_hit.x, mouse_hit.y, mouse_hit.z,
	//set title
	std::string s_mgeo = std::format("camera -> ({:02.2f},{:02.2f},{:02.2f}) | hit_angle -> ({:02.2f}) | sv -> ({}) | (sphere coords) -> ({:02.2f},{:02.2f})",
		cam.x, cam.y, cam.z, hit_angle,sv, -mouse_geo[0], mouse_geo[1]);
	de2::get_instance().set_title(s_mgeo);

	std::set<std::string> plates_to_request = visible_hierarchy;
	world.view<plate_name>([&](ecs_s::entity& e, plate_name& pn) {
		if (visible_hierarchy.find(pn.name) != visible_hierarchy.end()) {
			plates_to_request.erase(pn.name);
		}
	});

	//make async requests for missing plates in visible_hierarchy
	for (auto plate_path : plates_to_request) {
		if (requests_made_.find(plate_path) == requests_made_.end()) {
			//std::cout << "plate requested-> " << plate_path << std::endl;
			requests_made_[plate_path] = de2::get_instance().load_model_async<earth_plate>(plate_path, plate_path, resolution);
		}
	}

	evaluate_completed_requests(world);

	//remove all entities that is not in visible_hierarchy
	std::vector<ecs_s::entity> entities_to_remove;
	world.view<plate_name>([&](ecs_s::entity& e, plate_name& pn) {
		if (visible_hierarchy.find(pn.name) == visible_hierarchy.end()) {
			entities_to_remove.push_back(e);
		}
	});
	for (auto e : entities_to_remove) {
		world.remove_entity(e);
	}

	//iterate over all the plates and get a full quad tree
	std::set<std::string> plates_to_draw;
	std::map<std::string, size_t> node_count;
	for (auto plate_path : visible_hierarchy) {
		std::string plate_root = plate_path.substr(0, plate_path.size() - 1);
		if (node_count.find(plate_root) == node_count.end())
			node_count[plate_root] = 0;

		node_count[plate_path] = 1;
		node_count[plate_root]++;
	}

	for (auto kv : node_count) {
		if (kv.second < 4 && node_count[kv.first.substr(0, kv.first.size() - 1)] > 3) {
			plates_to_draw.emplace(kv.first);
		}
	}
	if (plates_to_draw.size() == 0)
		plates_to_draw = visible_hierarchy;

	world.truncate_component<visible>();
	world.view<plate_name>([&](ecs_s::entity& e, plate_name& pn) {
		if (std::find(plates_to_draw.begin(), plates_to_draw.end(), pn.name) != plates_to_draw.end()) {
			world.add_component(e, visible{});
		}
	});

};
void spheroid::evaluate_completed_requests(ecs_s::registry& world) {
	std::vector<std::string> completed_requests;
	for (auto it = requests_made_.begin(); it != requests_made_.end(); ++it) {
		if (it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			//TODO: object may never appear in the list after all, that means this function will block main thread!!!
			auto m = de2::get_instance().load_model<earth_plate>(it->first, it->first);
			m->upload();
			m->attach_program(de2::get_instance().programs["c_t_direct"]);

			auto ep = std::static_pointer_cast<earth_plate>(m);
			ecs_s::entity e = world.new_entity();
			world.add_component(e, plate_name{ it->first });
			world.add_component(e, m);
			cn_cache.put(it->first, std::static_pointer_cast<plate>(ep->m)->cn);
			completed_requests.push_back(it->first);
		}
	}
	for (auto plate_path : completed_requests) {
		requests_made_.erase(plate_path);
	}
}
corner_normals& spheroid::get_corner_normals(std::string plate_path) {
	if (!cn_cache.exists(plate_path)) {
		cn_cache.put(plate_path, calculate_corner_normals(plate_path, resolution)); 
	}
	return cn_cache.get(plate_path);
}
plate::plate(std::string plate_path, size_t resolution) : plate_path_(plate_path), resolution_(resolution) {
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	b = path_to_box(plate_path);
	int s = 0;
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
	// | a   |     |  bc |  bd |
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
				{ (x * step / b.a),  1 - (y * step / b.a)}/*uv*/
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
	cn = calculate_corner_normals(plate_path_, resolution_);
}

//EARTH PLATE
earth_plate::earth_plate(std::string plate_path, size_t resolution) {
	//std::cout << "earth_plate() -> " << plate_path << std::endl;
	m = std::make_shared<plate>(plate_path, resolution);
	tex = earth_plate::get_provider().get(plate_path);
	path_ = plate_path;
	m->name = plate_path;
}
earth_plate::~earth_plate() {
	glDeleteVertexArrays(1, &vao);
}
map_quest<disk_store>& earth_plate::get_provider() {
	static map_quest<disk_store> provider;
	return provider;
}