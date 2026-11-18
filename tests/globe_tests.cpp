#include "spheroid.h"
#include <cstdlib>

void check(bool condition, const char* message) {
	if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}

glm::dvec2 screen_position(const camera& cam, glm::dvec3 point, glm::dvec2 viewport, const glm::dmat4& projection) {
	auto clip = projection * cam.view_matrix() * glm::dvec4(point, 1);
	return { (clip.x / clip.w + 1) * viewport.x / 2, (1 - clip.y / clip.w) * viewport.y / 2 };
}

int main() {
	for (double latitude : { -85.0, -60.0, -30.0, 0.0, 30.0, 60.0, 85.0 }) {
		check(std::abs(merc_y_to_lat(lat_to_mercator_y(latitude, 4096), 4096) - latitude) < 1e-8, "Mercator latitude round trip");
		for (double longitude : { -179.0, -90.0, 0.0, 90.0, 179.0 }) {
			auto p = geo_to_ecef({ glm::radians(latitude), glm::radians(longitude), 0 });
			auto lla = ecef_to_lla(p);
			check(std::abs(lla.x - latitude) < 0.0001 && std::abs(lla.y - longitude) < 0.0001, "ECEF latitude and longitude round trip");
		}
	}
	camera cam;
	auto view = cam.get_view();
	auto north = geo_to_ecef({ glm::radians(30.0), 0, 0 });
	auto south = geo_to_ecef({ glm::radians(-30.0), 0, 0 });
	check((view * glm::vec4(north, 1)).y > (view * glm::vec4(south, 1)).y, "north appears above south");
	float t0, t1;
	check(solve_quadratic(1, -6, 9, t0, t1) && t0 == 3 && t1 == 3, "both tangent roots are initialized");
	check(glm::distance(ellipsoid_intersection({ 3, 0, 0 }, { -1, 0, 0 }, { 1, 1, 1 }), glm::dvec3(1, 0, 0)) < 1e-5, "ray returns the near surface, not its antipode");
	check(!std::isfinite(ellipsoid_intersection({ 3, 0, 0 }, { 1, 0, 0 }, { 1, 1, 1 }).x), "intersection behind the ray is a miss");
	auto projection = glm::perspective(glm::radians(55.0f), 1.6f, 100.0f, 50000000.0f);
	check(std::isfinite(ray_hit_to_lla({ 640, 400 }, { 1280, 800 }, projection, view).x), "center picking is finite");
	check(ray_hit_to_path({ 0, 0 }, { 1280, 800 }, projection, view, 2).empty(), "sky picking has no tile path");
	check(merc_to_path({ 1024, 1024 }, 1) == "dd", "map boundary stays in the last tile");
	auto back = merc_to_path(lla_to_merc({ 0, 179, 0 }, 4096), 3);
	check(!calculate_bounds(back).is_visible(cam.get_world_pos(), glm::dmat4(projection) * glm::dmat4(view), { earth_a, earth_a, earth_b }), "horizon rejects tiles on the far side");
	auto front = merc_to_path(lla_to_merc({ 1, 1, 0 }, 4096), 3);
	auto away = glm::lookAt(cam.get_world_pos(), 2.0f * cam.get_world_pos(), glm::vec3(0, 0, 1));
	check(!calculate_bounds(front).is_visible(cam.get_world_pos(), glm::dmat4(projection) * glm::dmat4(away), { earth_a, earth_a, earth_b }), "frustum rejects tiles behind the camera");

	const std::set<std::string> leaves{ "aa", "ab", "ac", "ad", "ba", "bb", "bc", "bd" };
	std::set<std::string> ready{ "a", "aa", "ab", "ac", "ad", "b", "ba", "c" };
	auto sources = select_plate_textures(leaves, ready);
	check(sources.size() == leaves.size(), "fallback images cover every requested geometry tile");
	check(sources.at("aa") == "aa" && sources.at("ad") == "ad" && sources.at("ba") == "ba", "one missing tile cannot downgrade loaded neighbours");
	check(sources.at("bb") == "b" && sources.at("bc") == "b", "only unloaded regions use ancestor imagery");
	for (const auto& path : { "bb", "bd", "bc" }) {
		ready.insert(path);
		auto next = select_plate_textures(leaves, ready);
		for (const auto& [leaf, old_source] : sources) check(next.at(leaf).size() >= old_source.size(), "async image arrivals only upgrade existing regions");
		sources = next;
	}
	for (const auto& leaf : leaves) check(sources.at(leaf) == leaf, "all tiles eventually use their own high resolution image");
	for (int frame = 0; frame < 120; frame++) {
		auto rotating = leaves;
		if (frame % 2) rotating.insert("ca");
		auto next = select_plate_textures(rotating, ready);
		for (const auto& leaf : leaves) check(next.at(leaf) == leaf, "newly visible missing tiles never cause whole-view resolution flicker");
		check(next.size() == rotating.size(), "newly exposed regions retain parent coverage while loading");
		if (rotating.contains("ca")) check(next.at("ca") == "c", "fallback is restricted to the newly exposed region");
	}
	check(select_plate_textures({ "aaaa" }, { "a", "aa" }).at("aaaa") == "aa", "fallback uses the closest loaded ancestor");
	check(select_plate_textures({ "d" }, { "a" }).empty(), "unrelated imagery is never displayed");
	check(plate_texture_region("aa", "a") == glm::vec4(0, 0.5, 0.5, 0.5), "lower-left child selects the correct parent texture quadrant");
	check(plate_texture_region("ab", "a") == glm::vec4(0.5, 0.5, 0.5, 0.5), "lower-right child selects the correct parent texture quadrant");
	check(plate_texture_region("ac", "a") == glm::vec4(0, 0, 0.5, 0.5), "upper-left child selects the correct parent texture quadrant");
	check(plate_texture_region("ad", "a") == glm::vec4(0.5, 0, 0.5, 0.5), "upper-right child selects the correct parent texture quadrant");
	check(plate_texture_region("aba", "a") == glm::vec4(0.5, 0.75, 0.25, 0.25), "ancestor cropping composes through multiple levels");
	check(plate_texture_region("abc", "abc") == glm::vec4(0, 0, 1, 1), "a tile's own image needs no texture transform");
	{
		std::map<std::string, plate_demand> demand{
			{ "aaaa", { "aaa", 400 } }, { "aaab", { "aaa", 200 } },
			{ "baaa", { "baaa", 900 } }
		};
		auto cold = plan_plate_requests(demand, {});
		check(cold.size() == 4 && cold[0].path == "b" && cold[1].path == "a", "cold regions request one coarse fallback each, prioritized by visible area");
		check(cold[0].fallback && cold[1].fallback && !cold[2].fallback, "coverage precedes detail without fetching every intermediate level");
		auto covered = plan_plate_requests(demand, { "a", "b" });
		check(covered.size() == 2 && covered[0].path == "baaa" && covered[1].path == "aaa", "covered regions request deduplicated local detail in screen benefit order");
		check(plan_plate_requests(demand, { "aaaa", "aaab", "baaa" }).empty(), "already resident finer imagery avoids unnecessary coarse downloads");
		check(plan_plate_requests({}, { "a", "b" }).empty(), "leaving the view removes obsolete requests");
	}
	{
		lru_cache<std::string, int, 3> images;
		images.put("a", 1); images.put("b", 2); images.put("c", 3);
		check(images.get("a") == 1, "returning to a recently visited tile reuses its cached image");
		images.put("d", 4);
		check(images.exists("a") && !images.exists("b"), "image retention is bounded and evicts the least recently used tile");
	}

	for (auto path : { "a", "b", "c", "d", "abc", "ddc", "bacbd" }) {
		plate mesh(path, 8);
		auto bounds = calculate_bounds(path);
		for (const auto& v : mesh.vertices) {
			for (int axis = 0; axis < 3; axis++) check(v.position[axis] >= bounds.min[axis] && v.position[axis] <= bounds.max[axis], "tile bounds contain every vertex");
			auto local = glm::transpose(bounds.axes) * (glm::dvec3(v.position) - bounds.center);
			for (int axis = 0; axis < 3; axis++) check(std::abs(local[axis]) <= bounds.extent[axis], "oriented bounds contain every vertex");
			check(glm::dot(v.position, v.normal) > 0, "surface normal faces outward");
		}
		for (size_t i = 0; i < mesh.indices.size(); i += 3) {
			glm::dvec3 a = mesh.vertices[mesh.indices[i]].position;
			glm::dvec3 b = mesh.vertices[mesh.indices[i + 1]].position;
			glm::dvec3 c = mesh.vertices[mesh.indices[i + 2]].position;
			check(glm::dot(glm::cross(b - a, c - a), a + b + c) > 0, "triangle winding survives back-face culling");
		}
	}
	plate left("aa", 8), right("ab", 8);
	for (int row = 0; row <= 8; row++) check(glm::distance(left.vertices[row * 9 + 8].position, right.vertices[row * 9].position) < 0.01, "adjacent tile edges coincide");
	check(left.vertices.size() == 81, "polar openings remain outside Mercator coverage");
	// Sample much more finely than the render mesh: parent bounds must contain
	// the curved surface, including descendants between the parent's vertices.
	for (auto path : { "a", "b", "c", "d", "abc", "ddc", "bacbd", "dddddddd", "aaaaaaaa" }) {
		auto bounds = calculate_bounds(path);
		plate dense(path, 63);
		for (const auto& vertex : dense.vertices) {
			auto local = glm::transpose(bounds.axes) * (glm::dvec3(vertex.position) - bounds.center);
			for (int axis = 0; axis < 3; axis++) check(std::abs(local[axis]) <= bounds.extent[axis], "analytic parent bounds include finely sampled curved descendants");
		}
	}

	{
		glm::dvec2 viewport{ 1280, 800 };
		double height = 7500000;
		auto p = glm::perspective(glm::radians(55.0), viewport.x / viewport.y, 100.0, 50000000.0);
		auto v = glm::lookAt(glm::dvec3(earth_a + height, 0, 0), glm::dvec3(0), glm::dvec3(0, 0, 1));
		auto clip = p * v;
		double expected = 2 * glm::pi<double>() * earth_a / 256 * (viewport.y * p[1][1] / 2) / height;
		double actual = projected_texel_size({ earth_a, 0, 0 }, clip, viewport);
		check(std::abs(actual / expected - 1) < 1e-9, "projected texel density agrees with the ground-resolution formula at the equator");
		auto high_eye = glm::lookAt(glm::dvec3(earth_a + height * 2, 0, 0), glm::dvec3(0), glm::dvec3(0, 0, 1));
		check(std::abs(projected_texel_size({ earth_a, 0, 0 }, p * high_eye, viewport) * 2 / actual - 1) < 1e-9, "doubling camera height halves source-pixel magnification at nadir");
		double phi = glm::radians(60.0);
		glm::dvec3 surface = earth_a * glm::dvec3(std::cos(phi), 0, std::sin(phi));
		auto polar_view = glm::lookAt(surface * (1 + height / earth_a), glm::dvec3(0), glm::dvec3(0, 0, 1));
		double polar_size = projected_texel_size(surface, p * polar_view, viewport, 0, earth_a, earth_a);
		check(std::abs(polar_size / (actual * std::cos(phi)) - 1) < 1e-9, "Mercator texture density accounts for latitude");
		double native = view_texel_size(clip, viewport);
		double hidpi = view_texel_size(clip, viewport * 2.0);
		check(std::abs(hidpi / native - 2) < 1e-9, "double framebuffer resolution doubles the required texture density");
		check(select_map_zoom(hidpi) == select_map_zoom(native) + 1, "HiDPI requires one finer map level");
		check(select_map_zoom(16.01, 4) == 5 && select_map_zoom(15.99, 5) == 5, "LOD hysteresis prevents zoom flicker near a density boundary");
		check(select_map_zoom(11.9, 5) == 4, "LOD coarsens once the lower level has enough pixels");
		check(select_map_zoom(1e12) == 19 && select_map_zoom(0) == 1, "tile zoom respects provider and empty-view limits");
		double worst = 0;
		for (int zoom : { 0, 2, 5, 10, 15 }) {
			for (double pitch : { -1.4, 0.0, 0.7, 1.4 }) {
				double distance = earth_a + std::ldexp(30000000.0, -zoom);
				glm::dvec3 eye = distance * glm::dvec3(std::cos(pitch), 0, std::sin(pitch));
				auto view_matrix = glm::lookAt(eye, glm::dvec3(0), glm::dvec3(0, 0, 1));
				for (double scale : { 1.0, 2.0, 3.0 }) {
					auto pixels = viewport * scale;
					auto matrix = p * view_matrix;
					size_t map_zoom = select_map_zoom(view_texel_size(matrix, pixels));
					// Independent, denser ray samples verify the selected level between LOD samples.
					for (int y = 0; y <= 32; y++) for (int x = 0; x <= 48; x++) {
						auto point = ray_hit_ellipsoid({ x * pixels.x / 48, y * pixels.y / 32 }, pixels, p, view_matrix);
						if (!std::isfinite(point.x)) continue;
						auto lla = ecef_to_lla(point);
						if (std::abs(lla.x) > 85.0511287798066) continue;
						double stretch = projected_texel_size(point, matrix, pixels, map_zoom);
						worst = std::max(worst, stretch);
						check(stretch <= 1.01, "selected raster level avoids source-pixel enlargement across the viewport");
					}
				}
			}
		}
		std::cout << "Texture density: default map zoom " << select_map_zoom(view_texel_size(glm::dmat4(projection) * glm::dmat4(cam.get_view()), viewport))
			<< ", worst sampled source-pixel stretch " << worst << " screen pixels.\n";
	}

	{
		renderer_system street;
		for (int i = 0; i < 40; i++) street.cam_->scroll(1);
		check(street.cam_->zoom_ == camera::maximum_zoom, "scrolling reaches the new street-level limit and clamps safely");
		check(std::abs(street.cam_->height() - 57.220458984375) < 1e-9, "maximum zoom reaches approximately 57 metres above the surface");
		for (size_t zoom : { size_t(15), size_t(16), size_t(17), size_t(18), size_t(19) }) {
			street.cam_->zoom_ = zoom;
			auto eye = glm::dvec3(street.cam_->get_world_pos());
			auto radial = glm::normalize(eye);
			double surface_radius = 1 / glm::length(radial / glm::dvec3(earth_a, earth_a, earth_b));
			check(std::abs(glm::length(eye) - surface_radius - street.cam_->height()) < 0.6, "camera height follows the local ellipsoid, without the equatorial-radius floor");
			auto p = street.get_projection();
			auto v = street.cam_->get_view();
			auto clip = glm::dmat4(p) * glm::dmat4(v);
			auto surface = radial * surface_radius;
			auto projected = clip * glm::dvec4(surface, 1);
			check(projected.z > -projected.w && projected.z < projected.w, "street-level surface survives both clipping planes");
			check(std::isfinite(ray_hit_to_lla({640,400}, {1280,800}, p, street.cam_->view_matrix()).x), "street-level mouse picking remains valid");
			check(select_map_zoom(view_texel_size(clip, {1280,800})) >= std::min(size_t(19), zoom + 2), "street-level views request detailed street imagery");
		}
		for (int i = 0; i < 40; i++) street.cam_->scroll(-1);
		check(street.cam_->zoom_ == 0, "scrolling back out still clamps at the whole-globe limit");
	}

	size_t tested_zooms = 0;
	double maximum_zoom_error = 0;
	for (int zoom : { 0, 2, 5, 10, 15, 18 }) {
		for (glm::dvec2 viewport : { glm::dvec2(1280,800), glm::dvec2(1600,1000), glm::dvec2(800,500) }) {
			de2::get_instance().viewport = viewport;
			for (glm::dvec2 fraction : { glm::dvec2(0), glm::dvec2(-0.04,0.03), glm::dvec2(0.05,-0.025) }) {
				renderer_system zoomed;
				zoomed.cam_->zoom_ = zoom;
				auto target = viewport * (glm::dvec2(0.5) + fraction);
				auto p = zoomed.get_projection();
				zoomed.cam_->set_projection(viewport,p);
				auto point = ray_hit_ellipsoid(target,viewport,p,zoomed.cam_->view_matrix());
				check(std::isfinite(point.x), "wheel zoom test starts on a surface point");
				auto original = zoomed.cam_->view_matrix();
				for (double direction : { 1.0, -1.0 }) {
					zoomed.cam_->scroll(direction,target);
					p = zoomed.get_projection();
					zoomed.cam_->set_projection(viewport,p);
					double error = glm::distance(screen_position(*zoomed.cam_,point,viewport,p),target);
					maximum_zoom_error = std::max(maximum_zoom_error,error);
					if(error>=0.1) std::cerr << "zoom error=" << error << " level=" << zoom << " direction=" << direction << '\n';
					check(error<0.1,"wheel zoom in and out pins the aimed-at point at every height and DPI");
					check((zoomed.cam_->get_view()*glm::vec4(0,0,1,0)).y>0,"wheel zoom preserves north-up orientation");
					tested_zooms++;
				}
				check(zoomed.cam_->zoom_==size_t(zoom),"opposite wheel ticks restore the zoom level");
				if(fraction==glm::dvec2(0)) check(glm::distance(zoomed.cam_->world_position(),glm::dvec3(glm::inverse(original)[3]))<0.01,"centre zoom has no orbit drift");
			}
		}
	}
	{
		renderer_system sky;
		de2::get_instance().viewport = {1280,800};
		sky.cam_->set_projection({1280,800},sky.get_projection());
		auto before = glm::normalize(sky.cam_->world_position());
		sky.cam_->scroll(1,{-100,-100});
		check(glm::distance(before,glm::normalize(sky.cam_->world_position()))<1e-12,"wheel over empty space zooms without changing orbit angles");
		sky.cam_->zoom_=camera::maximum_zoom;
		auto limited=sky.cam_->view_matrix();
		sky.cam_->scroll(1,{700,420});
		check(sky.cam_->view_matrix()==limited,"wheel at the zoom limit cannot shift the globe");
	}
	{
		de2::get_instance().viewport = {1280,800};
		renderer_system limb;
		const glm::dvec2 target{1000,400};
		auto p = limb.get_projection();
		limb.cam_->set_projection({1280,800},p);
		auto point = ray_hit_ellipsoid(target,{1280,800},p,limb.cam_->view_matrix());
		check(std::isfinite(point.x),"limb zoom starts on the map");
		for (int i=0;i<17;i++) {
			limb.cam_->scroll(1,target);
			p = limb.get_projection();
			limb.cam_->set_projection({1280,800},p);
			double error = glm::distance(screen_position(*limb.cam_,point,{1280,800},p),target);
			maximum_zoom_error = std::max(maximum_zoom_error,error);
			if(error>=0.1) std::cerr << "limb zoom level=" << limb.cam_->zoom_ << " error=" << error << "\n";
			check(error<0.1,"repeated wheel zoom from the limb to street level preserves the same location");
			tested_zooms++;
		}
		check(limb.cam_->zoom_==camera::maximum_zoom,"aimed zoom reaches the street-level limit");
		renderer_system shrinking;
		shrinking.cam_->set_projection({1280,800},shrinking.get_projection());
		auto before=glm::normalize(shrinking.cam_->world_position());
		shrinking.cam_->scroll(-1,target);
		check(glm::dot(before,glm::normalize(shrinking.cam_->world_position()))>0.5,"unreachable cursor pinning while zooming out cannot flip the orbit");
		check((shrinking.cam_->get_view()*glm::vec4(0,0,1,0)).y>0,"zooming out beyond the cursor's globe radius keeps north upright");
		renderer_system held;
		p=held.get_projection();
		const glm::dvec2 grab{720,430};
		check(held.cam_->begin_drag(grab,{1280,800},p),"wheel-during-drag test acquires an off-centre anchor");
		point=ray_hit_ellipsoid(grab,{1280,800},p,held.cam_->view_matrix());
		held.cam_->scroll(1,grab);
		p=held.get_projection();
		held.cam_->set_projection({1280,800},p);
		check(glm::distance(screen_position(*held.cam_,point,{1280,800},p),grab)<0.1,"wheel zoom during a held drag preserves the grabbed location");
		held.cam_->cursor_pos(grab.x+10,grab.y);
		check(std::abs(screen_position(*held.cam_,point,{1280,800},p).x-grab.x-10)<0.1,"horizontal dragging resumes smoothly after aimed zoom");
	}
	std::cout << "Cursor zoom: " << tested_zooms << " checks, maximum anchor error " << maximum_zoom_error << " px.\n";

	double maximum_drag_error = 0;
	size_t tested_drags = 0;
	for (int zoom : { 0, 2, 5, 10, 15, 16, 17, 18, 19 }) {
		for (glm::dvec2 viewport : { glm::dvec2(1280, 800), glm::dvec2(1600, 900), glm::dvec2(800, 500) }) {
			de2::get_instance().viewport = viewport;
			renderer_system projection_renderer;
			projection_renderer.cam_->zoom_ = zoom;
			auto p = projection_renderer.get_projection();
			for (glm::dvec2 offset : { glm::dvec2(0), glm::dvec2(-40, 20), glm::dvec2(50, -30) }) {
				camera drag;
				drag.zoom_ = zoom;
				auto point = ray_hit_ellipsoid(viewport / 2.0 + offset, viewport, p, drag.view_matrix());
				check(std::isfinite(point.x), "drag test starts on the globe");
				auto start = screen_position(drag, point, viewport, p);
				check(drag.begin_drag(start, viewport, p), "surface drag acquires an anchor");
				auto previous_mouse = start;
				for (glm::dvec2 delta : { glm::dvec2(8, 0), glm::dvec2(20, 0), glm::dvec2(20, 12), glm::dvec2(20, -8), glm::dvec2(-10, -8), glm::dvec2(15, 12), glm::dvec2(-10, -8), glm::dvec2(0) }) {
					auto target = start + delta;
					auto before = screen_position(drag, point, viewport, p);
					auto before_eye = drag.get_world_pos();
					auto motion = target - previous_mouse;
					drag.cursor_pos(target.x, target.y);
					auto after = screen_position(drag, point, viewport, p);
					double error = 0;
					if (motion.y == 0) {
						error = std::abs(after.x - before.x - motion.x);
						check(drag.get_world_pos().z == before_eye.z, "horizontal dragging cannot change camera pitch");
					}
					if (motion.x == 0) {
						error = std::abs(after.y - before.y - motion.y);
						auto eye = drag.get_world_pos();
						check(std::abs(std::atan2(eye.y, eye.x) - std::atan2(before_eye.y, before_eye.x)) < 1e-6, "vertical dragging cannot change camera longitude");
					}
					maximum_drag_error = std::max(maximum_drag_error, error);
					if (error > 0.75) std::cerr << "drag error=" << error << " zoom=" << zoom << " viewport=" << viewport.x << 'x' << viewport.y << '\n';
					check(error < 0.75 && std::isfinite(after.x) && std::isfinite(after.y), "grabbed location follows the driven mouse axis at every zoom and DPI scale");
					check((drag.get_view() * glm::vec4(0, 0, 1, 0)).y > 0, "drag keeps north upright");
					previous_mouse = target;
					tested_drags++;
				}
				drag.end_drag();
				auto stopped = drag.get_view();
				drag.cursor_pos(start.x + 100, start.y);
				check(drag.get_view() == stopped, "releasing the mouse ends rotation");
				camera combined, split;
				combined.zoom_ = split.zoom_ = zoom;
				check(combined.begin_drag(start, viewport, p) && split.begin_drag(start, viewport, p), "diagonal test acquires anchors");
				for (int i = 1; i <= 5; i++) {
					combined.cursor_pos(start.x + 2 * i, start.y + i);
					split.cursor_pos(start.x + 2 * i, start.y + i - 1);
					split.cursor_pos(start.x + 2 * i, start.y + i);
					check(combined.get_view() == split.get_view(), "diagonal input combines independent yaw and pitch movements");
				}
			}
		}
	}
	{
		camera drag;
		auto istanbul = geo_to_ecef({ glm::radians(41.0082), glm::radians(28.9784), 0 });
		auto start = screen_position(drag, istanbul, { 1280, 800 }, projection);
		check(drag.begin_drag(start, { 1280, 800 }, projection), "Istanbul is grabbable");
		auto original_eye = drag.get_world_pos();
		auto original_pole = screen_position(drag, { 0, 0, earth_b }, { 1280, 800 }, projection);
		for (int x = -100; x <= 40; x += 10) {
			drag.cursor_pos(start.x + x, start.y);
			double error = std::abs(screen_position(drag, istanbul, { 1280, 800 }, projection).x - start.x - x);
			if (error >= 0.1) std::cerr << "Istanbul error=" << error << " delta=" << x << " start=" << start.x << ',' << start.y << '\n';
			check(error < 0.1, "Istanbul tracks cursor X during horizontal dragging");
			check(drag.get_world_pos().z == original_eye.z, "horizontal Istanbul drag never tilts toward a pole");
			check(glm::distance(screen_position(drag, { 0, 0, earth_b }, { 1280, 800 }, projection), original_pole) < 0.0001, "the north pole stays fixed on screen during Z-axis rotation");
			if (x == -100) check(std::abs(screen_position(drag, istanbul, { 1280, 800 }, projection).y - start.y) > 1, "horizontal rotation follows the natural projected latitude arc");
		}
		bool istanbul_left_globe = false;
		for (int x = 45; x <= 350; x += 5) {
			auto before = drag.get_view();
			auto hit = ray_hit_ellipsoid(start + glm::dvec2(x, 0), { 1280, 800 }, projection, before);
			drag.cursor_pos(start.x + x, start.y);
			if (!std::isfinite(hit.x)) {
				istanbul_left_globe = true;
				check(drag.get_view() == before, "Istanbul's horizontal drag freezes as the cursor crosses the visible limb");
			}
			check((drag.get_view() * glm::vec4(0, 0, 1, 0)).y > 0, "Istanbul's drag cannot invert north near the limb");
			check(drag.get_world_pos().z == original_eye.z, "approaching the limb during horizontal dragging cannot change pitch");
		}
		check(istanbul_left_globe, "Istanbul regression reaches empty space inside the window");
		auto held_view = drag.get_view();
		drag.cursor_pos(10000, -10000);
		check(drag.get_view() == held_view, "moving into empty space freezes the last valid camera pose");
		drag.cursor_pos(-10000, 10000);
		check(drag.get_view() == held_view, "continued movement outside the globe cannot rotate or flip it");
		drag.cursor_pos(start.x, start.y);
		check(drag.get_view() == held_view, "re-entering the globe rebases the anchor without moving the camera");
		auto resumed_point = ray_hit_ellipsoid(start, { 1280, 800 }, projection, drag.get_view());
		drag.cursor_pos(start.x + 10, start.y);
		check(std::abs(screen_position(drag, resumed_point, { 1280, 800 }, projection).x - start.x - 10) < 0.1, "horizontal dragging resumes smoothly after re-entry");
		drag.end_drag();
		auto stopped = drag.get_view();
		check(!drag.begin_drag({ 0, 0 }, { 1280, 800 }, projection), "clicking the sky does not acquire a globe anchor");
		drag.cursor_pos(100, 100);
		check(drag.get_view() == stopped, "a drag starting in the sky does not rotate the globe");
	}
	{
		camera drag;
		check(drag.begin_drag({ 640, 400 }, { 1280, 800 }, projection), "edge-crossing test starts on the globe");
		bool crossed_limb = false;
		for (int x = 648; x <= 1256; x += 8) {
			auto before = drag.get_view();
			auto hit = ray_hit_ellipsoid({ x, 400 }, { 1280, 800 }, projection, before);
			drag.cursor_pos(x, 400);
			auto after = drag.get_view();
			if (!std::isfinite(hit.x)) {
				crossed_limb = true;
				check(after == before, "rotation stops on the first cursor sample beyond the limb");
			}
			for (int axis = 0; axis < 3; axis++) {
				glm::vec3 a{ before[0][axis], before[1][axis], before[2][axis] };
				glm::vec3 b{ after[0][axis], after[1][axis], after[2][axis] };
				check(glm::dot(a, b) > 0.9f, "small cursor movements cannot jump to another camera orientation near the limb");
			}
			check((after * glm::vec4(0, 0, 1, 0)).y > 0, "edge crossing preserves north-up orientation");
		}
		check(crossed_limb, "edge-crossing test reached empty space");
		auto frozen = drag.get_view();
		drag.cursor_pos(800, 400);
		check(drag.get_view() == frozen, "returning from the limb does not snap to the old anchor");
		drag.end_drag();
		drag.cursor_pos(700, 400);
		check(drag.get_view() == frozen, "release cancels suspended dragging");
	}
	{
		camera drag;
		check(drag.begin_drag({ 640, 400 }, { 1280, 800 }, projection), "polar test starts on the globe");
		drag.cursor_pos(640, 650);
		drag.end_drag();
		auto pole = screen_position(drag, { 0, 0, earth_b }, { 1280, 800 }, projection);
		check(!drag.begin_drag(pole, { 1280, 800 }, projection), "the uncovered Mercator pole is empty space for dragging");
	}
	{
		camera drag;
		auto point = ray_hit_ellipsoid({ 1000, 400 }, { 1280, 800 }, projection, drag.get_view());
		check(std::isfinite(point.x), "limb test starts on visible surface");
		auto start = screen_position(drag, point, { 1280, 800 }, projection);
		check(drag.begin_drag(start, { 1280, 800 }, projection), "a point near the limb is grabbable");
		for (int x = 0; x <= 150; x += 10) {
			drag.cursor_pos(start.x - x, start.y);
			check(std::abs(screen_position(drag, point, { 1280, 800 }, projection).x - start.x + x) < 0.1, "a grabbed limb point follows cursor X toward the center");
		}
	}
	{
		camera drag;
		check(drag.begin_drag({ 640, 400 }, { 1280, 800 }, projection), "zoom test acquires an anchor");
		drag.scroll(1);
		auto point = ray_hit_ellipsoid({ 640, 400 }, { 1280, 800 }, projection, drag.get_view());
		drag.cursor_pos(660, 400);
		check(std::abs(screen_position(drag, point, { 1280, 800 }, projection).x - 660) < 0.1, "zooming during a drag rebases the anchor without changing horizontal drag speed");
		drag.set_projection({ 1600, 1000 }, projection);
		point = ray_hit_ellipsoid({ 660, 400 }, { 1600, 1000 }, projection, drag.get_view());
		drag.cursor_pos(680, 400);
		check(std::abs(screen_position(drag, point, { 1600, 1000 }, projection).x - 680) < 0.1, "resizing during a drag uses the updated viewport");
		drag.cursor_pos(10000, 10000);
		drag.scroll(1);
		auto paused = drag.get_view();
		drag.cursor_pos(800, 500);
		check(drag.get_view() == paused, "zooming while outside preserves paused dragging and jump-free re-entry");
		point = ray_hit_ellipsoid({ 800, 500 }, { 1600, 1000 }, projection, drag.get_view());
		drag.cursor_pos(810, 500);
		check(std::abs(screen_position(drag, point, { 1600, 1000 }, projection).x - 810) < 0.1, "paused dragging resumes after zooming outside the globe");
	}

	size_t tested_hits = 0;
	for (int zoom : { 0, 2, 5, 10, 15 }) {
		for (double yaw : { 0.0, 0.7, 1.57, 3.14, 4.71, 6.28 }) {
			for (double pitch : { -1.4, -0.3, 0.0, 0.8, 1.4 }) {
				double distance = earth_a + 30000000 / std::pow(2.0, zoom);
				glm::vec3 eye = distance * glm::dvec3(std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), std::sin(pitch));
				auto v = glm::lookAt(eye, glm::vec3(0), glm::vec3(0, 0, 1));
				glm::dmat4 clip = glm::dmat4(projection) * glm::dmat4(v);
				for (int y = 0; y <= 8; y++) for (int x = 0; x <= 12; x++) {
					auto lla = ray_hit_to_lla({ x * 1280.0 / 12, y * 100.0 }, { 1280, 800 }, projection, v);
					if (!std::isfinite(lla.x) || std::abs(lla.x) > 85.0511287798066) continue;
					for (size_t level = 1; level <= size_t(zoom + 1); level++) {
						auto path = merc_to_path(lla_to_merc(lla, std::ldexp(256.0, int(level))), double(level - 1));
						check(calculate_bounds(path).is_visible(eye, clip, { earth_a, earth_a, earth_b }), "visible ray-hit tile and every ancestor survive frustum and horizon culling");
					}
					tested_hits++;
				}
			}
		}
	}
	check(tested_hits > 1000, "culling test exercised the globe at multiple positions and zooms");
	{
		ecs_s::sparse_set<int, 8> storage;
		int a = 1, b = 2, c = 3;
		storage.insert(3, a); storage.insert(9, b); storage.insert(9000, c);
		storage.erase(9);
		check(storage.has(9000) && storage[9000] == 3, "removing a tile preserves the moved entity's sparse index");
		ecs_s::registry world;
		auto e = world.new_entity();
		world.add_component(e, 5);
		world.view<int>([](ecs_s::entity&, int& value) { value = 7; });
		check(world.get_component_value_for<int>(e) == 7, "ECS views modify actual components");
		world.truncate_component<int>();
		check(!world.component_has<int>(e), "visibility truncation clears previous frame tags");
	}
	std::cout << "Passed: independent rotation axes, fixed north pole on horizontal drags, limb freeze, smooth re-entry, polar openings, " << tested_drags << " drag moves (maximum driven-axis error " << maximum_drag_error << " px), geodesy, picking, LOD coverage, winding, seams, ECS, and " << tested_hits << " visible ray samples.\n";
}
