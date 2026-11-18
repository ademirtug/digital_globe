#include "de2.h"
#include "spheroid.h"
#include "sky.h"
#include "app_menu.h"
#include <wx/app.h>
#include <wx/msgdlg.h>

class GlobeApp final : public wxApp {
    std::unique_ptr<ecs_s::registry> world_;
    std::unique_ptr<spheroid> globe_;
    std::unique_ptr<renderer_system> renderer_;
    std::unique_ptr<app_menu> menu_;
    std::unique_ptr<wxTimer> smoke_timer_;
    bool smoke_{ false };
    int result_{ 0 };
public:
    bool OnInit() override {
        SetAppName("digital_globe");
        for (int i = 1; i < argc; ++i) if (argv[i] == "--smoke-test") smoke_ = true;
        try {
            auto& engine = de2::get_instance();
            engine.on_shutdown = [this] { menu_.reset(); renderer_.reset(); world_.reset(); globe_.reset(); };
            engine.init();
            engine.programs["c_t_direct"] = std::make_shared<program>("c_t_direct", "shaders/c_t_direct.vert", "shaders/c_t_direct.frag");
            world_ = std::make_unique<ecs_s::registry>(); globe_ = std::make_unique<spheroid>(earth_a, earth_b, 8);
            renderer_ = std::make_unique<renderer_system>(); renderer_->background = std::make_shared<sky_background>();
            menu_ = std::make_unique<app_menu>(*renderer_);
            engine.on<pre_render>([this](std::chrono::nanoseconds) { if (!smoke_) globe_->process(*world_, *renderer_); });
            engine.on<render>([this](std::chrono::nanoseconds elapsed) { renderer_->process(*world_, elapsed); });
            if (smoke_) {
                smoke_timer_ = std::make_unique<wxTimer>(engine.window);
                engine.window->Bind(wxEVT_TIMER, [this, &engine](wxTimerEvent&) {
                    engine.draw(); if (glGetError() != GL_NO_ERROR) result_ = 1;
                    engine.window->Close();
                }, smoke_timer_->GetId());
                smoke_timer_->StartOnce(1500);
            }
            return true;
        } catch (const std::exception& error) {
            if (smoke_) std::cerr << error.what() << '\n';
            else wxMessageBox(wxString::FromUTF8(error.what()), "Digital Globe - Application error", wxOK | wxICON_ERROR);
            de2::get_instance().shutdown(); return false;
        }
    }
    int OnRun() override { int code = wxApp::OnRun(); return result_ ? result_ : code; }
    int OnExit() override { smoke_timer_.reset(); de2::get_instance().shutdown(); return 0; }
};
wxIMPLEMENT_APP(GlobeApp);
