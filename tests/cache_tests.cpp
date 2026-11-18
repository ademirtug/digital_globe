#include "app_settings.h"
#include "platform.h"
#include <wx/init.h>
#include <wx/utils.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <cstdlib>
#include <fstream>
#include <iostream>

void check(bool condition, const char* message) {
	if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}

int main() {
    wxInitializer runtime;
    check(runtime.IsOk(), "wxWidgets initializes");
	auto defaults = default_preferences();
	check(defaults.cache_directory == default_cache_directory(), "the default cache uses the app's Local App Data directory");
	auto sandbox = std::filesystem::temp_directory_path() / ("digital_globe_cache_tests_" + std::to_string(wxGetProcessId()));
	auto root = sandbox / u8"\u0130stanbul \u5730\u56fe";
	std::filesystem::create_directories(root);
	const std::vector<unsigned char> city{ 1,2,3,4 }, terrain{ 5,6,7,8 };
	tile_cache cache(root, 1024);
	auto generation = cache.generation();
	cache.write(map_style::city, "a.png", city, generation);
	cache.write(map_style::terrain, "a.png", terrain, generation);
	check(cache.read(map_style::city, "a.png") == city && cache.read(map_style::terrain, "a.png") == terrain, "map styles never reuse each other's tiles");
	check(cache.read(map_style::satellite, "a.png").empty(), "uncached map styles are cache misses");
	check(std::filesystem::exists(root / "osm/a.png"), "Unicode cache folder names are supported");
	auto original = std::filesystem::current_path();
	std::filesystem::current_path(sandbox);
	tile_cache reopened(root, 1024);
	check(reopened.read(map_style::city, "a.png") == city, "offline tiles survive cache recreation and a different working directory");
	std::filesystem::current_path(original);
#ifdef _WIN32
	auto locked = CreateFileW((root / "osm/a.png").c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	check(locked != INVALID_HANDLE_VALUE, "the cache test locks its own tile");
	check(cache.read(map_style::city, "a.png").empty(), "unreadable cache files are cache misses");
	CloseHandle(locked);
#endif
	cache.write(map_style::city, "../escape.png", city, generation);
	check(!std::filesystem::exists(root / "escape.png"), "tile names cannot escape the cache directory");

	auto moved = sandbox / L"moved";
	cache.configure(moved, 1024);
	check(cache.read(map_style::city, "a.png") == city && cache.read(map_style::terrain, "a.png") == terrain, "changing cache folders preserves tiles for every style");
	check(std::filesystem::exists(root / "osm/a.png"), "the original files remain available if a location change fails later");
	cache.write(map_style::city, "b.png", city, generation);
	check(cache.read(map_style::city, "b.png").empty(), "a download started before a location change cannot write into the new cache");
	{ std::ofstream note(moved / "osm/notes.txt"); note << "keep"; }
	{ std::ofstream note(moved / "osm/vacation.png"); note << "keep"; }
	cache.clear();
	check(cache.usage().bytes == 0 && cache.usage().files == 0, "clear cache removes all managed tiles for all styles");
	check(std::filesystem::exists(moved / "osm/notes.txt") && std::filesystem::exists(moved / "osm/vacation.png"), "clear cache keeps unrelated files");
	cache.write(map_style::city, "a.png", city, generation);
	check(cache.usage().files == 0, "old in-flight downloads cannot repopulate a cleared cache");

	auto quota_root = sandbox / L"quota";
	tile_cache quota(quota_root, 12);
	auto token = quota.generation();
	for (auto name : { "a.png", "b.png", "c.png" }) quota.write(map_style::city, name, city, token);
	std::filesystem::last_write_time(quota_root / "osm/b.png", std::filesystem::file_time_type::clock::now() - std::chrono::hours(24));
	quota.configure(quota_root, 12, false);
	token = quota.generation();
	quota.read(map_style::city, "a.png");
	quota.write(map_style::city, "d.png", city, token);
	check(quota.usage().bytes == 12 && quota.read(map_style::city, "b.png").empty(), "the disk size limit evicts the least recently used tile");
	check(quota.read(map_style::city, "a.png") == city, "recently used tiles are retained");
	quota.configure(quota_root, 4, false);
	check(quota.usage().bytes <= 4, "lowering the size limit trims existing cache files immediately");
	quota.configure(quota_root, 0, false);
	quota.write(map_style::city, "a.png", city, quota.generation());
	check(quota.usage().files == 0, "a zero limit disables disk caching");

	auto preferences = defaults;
	preferences.cache_directory = root;
	preferences.style = map_style::satellite;
	preferences.cache_limit_mb = 256;
	preferences.offline = preferences.decimal_coordinates = true;
	preferences.show_stars = preferences.vsync = false;
	auto settings_file = sandbox / L"settings.ini";
	save_preferences(settings_file, preferences);
	auto loaded = load_preferences(settings_file);
	check(loaded.cache_directory == root && loaded.cache_limit_mb == 256 && loaded.style == map_style::satellite && loaded.offline && loaded.decimal_coordinates && !loaded.show_stars && !loaded.vsync, "all settings persist, including Unicode cache locations");
	{ std::ofstream invalid(settings_file); invalid << "style=99\ncache_limit_mb=-1\ncache_directory=relative\nvsync=nonsense\n"; }
	loaded = load_preferences(settings_file);
	check(loaded.style == defaults.style && loaded.cache_limit_mb == defaults.cache_limit_mb && loaded.cache_directory == defaults.cache_directory && loaded.vsync, "invalid settings safely fall back to defaults");
	check(map_tile_url(map_style::city, "a").ends_with("/1/0/1.png"), "city tiles use XYZ coordinates");
	check(map_tile_url(map_style::terrain, "a").ends_with("World_Topo_Map/MapServer/tile/1/1/0"), "terrain tiles use the provider's row and column order");
	check(map_tile_url(map_style::satellite, "a").find("World_Imagery") != std::string::npos, "satellite uses its own imagery service");

	for (auto cache_root : { root, moved, quota_root }) {
		for (auto directory : { "osm", "terrain", "satellite" }) {
			auto folder = cache_root / directory;
			if (!std::filesystem::exists(folder)) continue;
			for (auto name : { "a.png", "b.png", "c.png", "d.png", "notes.txt", "vacation.png" }) std::filesystem::remove(folder / name);
			std::filesystem::remove(folder);
		}
		std::filesystem::remove(cache_root);
	}
	std::filesystem::remove(settings_file); std::filesystem::remove(sandbox);
	std::cout << "Passed: cache isolation, Unicode paths, offline persistence, cache relocation, safe clearing, stale downloads, LRU quotas, settings persistence, and map URLs.\n";
}
