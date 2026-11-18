#include "spheroid.h"
#include "test_context.h"
#include <cstdlib>

struct sample_view { const char* name; int zoom; double yaw, pitch; int width, height; };

void check(bool ok, const char* message) {
    if (!ok) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}

// Original axis-aligned selection, retained as an independent render oracle.
bool old_visible(const plate_bounds& bounds, glm::dvec3 eye, const glm::dmat4& clip) {
    glm::dvec4 rows[4];
    for (int i=0;i<4;i++) rows[i] = {clip[0][i],clip[1][i],clip[2][i],clip[3][i]};
    for(int axis=0;axis<3;axis++) for(double sign : {-1.0,1.0}) {
        auto p=rows[3]+sign*rows[axis];
        glm::dvec3 corner;
        for(int i=0;i<3;i++) corner[i]=p[i]>=0 ? bounds.max[i] : bounds.min[i];
        if(glm::dot(glm::dvec3(p),corner)+p.w<0) return false;
    }
    auto h=eye/glm::dvec3(earth_a*earth_a,earth_a*earth_a,earth_b*earth_b);
    glm::dvec3 corner;
    for(int i=0;i<3;i++) corner[i]=h[i]>=0 ? bounds.max[i] : bounds.min[i];
    return glm::dot(h,corner)>=1;
}

int main() {
	test_context context;
	{
		auto prg = std::make_shared<program>("tile IDs", "shaders/c_t_direct.vert", "tests/tile_id.frag");
		GLuint fbo, color, depth;
		glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glGenTextures(1, &color); glBindTexture(GL_TEXTURE_2D, color);
		glGenRenderbuffers(1, &depth); glBindRenderbuffer(GL_RENDERBUFFER, depth);
		glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND); glDisable(GL_DITHER); glDisable(GL_MULTISAMPLE); glDisable(GL_FRAMEBUFFER_SRGB);
		glFrontFace(GL_CCW); glCullFace(GL_BACK);
		std::map<std::string, plate_bounds> cache;
		const sample_view samples[] = {
			{ "default", 2, 0, 0.25, 1280, 800 },
			{ "diagonal", 3, 0.785, 0.25, 1280, 800 },
			{ "regional", 5, 0.785, 0.25, 1280, 800 },
			{ "close", 10, 0.785, 0.25, 1280, 800 },
			{ "north", 5, 0.785, 1.35, 1280, 800 },
			{ "4K", 3, 0.785, 0.25, 3840, 2160 },
			{ "street", 19, 0.506, 0.712, 1280, 800 }
		};
		std::ofstream report("obj/visibility-after.csv");
		report << "case,map_zoom,old_image_candidates,old_leaves,new_leaves,pixel_visible,new_detail_images,cold_requests\n";
		std::cout << "case,map_zoom,old_image_candidates,old_leaves,new_leaves,pixel_visible,new_detail_images,cold_requests\n";
		for (const auto& s : samples) {
			double height = std::ldexp(30000000.0, -s.zoom);
			bool street = s.zoom > 15;
			double radius = street ? 1 / std::hypot(std::cos(s.pitch) / earth_a, std::sin(s.pitch) / earth_b) : earth_a;
			glm::dvec3 eye = (radius + height) * glm::dvec3(std::cos(s.pitch) * std::cos(s.yaw), std::cos(s.pitch) * std::sin(s.yaw), std::sin(s.pitch));
			if (!street) eye = glm::vec3(eye); // Preserve the original six benchmark views.
			auto view = glm::lookAt(eye, glm::dvec3(0), glm::dvec3(0, 0, 1));
			if (!street) view = glm::mat4(view);
			auto projection = street
				? glm::mat4(glm::perspective(glm::radians(55.0), double(s.width) / s.height, height * 0.1, 2 * std::sqrt(height * (2 * earth_a + height))))
				: glm::perspective(glm::radians(55.0f), float(s.width) / s.height, 100.0f, 50000000.0f);
			auto clip = glm::dmat4(projection) * view;
			size_t level = select_map_zoom(view_texel_size(clip, { s.width, s.height }));
			std::queue<std::string> pending;
			for (auto root : { "a", "b", "c", "d" }) pending.push(root);
			std::vector<std::string> leaves;
			std::set<std::string> hierarchy;
			while (!pending.empty()) {
				auto path = pending.front(); pending.pop();
				if (!cache.contains(path)) cache.emplace(path, calculate_bounds(path));
				if (!old_visible(cache.at(path), eye, clip)) continue;
				hierarchy.insert(path);
				if (path.size() < level) for (char child : { 'a', 'b', 'c', 'd' }) pending.push(path + child);
				else leaves.push_back(path);
			}

            plate_view visible_view(eye, clip, {earth_a,earth_a,earth_b});
            std::set<std::string> selected;
            std::map<std::string,plate_demand> demand;
            for (auto root : {"a","b","c","d"}) pending.push(root);
            while(!pending.empty()) {
                auto path=pending.front(); pending.pop();
                if (!cache.contains(path)) cache.emplace(path,calculate_bounds(path));
                const auto& bounds=cache.at(path);
                if (!visible_view.is_visible(bounds)) continue;
                if(path.size()<level) { for(char child : {'a','b','c','d'}) pending.push(path+child); continue; }
                selected.insert(path);
                double density=plate_texel_size(bounds,clip,{s.width,s.height});
                size_t local=std::isfinite(density) ? select_map_zoom(density,0,level) : level;
                demand.emplace(path,plate_demand{path.substr(0,local),plate_screen_area(bounds,clip,{s.width,s.height})});
                plate mesh(path,16);
                for(const auto& vertex : mesh.vertices) {
                    double actual=projected_texel_size(vertex.position,clip,{s.width,s.height});
                    check(actual<=density*1.00001,"local density bound encloses all sampled projection derivatives");
                    if(local<level) check(std::ldexp(actual,-int(local))<=1.00001,"local coarsening never enlarges a source pixel");
                }
            }

			glBindTexture(GL_TEXTURE_2D, color);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, s.width, s.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, depth);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, s.width, s.height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return 4;
			glViewport(0, 0, s.width, s.height);
			glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			prg->use(); prg->setuniform("view", view); prg->setuniform("projection", projection);

            for (size_t i = 0; i < leaves.size(); i++) {
                auto mesh = std::make_shared<plate>(leaves[i], 8);
				unsigned id = unsigned(i + 1);
				prg->setuniform("tile_color", glm::vec3(id & 255, (id >> 8) & 255, (id >> 16) & 255) / 255.0f);
				model tile; tile.m = mesh; tile.attach_program(prg); tile.upload(); tile.draw();
			}
			std::vector<unsigned char> pixels(size_t(s.width) * s.height * 4);
			glReadPixels(0, 0, s.width, s.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
			std::set<size_t> visible;
			size_t covered_pixels = 0;
			for (size_t i = 0; i < pixels.size(); i += 4) {
				size_t id = size_t(pixels[i]) + (size_t(pixels[i+1]) << 8) + (size_t(pixels[i+2]) << 16);
				if (id) { visible.insert(id - 1); covered_pixels++; }
			}
			check(!visible.empty(), "reference globe renders into the framebuffer");
			if (street) check(covered_pixels > size_t(s.width) * s.height * 0.9999, "street-level local-origin geometry covers the viewport without clipping or cracks");

            for(size_t id : visible) check(selected.contains(leaves.at(id)),"new hierarchy culling retains every pixel-contributing reference mesh");
            check(selected.size() <= leaves.size(),"oriented culling does not increase selection in diagnostic views");
            auto requests=plan_plate_requests(demand,{"a","b","c","d"});
            auto cold=plan_plate_requests(demand,{});
            check(cold.size()<=requests.size()+4,"cold start adds at most four fallback roots, not all ancestors");
            if(std::string(s.name)=="regional" || std::string(s.name)=="close")
                check(selected.size()<=leaves.size()*0.75,"regional and close views eliminate at least a quarter of old offscreen candidates");
            if(std::string(s.name)=="north") check(requests.size()<selected.size(),"local polar density reduces distinct imagery requests");
            std::string line=std::format("{},{},{},{},{},{},{},{}",s.name,level,hierarchy.size(),leaves.size(),selected.size(),visible.size(),requests.size(),cold.size());
            std::cout << line << '\n'; report << line << '\n';

			if (glGetError() != GL_NO_ERROR) return 5;
		}
		std::cout << "Passed: GPU tile coverage, curved bounds, local density bounds, and download planning.\n";
		glDeleteFramebuffers(1, &fbo); glDeleteRenderbuffers(1, &depth); glDeleteTextures(1, &color);
	}
}
