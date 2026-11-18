#include "test_context.h"
#include "util.h"
#include <cstdlib>
void check(bool condition, const char* message) { if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); } }
int main() {
    test_context context; auto& engine = de2::get_instance();
    auto* status = engine.window->GetStatusBar();
    check(status && status->GetFieldsCount() == 2, "the globe has a two-field status bar");
    check(engine.window->GetTitle() == "digital_globe", "the title is the application name");
    const std::string coordinates = "41 00' 29\"N - 28 58' 42\"E";
    engine.set_status(coordinates);
    check(status->GetStatusText(1) == wxString::FromUTF8(coordinates), "coordinates appear in the right field");
    renderer_system renderer;
    for (auto size : { wxSize(1280,800), wxSize(960,640), wxSize(520,360) }) {
        engine.window->SetClientSize(size); wxYieldIfNeeded(); engine.draw();
        check(status->GetStatusText(1) == wxString::FromUTF8(coordinates), "resizing preserves the coordinates");
        auto logical = engine.content_size(); auto scale = engine.canvas->GetContentScaleFactor();
        GLint viewport[4]{}; glGetIntegerv(GL_VIEWPORT, viewport);
        check(viewport[0] == 0 && viewport[1] == 0 && viewport[2] == std::lround(logical.x * scale) && viewport[3] == std::lround(logical.y * scale), "the canvas has an independent framebuffer with correct DPI scaling");
        check(logical.x > 0 && logical.y > 0 && status->GetPosition().y >= engine.canvas->GetPosition().y + engine.canvas->GetSize().y, "the status bar is outside the map canvas");
        auto center = logical / 2.0; engine.on_cursor_pos(center.x, center.y);
        check(glm::distance(glm::dvec2(renderer.mouse_pos), glm::dvec2(engine.viewport)/2.0) < 1e-5, "logical pointer coordinates map to physical pixels");
        auto point = ray_hit_ellipsoid(renderer.mouse_pos, engine.viewport, renderer.get_projection(), renderer.cam_->view_matrix());
        check(std::isfinite(point.x), "map picking works in the wxWidgets canvas");
        check(renderer.cam_->begin_drag(center, logical, renderer.get_projection()), "a drag acquires the map surface");
        auto before = renderer.cam_->view_matrix(); engine.on_cursor_pos(center.x, logical.y + 2);
        check(renderer.cam_->view_matrix() == before, "dragging outside the canvas cannot rotate the globe");
        renderer.cam_->end_drag(); check(glGetError() == GL_NO_ERROR, "resizing leaves OpenGL valid");
    }
    engine.set_status(""); check(status->GetStatusText(1).empty(), "coordinates can be cleared");
    std::cout << "Passed: wxWidgets status bar, resize, HiDPI mapping, picking, and input boundaries.\n";
}
