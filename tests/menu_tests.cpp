#include "test_context.h"
#include "app_menu.h"
#include "app_resources.h"
#include <cstdlib>
void check(bool condition, const char* message) { if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); } }
int main() {
    test_context context; auto& engine = de2::get_instance(); renderer_system renderer; app_menu menu(renderer);
    auto* bar = engine.window->GetMenuBar();
    check(bar && bar->GetMenuCount() == 5, "the portable menu has five top-level menus");
    check(bar->GetMenu(1)->GetMenuItemCount() == 3, "city, terrain, and satellite are available");
    auto preferences = app_settings::instance().snapshot();
    check(bar->IsChecked(ID_MAP_CITY + int(preferences.style)), "the selected map is checked");
    wxCommandEvent zoom(wxEVT_MENU, ID_VIEW_ZOOM_IN); engine.window->ProcessWindowEvent(zoom);
    check(renderer.cam_->zoom_ == 3, "the zoom command controls the camera");
    wxCommandEvent reset(wxEVT_MENU, ID_VIEW_RESET); engine.window->ProcessWindowEvent(reset);
    check(renderer.cam_->zoom_ == 2, "reset restores the default camera");
    for (auto size : {wxSize(1280,800), wxSize(520,360)}) {
        engine.window->SetClientSize(size); wxYieldIfNeeded(); engine.draw();
        auto logical = engine.content_size(); auto scale = engine.canvas->GetContentScaleFactor();
        GLint viewport[4]{}; glGetIntegerv(GL_VIEWPORT, viewport);
        check(viewport[2] == std::lround(logical.x * scale) && viewport[3] == std::lround(logical.y * scale), "menu layout does not corrupt the GL viewport");
        check(glGetError() == GL_NO_ERROR, "menu resizing keeps OpenGL valid");
    }
    std::cout << "Passed: portable menus, selected style, camera commands, and viewport resizing.\n";
}
