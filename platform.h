#pragma once
#include <filesystem>
#include <wx/string.h>

std::filesystem::path path_from_wx(const wxString& value);
wxString path_to_wx(const std::filesystem::path& value);
std::filesystem::path resource_path(const std::filesystem::path& relative);
std::filesystem::path configuration_directory();
std::filesystem::path default_cache_directory();
bool atomic_replace(const std::filesystem::path& source, const std::filesystem::path& destination);
bool ordinary_directory(const std::filesystem::path& path);
