#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <glm/gtc/matrix_transform.hpp>
#include "model.h"
#include <wx/frame.h>
#include <wx/glcanvas.h>
#include <wx/statusbr.h>
#include <wx/timer.h>
#include "ecs_s/ecs_s.hpp"
#include "lru_cache.h"
#include "thread_pool.h"

struct pre_render {};
struct render {};
struct post_render {};
struct visible {};

template<typename T>
class sub_system {
public:
	virtual ~sub_system() = default;
	virtual void process(ecs_s::registry& world, T& evt) = 0;
};

class camera {
	bool dragging_{ false };
	bool drag_suspended_{ false };
	glm::dvec2 last_{ 0.0 };
	double yaw_{ 0.0 }, pitch_{ 0.25 };
	glm::dvec3 anchor_{ 0.0 };
	glm::dvec2 grab_offset_{ 0.0 };
	glm::dvec2 viewport_{ 1280, 800 };
	glm::dmat4 projection_{ 1.0 };
	double distance(double pitch) const;
	glm::dmat4 view_at(double yaw, double pitch) const;
	glm::dvec2 project_anchor(double yaw, double pitch) const;
	bool pick_surface(glm::dvec2 mouse, glm::dvec3& hit) const;
	bool drag_to(double target, bool horizontal);
	void zoom_to(glm::dvec2 target);
	void rebase_drag();
public:
	static constexpr size_t maximum_zoom = 19;
	size_t zoom_{ 2 };
	double height() const;
	glm::dvec3 world_position() const;
	glm::dmat4 view_matrix() const;
	glm::vec3 get_world_pos() const;
	glm::mat4 get_view() const;
	void mouse_button(glm::dvec2 mouse, bool pressed);
	void set_projection(glm::dvec2 viewport, const glm::dmat4& projection);
	bool begin_drag(glm::dvec2 mouse, glm::dvec2 viewport, const glm::dmat4& projection);
	void end_drag() { dragging_ = drag_suspended_ = false; }
	void cursor_pos(double x, double y);
	void scroll(double offset);
	void scroll(double offset, glm::dvec2 mouse);
};

class de2 {
	template<typename T>
	static auto& get_subs() {
		static std::vector<std::function<void(std::chrono::nanoseconds)>> subs;
		return subs;
	}
	thread_pool pool_{ 2 };
	wxStatusBar* status_bar_{ nullptr };
    std::unique_ptr<wxGLContext> context_;
    std::unique_ptr<wxTimer> timer_;
    bool initialized_{ false }, stopped_{ false };
    std::chrono::steady_clock::time_point last_;
    std::string status_text_;
    void update_viewport();
	de2() = default;
public:
	static de2& get_instance() {
		static de2 instance;
		return instance;
	}
	de2(const de2&) = delete;
	de2& operator=(const de2&) = delete;
	~de2();
	void init(bool visible = true);
	void run();
    void draw();
    void shutdown();
    void make_current();
    void set_vsync(bool enabled);
    std::function<void()> on_shutdown;
	void set_title(const std::string& title);
	void set_status(const std::string& text);
	void set_info(const std::wstring& text);
	glm::dvec2 content_size() const;
	template<typename T, typename F>
	void on(F&& f) {
		get_subs<T>().emplace_back(std::forward<F>(f));
	}
	template<typename T, typename... Ts>
	auto load_model_async(const std::string& key = "", Ts&&... args) -> std::future<std::shared_ptr<model>> {
		return pool_.enqueue([args = std::make_tuple(std::forward<Ts>(args)...)]() mutable {
			return std::apply([](auto&&... xs) -> std::shared_ptr<model> {
				return std::make_shared<T>(std::forward<decltype(xs)>(xs)...);
			}, std::move(args));
		});
	}
	template<typename F>
	auto async(F&& f) { return pool_.enqueue(std::forward<F>(f)); }
	wxFrame* window{ nullptr };
    wxGLCanvas* canvas{ nullptr };
	glm::vec2 viewport{ 1280, 800 };
	std::map<std::string, std::shared_ptr<program>> programs;
	std::function<void(bool)> on_mouse_button;
	std::function<void(double, double)> on_cursor_pos;
	std::function<void(double)> on_scroll;
};

class sky_background;

class renderer_system {
public:
	renderer_system();
	~renderer_system();
	void process(ecs_s::registry& world, std::chrono::nanoseconds ns);
	glm::mat4 get_view() { return cam_->get_view(); }
	glm::mat4 get_projection();
	std::shared_ptr<camera> cam_{ std::make_shared<camera>() };
	std::shared_ptr<directional_light> l;
	std::shared_ptr<sky_background> background;
	bool show_background{ true };
	glm::vec2 mouse_pos{ 640, 400 };
	float z_near{ 100.0f }, z_far{ 50000000.0f };
};
