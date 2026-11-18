#include "platform.h"
#include <wx/stdpaths.h>
#include <wx/utils.h>
#ifdef _WIN32
#include <windows.h>
#endif

std::filesystem::path path_from_wx(const wxString& value) {
#ifdef _WIN32
	return std::filesystem::path(value.ToStdWstring());
#else
	return std::filesystem::u8path(value.ToUTF8().data());
#endif
}
wxString path_to_wx(const std::filesystem::path& value) {
	auto bytes = value.u8string();
	return wxString::FromUTF8(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}
namespace {
	std::filesystem::path user_directory(const char* variable, const char* fallback) {
		wxString value;
		if (wxGetEnv(variable, &value)) {
			auto path = path_from_wx(value);
			if (path.is_absolute()) return path / "digital_globe";
		}
		return path_from_wx(wxGetHomeDir()) / fallback / "digital_globe";
	}
}
std::filesystem::path configuration_directory() {
#ifdef _WIN32
	return path_from_wx(wxStandardPaths::Get().GetUserDir(wxStandardPaths::Dir_Cache)) / "digital_globe";
#else
	return user_directory("XDG_CONFIG_HOME", ".config");
#endif
}
std::filesystem::path default_cache_directory() {
#ifdef _WIN32
	return configuration_directory() / "mapdata";
#else
	return user_directory("XDG_CACHE_HOME", ".cache") / "mapdata";
#endif
}
std::filesystem::path resource_path(const std::filesystem::path& relative) {
	auto executable = path_from_wx(wxStandardPaths::Get().GetExecutablePath()).parent_path();
	for (const auto& root : { executable, executable / ".." / "share" / "digital_globe", std::filesystem::current_path() }) {
		auto path = root / relative;
		if (std::filesystem::exists(path)) return path;
	}
	throw std::runtime_error("Cannot find application resource: " + relative.string());
}
bool atomic_replace(const std::filesystem::path& source, const std::filesystem::path& destination) {
#ifdef _WIN32
	return MoveFileExW(source.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
	std::error_code error;
	std::filesystem::rename(source, destination, error);
	return !error;
#endif
}
bool ordinary_directory(const std::filesystem::path& path) {
#ifdef _WIN32
	auto attributes = GetFileAttributesW(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) && !(attributes & FILE_ATTRIBUTE_REPARSE_POINT);
#else
	std::error_code error;
	return std::filesystem::is_directory(std::filesystem::symlink_status(path, error)) && !error;
#endif
}
