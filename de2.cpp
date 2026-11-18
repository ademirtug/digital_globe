#define GLAD_GL_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "de2.h"
#include "util.h"
#include "sky.h"
#include "platform.h"
#include <wx/app.h>
#include <wx/dcclient.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <iterator>
#include <sstream>

namespace {
	GLuint load_shader(GLenum type, const std::string& path) {
		std::ifstream file(resource_path(path));
		if (!file) throw std::runtime_error("cannot open shader: " + path);
		std::string source{ std::istreambuf_iterator<char>(file), {} };
		const char* data = source.c_str();
		GLuint id = glCreateShader(type);
		glShaderSource(id, 1, &data, nullptr);
		glCompileShader(id);
		GLint ok = GL_FALSE;
		glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			char message[4096]{};
			glGetShaderInfoLog(id, sizeof(message), nullptr, message);
			glDeleteShader(id);
			throw std::runtime_error(path + ": " + message);
		}
		return id;
	}
}

program::program(const std::string& name, const std::string& vertex_path, const std::string& fragment_path) {
	GLuint vs = load_shader(GL_VERTEX_SHADER, vertex_path);
	GLuint fs = 0;
	try { fs = load_shader(GL_FRAGMENT_SHADER, fragment_path); }
	catch (...) { glDeleteShader(vs); throw; }
	id_ = glCreateProgram();
	glAttachShader(id_, vs);
	glAttachShader(id_, fs);
	glLinkProgram(id_);
	glDeleteShader(vs);
	glDeleteShader(fs);
	GLint ok = GL_FALSE;
	glGetProgramiv(id_, GL_LINK_STATUS, &ok);
	if (!ok) {
		char message[4096]{};
		glGetProgramInfoLog(id_, sizeof(message), nullptr, message);
		glDeleteProgram(id_);
		throw std::runtime_error(name + ": " + message);
	}
}
program::~program() { glDeleteProgram(id_); }
void program::use() { glUseProgram(id_); }
void program::setuniform(const std::string& name, const glm::mat4& value) {
	if (name == "view") view_ = value;
	glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void program::setuniform(const std::string& name, const glm::dmat4& value) {
	if (name == "view") view_ = value;
	auto matrix = glm::mat4(value);
	glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}
void program::set_model(const glm::mat4& transform, glm::dvec3 origin) {
	auto matrix = glm::translate(glm::dmat4(transform), origin);
	setuniform("model", glm::mat4(matrix));
	// Cancel Earth-sized translations on the CPU in double precision. Vertices
	// sent to the GPU are small offsets from each tile's origin.
	setuniform("model_view", glm::mat4(view_ * matrix));
}
void program::setuniform(const std::string& name, const glm::vec3& value) {
	glUniform3fv(glGetUniformLocation(id_, name.c_str()), 1, glm::value_ptr(value));
}
void program::setuniform(const std::string& name, const glm::vec2& value) {
	glUniform2fv(glGetUniformLocation(id_, name.c_str()), 1, glm::value_ptr(value));
}
void program::setuniform(const std::string& name, float value) {
	glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}
void program::setuniform(const std::string& name, int value) {
	glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}

texture::texture(const std::filesystem::path& path) {
	std::ifstream file(path, std::ios::binary);
	decode({ std::istreambuf_iterator<char>(file), {} });
}
texture::texture(const std::shared_ptr<std::vector<unsigned char>>& data) { decode(*data); }
texture::~texture() { if (id_) glDeleteTextures(1, &id_); }
void texture::decode(const std::vector<unsigned char>& data) {
	int channels = 0;
	unsigned char* image = data.empty() ? nullptr : stbi_load_from_memory(data.data(), (int)data.size(), &width_, &height_, &channels, 4);
	if (image) {
		pixels_.assign(image, image + size_t(width_) * height_ * 4);
		stbi_image_free(image);
	}
	else {
		static std::once_flag warning;
		std::call_once(warning, [] { std::cerr << "map image unavailable; using a placeholder tile\n"; });
		width_ = height_ = 1;
		pixels_ = { 80, 110, 130, 255 };
	}
}
void texture::upload() {
	if (id_) return;
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_.data());
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (GLAD_GL_ARB_texture_filter_anisotropic || GLAD_GL_EXT_texture_filter_anisotropic) {
		GLfloat maximum = 1;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maximum);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, std::min(8.0f, maximum));
	}
	pixels_.clear();
}
void texture::activate() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id_);
}

model::~model() {
	if (ebo_) glDeleteBuffers(1, &ebo_);
	if (vbo_) glDeleteBuffers(1, &vbo_);
	if (vao) glDeleteVertexArrays(1, &vao);
}
void model::upload() {
	if (!m || vao) return;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	if (m->positions.size() == m->vertices.size()) {
		auto local = m->vertices;
		for (size_t i = 0; i < local.size(); i++) local[i].position = m->positions[i] - m->origin;
		glBufferData(GL_ARRAY_BUFFER, local.size() * sizeof(vertex), local.data(), GL_STATIC_DRAW);
	}
	else glBufferData(GL_ARRAY_BUFFER, m->vertices.size() * sizeof(vertex), m->vertices.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &ebo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->indices.size() * sizeof(int), m->indices.data(), GL_STATIC_DRAW);
	for (GLuint i = 0; i < 3; i++) glEnableVertexAttribArray(i);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));
	glBindVertexArray(0);
	if (tex) tex->upload();
}
void model::draw() {
	if (!prg || !vao) return;
	prg->use();
	prg->set_model(transform, m->origin);
	prg->setuniform("texture_scale", texture_scale);
	prg->setuniform("texture_offset", texture_offset);
	prg->setuniform("material.diffuse", 0);
	prg->setuniform("material.specular", m->specular);
	prg->setuniform("material.shininess", m->shininess);
	if (tex) tex->activate();
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)m->indices.size(), GL_UNSIGNED_INT, nullptr);
}

double camera::height() const {
	return std::ldexp(30000000.0, -int(std::min(zoom_, maximum_zoom)));
}
double camera::distance(double pitch) const {
	// Pitch is geocentric latitude. Intersect that radial direction with WGS84
	// before adding height; using the equatorial radius creates an altitude floor.
	double x = std::cos(pitch) / earth_a, z = std::sin(pitch) / earth_b;
	return 1 / std::hypot(x, z) + height();
}
glm::dmat4 camera::view_at(double yaw, double pitch) const {
	auto eye = distance(pitch) * glm::dvec3(std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), std::sin(pitch));
	return glm::lookAt(eye, glm::dvec3(0), glm::dvec3(0, 0, 1));
}
glm::dvec3 camera::world_position() const {
	return distance(pitch_) * glm::dvec3(std::cos(pitch_) * std::cos(yaw_), std::cos(pitch_) * std::sin(yaw_), std::sin(pitch_));
}
glm::dmat4 camera::view_matrix() const { return view_at(yaw_, pitch_); }
glm::vec3 camera::get_world_pos() const { return world_position(); }
glm::mat4 camera::get_view() const {
	return view_at(yaw_, pitch_);
}
void camera::set_projection(glm::dvec2 viewport, const glm::dmat4& projection) {
	if (viewport == viewport_ && projection == projection_) return;
	viewport_ = viewport;
	projection_ = projection;
	rebase_drag();
}
bool camera::begin_drag(glm::dvec2 mouse, glm::dvec2 viewport, const glm::dmat4& projection) {
	dragging_ = false;
	drag_suspended_ = false;
	grab_offset_ = { 0, 0 };
	last_ = mouse;
	viewport_ = viewport;
	projection_ = projection;
	// Remember the surface point we grabbed for the entire gesture.
	dragging_ = pick_surface(mouse, anchor_);
	return dragging_;
}
void camera::rebase_drag() {
	if (!dragging_) return;
	bool acquired = begin_drag(last_, viewport_, projection_);
	dragging_ = true;
	drag_suspended_ = !acquired;
}
bool camera::pick_surface(glm::dvec2 mouse, glm::dvec3& hit) const {
	if (viewport_.x <= 0 || viewport_.y <= 0 || !std::isfinite(mouse.x) || !std::isfinite(mouse.y)) return false;
	if (mouse.x < 0 || mouse.y < 0 || mouse.x > viewport_.x || mouse.y > viewport_.y) return false;
	auto inverse = glm::inverse(projection_ * view_matrix());
	glm::dvec2 ndc{ 2 * mouse.x / viewport_.x - 1, 1 - 2 * mouse.y / viewport_.y };
	auto near_point = inverse * glm::dvec4(ndc, -1, 1);
	auto far_point = inverse * glm::dvec4(ndc, 1, 1);
	glm::dvec3 origin = glm::dvec3(near_point) / near_point.w;
	glm::dvec3 direction = glm::normalize(glm::dvec3(far_point) / far_point.w - origin);
	glm::dvec3 radii{ earth_a, earth_a, earth_b };
	auto o = origin / radii, d = direction / radii;
	double a = glm::dot(d, d), b = glm::dot(o, d), c = glm::dot(o, o) - 1;
	double discriminant = b * b - a * c;
	if (!std::isfinite(discriminant) || discriminant < 0) return false;
	double t = (-b - std::sqrt(discriminant)) / a;
	if (t < 0 || !std::isfinite(t)) return false;
	hit = origin + t * direction;
	double latitude = std::atan2(hit.z, (earth_b * earth_b / (earth_a * earth_a)) * std::hypot(hit.x, hit.y));
	return std::abs(latitude) <= glm::radians(85.0511287798066);
}
glm::dvec2 camera::project_anchor(double yaw, double pitch) const {
	auto clip = projection_ * view_at(yaw, pitch) * glm::dvec4(anchor_, 1);
	return { (clip.x / clip.w + 1) * viewport_.x / 2, (1 - clip.y / clip.w) * viewport_.y / 2 };
}
void camera::mouse_button(glm::dvec2 mouse, bool pressed) {
    if (pressed) begin_drag(mouse, viewport_, projection_);
    else end_drag();
}
void camera::cursor_pos(double x, double y) {
	if (dragging_) {
		glm::dvec2 target{ x, y };
		glm::dvec3 hit;
		if (!pick_surface(target, hit)) {
			drag_suspended_ = true;
			last_ = target;
			return;
		}
		if (drag_suspended_) {
			// Re-entering starts from the point now under the mouse, without a snap.
			anchor_ = hit;
			grab_offset_ = { 0, 0 };
			drag_suspended_ = false;
			last_ = target;
			return;
		}
		// Small screen-space steps keep the solve on the same local solution.
		size_t steps = size_t(std::ceil(glm::length(target - last_) / 4));
		steps = std::clamp(steps, size_t(1), size_t(512));
		auto motion = target - last_;
		for (size_t i = 1; i <= steps; i++) {
			auto next = glm::mix(last_, target, double(i) / steps);
			if (!pick_surface(next, hit)) {
				drag_suspended_ = true;
				break;
			}
			if (motion.x != 0) {
				auto before = project_anchor(yaw_, pitch_);
				if (!drag_to(next.x + grab_offset_.x, true)) { drag_suspended_ = true; break; }
				// A latitude parallel curves on screen. Follow that arc without tilting.
				grab_offset_.y += project_anchor(yaw_, pitch_).y - before.y;
			}
			if (motion.y != 0) {
				auto before = project_anchor(yaw_, pitch_);
				if (!drag_to(next.y + grab_offset_.y, false)) { drag_suspended_ = true; break; }
				grab_offset_.x += project_anchor(yaw_, pitch_).x - before.x;
			}
		}
	}
	last_ = { x, y };
}
bool camera::drag_to(double target, bool horizontal) {
	double old_yaw = yaw_, old_pitch = pitch_;
	auto coordinate = [&](double yaw, double pitch) {
		auto p = project_anchor(yaw, pitch);
		return horizontal ? p.x : p.y;
	};
	for (int iteration = 0; iteration < 16; iteration++) {
		double error = target - coordinate(yaw_, pitch_);
		if (std::abs(error) < 0.025) break;
		// Horizontal input solves yaw alone; vertical input solves pitch alone.
		const double h = 0.00001;
		double derivative = horizontal
			? (coordinate(yaw_ + h, pitch_) - coordinate(yaw_ - h, pitch_)) / (2 * h)
			: (coordinate(yaw_, pitch_ + h) - coordinate(yaw_, pitch_ - h)) / (2 * h);
		if (!std::isfinite(derivative) || std::abs(derivative) < 1e-8) break;
		double step = std::clamp(error / derivative, -0.25, 0.25);
		bool moved = false;
		for (double scale = 1; scale >= 1.0 / 128; scale /= 2) {
			double yaw = horizontal ? yaw_ + step * scale : old_yaw;
			double pitch = horizontal ? old_pitch : std::clamp(pitch_ + step * scale, -1.55, 1.55);
			if (std::abs(std::remainder(yaw - old_yaw, 2 * glm::pi<double>())) > 0.35 || std::abs(pitch - old_pitch) > 0.35) continue;
			auto eye = distance(pitch) * glm::dvec3(std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), std::sin(pitch));
			auto surface_normal = anchor_ / glm::dvec3(earth_a * earth_a, earth_a * earth_a, earth_b * earth_b);
			if (glm::dot(surface_normal, eye - anchor_) <= 0) continue;
			double next_error = target - coordinate(yaw, pitch);
			if (std::abs(next_error) >= std::abs(error)) continue;
			yaw_ = horizontal ? std::remainder(yaw, 2 * glm::pi<double>()) : old_yaw;
			pitch_ = pitch;
			moved = true;
			break;
		}
		if (!moved) break;
	}
	double error = target - coordinate(yaw_, pitch_);
	if (std::isfinite(error) && std::abs(error) <= 0.25) return true;
	// An unreachable target must not leave a partially solved, rotated camera.
	yaw_ = old_yaw;
	pitch_ = old_pitch;
	return false;
}
void camera::zoom_to(glm::dvec2 target) {
	// Wheel zoom may adjust both orbit angles to preserve the aimed-at point.
	// Dragging still solves each axis independently.
	auto front = [&](double yaw, double pitch) {
		auto eye = distance(pitch) * glm::dvec3(std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), std::sin(pitch));
		auto normal = anchor_ / glm::dvec3(earth_a * earth_a, earth_a * earth_a, earth_b * earth_b);
		return glm::dot(normal, eye - anchor_) > 0;
	};
	if (!front(yaw_, pitch_)) {
		// A point near the old horizon can be behind the new horizon after zooming
		// in. Seed the visible solution from that point, never its far-side twin.
		yaw_ = std::atan2(anchor_.y, anchor_.x);
		pitch_ = std::clamp(std::atan2(anchor_.z, std::hypot(anchor_.x, anchor_.y)), -1.55, 1.55);
	}
	for (int iteration = 0; iteration < 48; iteration++) {
		auto error = target - project_anchor(yaw_, pitch_);
		// Tiny residuals at globe scale would be magnified by later wheel ticks.
		if (!std::isfinite(error.x) || !std::isfinite(error.y) || glm::length(error) < 1e-8) break;
		const double h = 0.000001;
		auto dx = (project_anchor(yaw_ + h, pitch_) - project_anchor(yaw_ - h, pitch_)) / (2 * h);
		auto dy = (project_anchor(yaw_, pitch_ + h) - project_anchor(yaw_, pitch_ - h)) / (2 * h);
		glm::dmat2 derivative(dx, dy);
		if (std::abs(glm::determinant(derivative)) < 1e-10) break;
		auto step = glm::inverse(derivative) * error;
		if (!std::isfinite(step.x) || !std::isfinite(step.y)) break;
		if (glm::length(step) > 0.2) step *= 0.2 / glm::length(step);
		bool moved = false;
		for (double scale = 1; scale >= 1.0 / 1024; scale /= 2) {
			double yaw = std::remainder(yaw_ + step.x * scale, 2 * glm::pi<double>());
			double pitch = std::clamp(pitch_ + step.y * scale, -1.55, 1.55);
			if (!front(yaw, pitch)) continue;
			auto next = target - project_anchor(yaw, pitch);
			if (!std::isfinite(next.x) || !std::isfinite(next.y) || glm::length(next) >= glm::length(error)) continue;
			yaw_ = yaw; pitch_ = pitch; moved = true;
			break;
		}
		// Zooming out can shrink the globe inside the cursor's radius. Retain the
		// closest visible solution when exact pinning is no longer possible.
		if (!moved) break;
	}
}
void camera::scroll(double offset, glm::dvec2 mouse) {
	last_ = mouse;
	scroll(offset);
}
void camera::scroll(double offset) {
	size_t zoom = std::clamp(int(zoom_) + (offset > 0 ? 1 : offset < 0 ? -1 : 0), 0, int(maximum_zoom));
	if (zoom == zoom_) return;
	glm::dvec3 point;
	bool aimed = pick_surface(last_, point);
	auto held_anchor = anchor_;
	zoom_ = zoom;
	if (aimed) {
		anchor_ = point;
		zoom_to(last_);
		anchor_ = held_anchor;
	}
	rebase_drag();
}

void de2::init(bool visible) {
    if (!wxTheApp) {
        wxApp::SetInstance(new wxApp);
        int argc = 1;
        static char name[] = "digital_globe";
        char* argv[] = { name, nullptr };
        if (!wxEntryStart(argc, argv) || !wxTheApp->CallOnInit()) throw std::runtime_error("wxWidgets initialization failed");
    }
    wxTheApp->SetAppName("digital_globe");
    window = new wxFrame(nullptr, wxID_ANY, "digital_globe", wxDefaultPosition, wxSize(1280, 850));
    status_bar_ = window->CreateStatusBar(2);
    int widths[] = { -1, window->FromDIP(240) };
    status_bar_->SetStatusWidths(2, widths);
    wxGLAttributes attributes;
    attributes.PlatformDefaults().RGBA().DoubleBuffer().Depth(24).EndList();
    canvas = new wxGLCanvas(window, attributes);
    auto* layout = new wxBoxSizer(wxVERTICAL);
    layout->Add(canvas, 1, wxEXPAND);
    window->SetSizer(layout);
    window->Show();
    window->Layout();
    wxYieldIfNeeded();
    wxGLContextAttrs context_attributes;
    context_attributes.PlatformDefaults().CoreProfile().OGLVersion(3, 3).EndList();
    context_ = std::make_unique<wxGLContext>(canvas, nullptr, &context_attributes);
    if (!context_->IsOK() || !canvas->SetCurrent(*context_)) throw std::runtime_error("OpenGL 3.3 is not available");
    if (!gladLoaderLoadGL()) throw std::runtime_error("OpenGL functions could not be loaded");
    glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE);
    initialized_ = true;
    update_viewport();
    canvas->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxPaintDC dc(canvas);
        if (initialized_ && !stopped_) {
            try { draw(); }
            catch (const std::exception& error) {
                timer_->Stop();
                wxMessageBox(wxString::FromUTF8(error.what()), "Digital Globe", wxOK | wxICON_ERROR, window);
                window->Close();
            }
        }
    });
    canvas->Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
        if (initialized_ && !stopped_) { make_current(); update_viewport(); canvas->Refresh(false); }
        event.Skip();
    });
    canvas->Bind(wxEVT_MOTION, [this](wxMouseEvent& event) { if (on_cursor_pos) on_cursor_pos(event.GetX(), event.GetY()); });
    canvas->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent&) { if (on_cursor_pos) on_cursor_pos(-1, -1); });
    canvas->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event) {
        canvas->SetFocus();
        if (!canvas->HasCapture()) canvas->CaptureMouse();
        if (on_cursor_pos) on_cursor_pos(event.GetX(), event.GetY());
        if (on_mouse_button) on_mouse_button(true);
    });
    canvas->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent&) {
        if (on_mouse_button) on_mouse_button(false);
        if (canvas->HasCapture()) canvas->ReleaseMouse();
    });
    canvas->Bind(wxEVT_MOUSE_CAPTURE_LOST, [this](wxMouseCaptureLostEvent&) { if (on_mouse_button) on_mouse_button(false); });
    canvas->Bind(wxEVT_MOUSEWHEEL, [this](wxMouseEvent& event) {
        if (on_cursor_pos) on_cursor_pos(event.GetX(), event.GetY());
        if (on_scroll && event.GetWheelDelta()) on_scroll(double(event.GetWheelRotation()) / event.GetWheelDelta());
    });
    window->Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& event) { shutdown(); event.Skip(); });
    timer_ = std::make_unique<wxTimer>(window);
    window->Bind(wxEVT_TIMER, [this](wxTimerEvent&) { if (!stopped_ && canvas->IsShownOnScreen()) canvas->Refresh(false); }, timer_->GetId());
    last_ = std::chrono::steady_clock::now();
    set_vsync(true);
    if (!visible) timer_->Stop(); // Tests need a realized GL surface but no background animation.
}
void de2::make_current() { if (context_ && canvas) canvas->SetCurrent(*context_); }
void de2::set_vsync(bool enabled) {
    make_current();
#if wxCHECK_VERSION(3, 3, 0)
    if (canvas) canvas->SetSwapInterval(enabled ? 1 : 0);
#endif
    if (timer_) timer_->Start(enabled ? 16 : 1);
}
void de2::run() { wxTheApp->MainLoop(); }
void de2::draw() {
    make_current(); update_viewport();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_); last_ = now;
    if (viewport.x <= 0 || viewport.y <= 0) return;
    for (auto& callback : get_subs<pre_render>()) callback(elapsed);
    for (auto& callback : get_subs<render>()) callback(elapsed);
    for (auto& callback : get_subs<post_render>()) callback(elapsed);
    canvas->SwapBuffers();
}
void de2::shutdown() {
    if (stopped_) return;
    stopped_ = true;
    if (timer_) timer_->Stop();
    pool_.shutdown();
    make_current();
    if (on_shutdown) { auto callback = std::move(on_shutdown); callback(); }
    get_subs<pre_render>().clear(); get_subs<render>().clear(); get_subs<post_render>().clear();
    programs.clear();
    initialized_ = false;
    context_.reset();
}
void de2::set_title(const std::string& text) { window->SetTitle(wxString::FromUTF8(text)); }
void de2::set_status(const std::string& text) {
    if (!status_bar_ || status_text_ == text) return;
    status_text_ = text; status_bar_->SetStatusText(wxString::FromUTF8(text), 1);
}
void de2::set_info(const std::wstring& text) { if (status_bar_) status_bar_->SetStatusText(wxString(text), 0); }
void de2::update_viewport() {
    auto size = canvas->GetClientSize();
    double scale = canvas->GetContentScaleFactor();
    viewport = { float(std::lround(size.x * scale)), float(std::lround(size.y * scale)) };
    glViewport(0, 0, int(viewport.x), int(viewport.y));
}
glm::dvec2 de2::content_size() const {
    if (!canvas) return viewport;
    auto size = canvas->GetClientSize(); return { size.x, size.y };
}
de2::~de2() { pool_.shutdown(); }

renderer_system::renderer_system() {
	auto& eng = de2::get_instance();
	auto sync_projection = [this, &eng]() {
		auto logical = glm::max(glm::dvec2(1), eng.content_size());
		if (eng.viewport.x > 0 && eng.viewport.y > 0) cam_->set_projection(logical, get_projection());
		return logical;
	};
	eng.on_mouse_button = [this, &eng, sync_projection](bool pressed) {
        auto logical = sync_projection();
        cam_->mouse_button(glm::dvec2(mouse_pos) * logical / glm::dvec2(eng.viewport), pressed);
    };
	eng.on_cursor_pos = [this, &eng, sync_projection](double x, double y) {
		auto logical = sync_projection();
		mouse_pos = glm::dvec2(x, y) * glm::dvec2(eng.viewport) / logical;
		cam_->cursor_pos(x, y);
	};
	eng.on_scroll = [this, &eng, sync_projection](double y) {
		auto logical = sync_projection();
		cam_->scroll(y, glm::dvec2(mouse_pos) * logical / glm::dvec2(eng.viewport));
		sync_projection();
	};
}
renderer_system::~renderer_system() {
	auto& eng = de2::get_instance();
	eng.on_mouse_button = {};
	eng.on_cursor_pos = {};
	eng.on_scroll = {};
}
glm::mat4 renderer_system::get_projection() {
	auto vp = de2::get_instance().viewport;
	double height = cam_->height();
	double near_plane = std::min(double(z_near), std::max(0.5, height * 0.1));
	// The furthest visible surface is at the horizon. Keep generous headroom for
	// the ellipsoid and mesh while avoiding an enormous far/near ratio at street level.
	double far_plane = std::min(double(z_far), 2 * std::sqrt(height * (2 * earth_a + height)));
	return glm::perspective(glm::radians(55.0), double(vp.x) / std::max(1.0, double(vp.y)), near_plane, std::max(near_plane * 2, far_plane));
}
void renderer_system::process(ecs_s::registry& world, std::chrono::nanoseconds ns) {
	glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (background && show_background) background->draw(cam_->view_matrix(), get_projection());
	for (auto& [name, prg] : de2::get_instance().programs) {
		prg->use();
		prg->setuniform("view", cam_->view_matrix());
		prg->setuniform("projection", get_projection());
		prg->setuniform("view_pos", cam_->get_world_pos());
		if (l) {
			prg->setuniform("light.position", l->position);
			prg->setuniform("light.ambient", l->ambient);
			prg->setuniform("light.diffuse", l->diffuse);
			prg->setuniform("light.specular", l->specular);
		}
	}
	world.view<std::shared_ptr<model>, visible>([](ecs_s::entity& e, std::shared_ptr<model>& m, visible&) { m->draw(); });
}
