#include "spheroid.h"
//http://github.com/ademirtug/ecs_s/
#include "ecs_s/ecs_s.hpp"
#include "util.h"
#include "map_provider.h"
#include <glm/gtx/vector_angle.hpp>

void spheroid::process(ecs_s::registry& world, renderer_system& renderer) {
	auto preferences = app_settings::instance().snapshot();
	auto revision = app_settings::instance().tile_revision();
	if (revision != settings_revision_) {
		for (auto& [path, request] : requests_made_) *request.wanted = false;
		requests_made_.clear(); retry_after_.clear();
		resident_images_.clear(); loaded_plates_.clear();
		world.truncate_component<visible>();
		for (auto& [path, entity] : drawn_plates_) world.get_component_value_for<std::shared_ptr<model>>(entity)->tex.reset();
		lod_clip_ = glm::dmat4(0.0);
		settings_revision_ = revision;
	}
	auto vp = de2::get_instance().viewport;
	auto projection = renderer.get_projection();
	auto view = renderer.cam_->view_matrix();
	bool changed = update_visible_plates(renderer);
	bool completed = evaluate_completed_requests();
	auto now = std::chrono::steady_clock::now();
	bool retry = std::any_of(retry_after_.begin(), retry_after_.end(), [now](const auto& item) { return item.second <= now; });
	if (changed || completed || retry) {
		std::set<std::string> leaves, ready;
		for (const auto& [leaf, demand] : visible_plates_) {
			leaves.insert(leaf);
			for (auto path = leaf; !path.empty(); path.pop_back()) {
				if (resident_images_.contains(path)) ready.insert(path);
				else if (loaded_plates_.exists(path)) {
					loaded_plates_.get(path);
					ready.insert(path);
				}
			}
		}
		auto requests = plan_plate_requests(visible_plates_, ready);
		std::set<std::string> wanted;
		for (const auto& request : requests) wanted.insert(request.path);
		for (auto& [path, request] : requests_made_) *request.wanted = wanted.contains(path);
		for (auto it = retry_after_.begin(); it != retry_after_.end();) {
			if (it->second <= now) it = retry_after_.erase(it); else ++it;
		}
		for (const auto& request : requests) {
			// Two workers, two outstanding jobs: no backlog of obsolete network work.
			if (requests_made_.size() >= 2) break;
			const auto& path = request.path;
			if (requests_made_.contains(path) || retry_after_.contains(path)) continue;
			auto wanted = std::make_shared<std::atomic_bool>(true);
			auto result = de2::get_instance().async([path, wanted, style = preferences.style, offline = preferences.offline]() -> std::shared_ptr<texture> {
				if (!wanted->load()) return {};
				return earth_plate::get_provider().get(path, style, offline);
			});
			requests_made_.emplace(path, image_request{ wanted, std::move(result) });
		}

		// Only meshes leaving the view are removed. Their loaded images stay cached.
		for (auto it = drawn_plates_.begin(); it != drawn_plates_.end();) {
			if (leaves.contains(it->first)) { ++it; continue; }
			world.remove_entity(it->second);
			it = drawn_plates_.erase(it);
		}
		for (const auto& path : leaves) {
			if (drawn_plates_.contains(path)) continue;
			auto tile = std::make_shared<texture_model>();
			tile->m = std::make_shared<plate>(path, resolution_);
			tile->attach_program(de2::get_instance().programs["c_t_direct"]);
			tile->upload();
			auto e = world.new_entity();
			world.add_component(e, plate_name{ path });
			world.add_component(e, std::static_pointer_cast<model>(tile));
			drawn_plates_.emplace(path, e);
		}

		// Uniform leaf geometry prevents cracks. Each region independently uses its
		// best loaded image; a missing edge tile never downgrades the entire globe.
		auto textures = select_plate_textures(leaves, ready);
		world.truncate_component<visible>();
		std::map<std::string, std::shared_ptr<texture>> resident;
		size_t lowest = tile_zoom_, highest = 0;
		for (const auto& [path, source] : textures) {
			auto e = drawn_plates_.at(path);
			auto& tile = world.get_component_value_for<std::shared_ptr<model>>(e);
			auto image = resident_images_.contains(source) ? resident_images_.at(source) : loaded_plates_.get(source);
			image->upload(); // Offscreen completions stay CPU-only until actually used.
			tile->tex = image;
			resident.emplace(source, image);
			auto region = plate_texture_region(path, source);
			tile->texture_offset = { region.x, region.y };
			tile->texture_scale = { region.z, region.w };
			world.add_component(e, visible{});
			lowest = std::min(lowest, source.size());
			highest = std::max(highest, source.size());
		}
		resident_images_ = std::move(resident);
		if (textures.empty()) lowest = 0;
		lowest_zoom_ = lowest; highest_zoom_ = highest; visible_count_ = textures.size();
	}

	auto coords = ray_hit_to_lla(renderer.mouse_pos, vp, projection, view);
	std::string location;
	if (renderer.mouse_pos.x >= 0 && renderer.mouse_pos.y >= 0 && renderer.mouse_pos.x < vp.x && renderer.mouse_pos.y < vp.y &&
		std::isfinite(coords.x) && std::isfinite(coords.y)) {
		if (preferences.decimal_coordinates)
			location = std::format("{:.5f}{} - {:.5f}{}", std::abs(coords.x), coords.x < 0 ? "S" : "N", std::abs(coords.y), coords.y < 0 ? "W" : "E");
		else location = std::format("{}{} - {}{}", dms(coords.x).to_string(), coords.x < 0 ? "S" : "N",
			dms(coords.y).to_string(), coords.y < 0 ? "W" : "E");
	}
	de2::get_instance().set_status(location);
}

bool spheroid::evaluate_completed_requests() {
	bool completed = false;
	for (auto it = requests_made_.begin(); it != requests_made_.end();) {
		if (it->second.result.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) { ++it; continue; }
		completed = true;
		try {
			auto image = it->second.result.get();
			if (image) loaded_plates_.put(it->first, image);
		}
		catch (const std::exception& error) {
			std::cerr << "tile " << it->first << ": " << error.what() << '\n';
			retry_after_[it->first] = std::chrono::steady_clock::now() + std::chrono::seconds(10);
		}
		it = requests_made_.erase(it);
	}
	return completed;
}

corner_normals& spheroid::get_corner_normals(std::string plate_path) {
	if (!cn_cache.exists(plate_path)) {
		cn_cache.put(plate_path, calculate_corner_normals(plate_path, resolution_)); 
	}
	return cn_cache.get(plate_path);
}
std::array<glm::vec3, 4> & spheroid::get_corners(std::string plate_path) {
	if (!c_cache.exists(plate_path)) {
		c_cache.put(plate_path, calculate_corners(plate_path, resolution_));
	}
	return c_cache.get(plate_path);
}


bool spheroid::update_visible_plates(renderer_system& renderer) {
	glm::dmat4 clip = glm::dmat4(renderer.get_projection()) * renderer.cam_->view_matrix();
	glm::dvec2 viewport = de2::get_instance().viewport;
	if (clip == lod_clip_ && viewport == lod_viewport_) return false;
	texel_pixels_ = view_texel_size(clip, viewport, earth_a, c);
	tile_zoom_ = select_map_zoom(texel_pixels_, tile_zoom_);
	lod_clip_ = clip;
	lod_viewport_ = viewport;
	plate_view view(renderer.cam_->world_position(), clip, { earth_a, earth_a, c });
	std::map<std::string, plate_demand> selected;
	std::queue<std::string> pending;
	for (auto root : { "a", "b", "c", "d" }) pending.push(root);
	while (!pending.empty()) {
		auto path = pending.front(); pending.pop();
		if (!bounds_cache.exists(path)) bounds_cache.put(path, calculate_bounds(path, earth_a, c));
		const auto& bounds = bounds_cache.get(path);
		if (!view.is_visible(bounds)) continue;
		if (path.size() < tile_zoom_) {
			for (char child : { 'a', 'b', 'c', 'd' }) pending.push(path + child);
			continue;
		}
		auto previous = visible_plates_.find(path);
		size_t current = previous == visible_plates_.end() ? 0 : previous->second.source.size();
		double density = plate_texel_size(bounds, clip, viewport);
		size_t level = std::isfinite(density) ? select_map_zoom(density, current, tile_zoom_) : tile_zoom_;
		selected.emplace(path, plate_demand{ path.substr(0, level), plate_screen_area(bounds, clip, viewport) });
	}
	visible_plates_ = std::move(selected);
	return true;
}

//PLATE
plate::plate(std::string plate_path, size_t resolution) : plate_path_(plate_path), resolution_(resolution) {
	size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
	b = path_to_square(plate_path);
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
			double step = double(b.a) / resolution_;
			auto position = merc_to_ecef({ b.x + x * step, b.y + y * step }, map_size);
			positions.push_back(position);
			 
			vertices.push_back({
				position,/*3D position*/
				//{ b.x + x * step, b.y + y * step, 0 },/*2D position*/
				{ 0.0, 0.0, 0.0 },/*normal*/
				{ (x * step / b.a),  1 - (y * step / b.a)}/*uv*/
				});
		}
	}

	origin = positions[positions.size() / 2];
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

		}
	}
	for (auto& v : vertices) {
		v.normal = glm::normalize(glm::dvec3(v.position) / glm::dvec3(earth_a * earth_a, earth_a * earth_a, earth_b * earth_b));
	}
	size_of_indices = indices.size();

	//pre calculate corner normals;
	cn = calculate_corner_normals(plate_path_, resolution_);
	corners = calculate_corners(plate_path_, resolution_);

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
}
open_street_map<disk_store>& earth_plate::get_provider() {
	static open_street_map<disk_store> provider;
	return provider;
}

