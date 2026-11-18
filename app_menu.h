#pragma once
#include "de2.h"
#include "app_settings.h"
#include <wx/menu.h>
class app_menu {
    renderer_system& renderer_;
    wxFrame* window_;
    wxMenuBar* menu_;
    void on_command(wxCommandEvent& event);
    void on_open(wxMenuEvent& event);
    void command(int id);
    void refresh();
    void apply(const app_preferences& preferences);
    void preferences_dialog();
    void clear_cache(wxWindow* owner);
    void open_cache();
public:
    explicit app_menu(renderer_system& renderer);
    ~app_menu();
    app_menu(const app_menu&) = delete;
    app_menu& operator=(const app_menu&) = delete;
};
