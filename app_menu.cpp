#include "app_menu.h"
#include "app_resources.h"
#include "platform.h"
#include <wx/wx.h>
#include <wx/choice.h>
#include <wx/dirdlg.h>
#include <wx/statline.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/valnum.h>

app_menu::app_menu(renderer_system& renderer) : renderer_(renderer), window_(de2::get_instance().window) {
    menu_ = new wxMenuBar;
    auto* file = new wxMenu; file->Append(ID_APP_EXIT, "E&xit"); menu_->Append(file, "&File");
    auto* maps = new wxMenu;
    maps->AppendRadioItem(ID_MAP_CITY, "&City map"); maps->AppendRadioItem(ID_MAP_TERRAIN, "&Terrain map");
    maps->AppendRadioItem(ID_MAP_SATELLITE, "&Satellite imagery"); menu_->Append(maps, "&Map");
    auto* view = new wxMenu;
    view->Append(ID_VIEW_RESET, "&Reset view\tF5"); view->Append(ID_VIEW_ZOOM_IN, "Zoom &in"); view->Append(ID_VIEW_ZOOM_OUT, "Zoom &out");
    view->AppendSeparator(); view->AppendCheckItem(ID_VIEW_STARS, "Show &stars");
    auto* coordinates = new wxMenu;
    coordinates->AppendRadioItem(ID_COORD_DMS, "Degrees, minutes, seconds"); coordinates->AppendRadioItem(ID_COORD_DECIMAL, "Decimal degrees");
    view->AppendSubMenu(coordinates, "&Coordinates"); menu_->Append(view, "&View");
    auto* settings = new wxMenu;
    settings->Append(ID_SETTINGS, "&Preferences...\tF2"); settings->AppendCheckItem(ID_OFFLINE, "Work &offline");
    settings->AppendSeparator(); settings->Append(ID_CACHE_OPEN, "Open cache folder"); settings->Append(ID_CACHE_CLEAR, "Clear downloaded maps...");
    menu_->Append(settings, "&Settings");
    auto* help = new wxMenu; help->Append(ID_HELP_CONTROLS, "&Controls"); help->Append(ID_HELP_CREDITS, "Map sources && &credits"); menu_->Append(help, "&Help");
    window_->SetMenuBar(menu_);
    window_->Bind(wxEVT_MENU, &app_menu::on_command, this);
    window_->Bind(wxEVT_MENU_OPEN, &app_menu::on_open, this);
    auto preferences = app_settings::instance().snapshot();
    renderer_.show_background = preferences.show_stars;
    de2::get_instance().set_vsync(preferences.vsync);
    refresh();
}
app_menu::~app_menu() {
    window_->Unbind(wxEVT_MENU, &app_menu::on_command, this);
    window_->Unbind(wxEVT_MENU_OPEN, &app_menu::on_open, this);
}
void app_menu::on_command(wxCommandEvent& event) {
    try { command(event.GetId()); }
    catch (const std::exception& error) { wxMessageBox(wxString::FromUTF8(error.what()), "Digital Globe", wxOK | wxICON_ERROR, window_); }
}
void app_menu::on_open(wxMenuEvent& event) { refresh(); event.Skip(); }
void app_menu::refresh() {
    auto preferences = app_settings::instance().snapshot();
    menu_->Check(ID_MAP_CITY + int(preferences.style), true);
    menu_->Check(preferences.decimal_coordinates ? ID_COORD_DECIMAL : ID_COORD_DMS, true);
    menu_->Check(ID_OFFLINE, preferences.offline); menu_->Check(ID_VIEW_STARS, preferences.show_stars);
    de2::get_instance().set_info(std::wstring(source_for(preferences.style).name) +
        (preferences.offline ? L" | Offline - cached maps only" : L" | Online") + L" | F2 Settings");
}
void app_menu::apply(const app_preferences& preferences) {
    auto previous = app_settings::instance().snapshot();
    bool cache_changed = previous.cache_directory != preferences.cache_directory || previous.cache_limit_mb != preferences.cache_limit_mb;
    if (cache_changed) shared_tile_cache().configure(preferences.cache_directory, preferences.cache_limit_mb * 1024 * 1024);
    try { app_settings::instance().update(preferences); }
    catch (...) { if (cache_changed) shared_tile_cache().configure(previous.cache_directory, previous.cache_limit_mb * 1024 * 1024, false); throw; }
    renderer_.show_background = preferences.show_stars;
    de2::get_instance().set_vsync(preferences.vsync); refresh();
}
void app_menu::command(int id) {
    auto preferences = app_settings::instance().snapshot();
    switch (id) {
    case ID_APP_EXIT: window_->Close(); return;
    case ID_MAP_CITY: case ID_MAP_TERRAIN: case ID_MAP_SATELLITE: preferences.style = map_style(id - ID_MAP_CITY); break;
    case ID_OFFLINE: preferences.offline = !preferences.offline; break;
    case ID_VIEW_STARS: preferences.show_stars = !preferences.show_stars; break;
    case ID_COORD_DMS: case ID_COORD_DECIMAL: preferences.decimal_coordinates = id == ID_COORD_DECIMAL; break;
    case ID_VIEW_RESET: *renderer_.cam_ = camera(); return;
    case ID_VIEW_ZOOM_IN: case ID_VIEW_ZOOM_OUT: {
        auto viewport = de2::get_instance().content_size();
        renderer_.cam_->set_projection(viewport, renderer_.get_projection());
        renderer_.cam_->scroll(id == ID_VIEW_ZOOM_IN ? 1 : -1, viewport / 2.0); return;
    }
    case ID_SETTINGS: preferences_dialog(); return;
    case ID_CACHE_OPEN: open_cache(); return;
    case ID_CACHE_CLEAR: clear_cache(window_); return;
    case ID_HELP_CONTROLS:
        wxMessageBox("Drag to pan. Use the mouse wheel to zoom toward the pointer.\n\nF2 - Preferences\nF5 - Reset view\n\nOffline mode uses cached tiles for the selected style. Terrain uses topographic imagery, not an elevation mesh.\n\nOn wxWidgets 3.2, the VSync setting limits refresh to 60 FPS; hardware synchronization depends on the graphics driver.", "Digital Globe - Controls", wxOK | wxICON_INFORMATION, window_); return;
    case ID_HELP_CREDITS:
        wxMessageBox(wxString(source_for(preferences.style).credits), "Digital Globe - Map sources and credits", wxOK | wxICON_INFORMATION, window_); return;
    default: return;
    }
    apply(preferences);
}
void app_menu::open_cache() {
    auto folder = app_settings::instance().snapshot().cache_directory;
    std::filesystem::create_directories(folder);
    if (!wxLaunchDefaultBrowser(wxFileSystem::FileNameToURL(path_to_wx(folder)))) throw std::runtime_error("Could not open the cache folder.");
}
void app_menu::clear_cache(wxWindow* owner) {
    auto usage = shared_tile_cache().usage();
    if (!usage.files) { wxMessageBox("The map cache is already empty.", "Digital Globe", wxOK | wxICON_INFORMATION, owner); return; }
    auto question = wxString::Format("Delete %llu downloaded tiles (%.1f MB) from:\n\n", static_cast<unsigned long long>(usage.files), double(usage.bytes) / (1024 * 1024)) +
        path_to_wx(app_settings::instance().snapshot().cache_directory) + "\n\nOffline access to these tiles will be lost. Other files and settings are kept.";
    if (wxMessageBox(question, "Clear downloaded maps?", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, owner) == wxYES) shared_tile_cache().clear();
}
void app_menu::preferences_dialog() {
    auto draft = app_settings::instance().snapshot();
    wxDialog dialog(window_, wxID_ANY, "Digital Globe - Preferences", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    auto* layout = new wxBoxSizer(wxVERTICAL);
    auto* display = new wxStaticBoxSizer(wxVERTICAL, &dialog, "Map and display");
    auto* grid = new wxFlexGridSizer(2, dialog.FromDIP(8), dialog.FromDIP(12)); grid->AddGrowableCol(1);
    grid->Add(new wxStaticText(&dialog, wxID_ANY, "Map style"), 0, wxALIGN_CENTER_VERTICAL);
    auto* style = new wxChoice(&dialog, wxID_ANY); for (auto map : { map_style::city, map_style::terrain, map_style::satellite }) style->Append(wxString(source_for(map).name));
    style->SetSelection(int(draft.style)); grid->Add(style, 1, wxEXPAND);
    grid->Add(new wxStaticText(&dialog, wxID_ANY, "Coordinates"), 0, wxALIGN_CENTER_VERTICAL);
    auto* coordinates = new wxChoice(&dialog, wxID_ANY); coordinates->Append("Degrees, minutes, seconds"); coordinates->Append("Decimal degrees");
    coordinates->SetSelection(draft.decimal_coordinates ? 1 : 0); grid->Add(coordinates, 1, wxEXPAND);
    display->Add(grid, 0, wxEXPAND | wxALL, dialog.FromDIP(8));
    auto* toggles = new wxBoxSizer(wxHORIZONTAL);
    auto* offline = new wxCheckBox(&dialog, wxID_ANY, "Work offline"); offline->SetValue(draft.offline);
    auto* stars = new wxCheckBox(&dialog, wxID_ANY, "Show stars"); stars->SetValue(draft.show_stars);
    auto* vsync = new wxCheckBox(&dialog, wxID_ANY, "VSync / 60 FPS limit"); vsync->SetValue(draft.vsync);
    for (auto* control : { offline, stars, vsync }) toggles->Add(control, 0, wxALL, dialog.FromDIP(8));
    display->Add(toggles); layout->Add(display, 0, wxEXPAND | wxALL, dialog.FromDIP(10));
    auto* cache = new wxStaticBoxSizer(wxVERTICAL, &dialog, "Downloaded map cache");
    auto* folder_row = new wxBoxSizer(wxHORIZONTAL);
    auto* folder = new wxTextCtrl(&dialog, wxID_ANY, path_to_wx(draft.cache_directory), wxDefaultPosition, dialog.FromDIP(wxSize(380, -1)), wxTE_READONLY);
    auto* browse = new wxButton(&dialog, wxID_ANY, "Change..."); auto* reset = new wxButton(&dialog, wxID_ANY, "Default");
    folder_row->Add(folder, 1, wxRIGHT | wxEXPAND, dialog.FromDIP(8)); folder_row->Add(browse); folder_row->Add(reset, 0, wxLEFT, dialog.FromDIP(8));
    cache->Add(folder_row, 0, wxEXPAND | wxALL, dialog.FromDIP(8));
    cache->Add(new wxStaticText(&dialog, wxID_ANY, "Existing tiles are copied when you save a new cache location."), 0, wxLEFT | wxRIGHT | wxBOTTOM, dialog.FromDIP(8));
    auto* limit_row = new wxBoxSizer(wxHORIZONTAL);
    limit_row->Add(new wxStaticText(&dialog, wxID_ANY, "Size limit (MB)"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, dialog.FromDIP(8));
    auto* limit = new wxTextCtrl(&dialog, wxID_ANY, wxString::Format("%llu", static_cast<unsigned long long>(draft.cache_limit_mb)), wxDefaultPosition, dialog.FromDIP(wxSize(90, -1)));
    limit_row->Add(limit); limit_row->Add(new wxStaticText(&dialog, wxID_ANY, "0 disables disk caching"), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, dialog.FromDIP(10));
    cache->Add(limit_row, 0, wxALL, dialog.FromDIP(8));
    cache->Add(new wxStaticText(&dialog, wxID_ANY, "Oldest tiles are removed automatically at the size limit."), 0, wxLEFT | wxRIGHT | wxBOTTOM, dialog.FromDIP(8));
    auto* bottom = new wxBoxSizer(wxHORIZONTAL);
    auto* usage = new wxStaticText(&dialog, wxID_ANY, "");
    auto refresh_usage = [&] { auto value = shared_tile_cache().usage(); usage->SetLabel(wxString::Format("%.1f MB in %llu tiles", double(value.bytes)/(1024*1024), static_cast<unsigned long long>(value.files))); };
    refresh_usage(); bottom->Add(usage, 1, wxALIGN_CENTER_VERTICAL);
    auto* open = new wxButton(&dialog, wxID_ANY, "Open folder"); auto* clear = new wxButton(&dialog, wxID_ANY, "Clear cache...");
    bottom->Add(open); bottom->Add(clear, 0, wxLEFT, dialog.FromDIP(8)); cache->Add(bottom, 0, wxEXPAND | wxALL, dialog.FromDIP(8));
    layout->Add(cache, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, dialog.FromDIP(10));
    layout->Add(new wxStaticText(&dialog, wxID_ANY, "Offline mode uses only downloaded maps for the selected style."), 0, wxLEFT | wxRIGHT | wxBOTTOM, dialog.FromDIP(12));
    layout->Add(dialog.CreateSeparatedButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, dialog.FromDIP(10));
    dialog.FindWindow(wxID_OK)->SetLabel("Save");
    dialog.SetSizerAndFit(layout); dialog.SetMinSize(dialog.GetSize()); dialog.CentreOnParent();
    browse->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { wxDirDialog chooser(&dialog, "Choose a location for the Digital Globe cache", path_to_wx(draft.cache_directory.parent_path())); if (chooser.ShowModal() == wxID_OK) { draft.cache_directory = path_from_wx(chooser.GetPath()) / "digital_globe_cache"; folder->SetValue(path_to_wx(draft.cache_directory)); } });
    reset->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { draft.cache_directory = default_preferences().cache_directory; folder->SetValue(path_to_wx(draft.cache_directory)); });
    open->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { try { open_cache(); } catch(const std::exception& error) { wxMessageBox(wxString::FromUTF8(error.what()), "Cache", wxOK | wxICON_ERROR, &dialog); } });
    clear->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { try { clear_cache(&dialog); refresh_usage(); } catch(const std::exception& error) { wxMessageBox(wxString::FromUTF8(error.what()), "Cache", wxOK | wxICON_ERROR, &dialog); } });
    dialog.Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        unsigned long long size = 0;
        if (!limit->GetValue().ToULongLong(&size) || size > 1048576) { wxMessageBox("Enter a size between 0 and 1,048,576 MB.", "Cache limit", wxOK | wxICON_WARNING, &dialog); limit->SetFocus(); return; }
        draft.cache_limit_mb = size; draft.style = map_style(style->GetSelection()); draft.decimal_coordinates = coordinates->GetSelection() == 1;
        draft.offline = offline->GetValue(); draft.show_stars = stars->GetValue(); draft.vsync = vsync->GetValue();
        try { wxBusyCursor wait; apply(draft); dialog.EndModal(wxID_OK); }
        catch (const std::exception& error) { wxMessageBox(wxString::FromUTF8(error.what()), "Preferences", wxOK | wxICON_ERROR, &dialog); }
    }, wxID_OK);
    dialog.ShowModal();
}
