#pragma once

#include <filesystem>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

enum class map_style { city, terrain, satellite };

struct map_source {
	const char* directory;
	const wchar_t* name;
	const wchar_t* credits;
	size_t maximum_zoom;
};

const map_source& source_for(map_style style);
std::string map_tile_url(map_style style, const std::string& plate_path);
std::filesystem::path application_data_directory();

struct app_preferences {
	map_style style{ map_style::city };
	std::filesystem::path cache_directory;
	uint64_t cache_limit_mb{ 1024 };
	bool offline{ false }, show_stars{ true }, vsync{ true }, decimal_coordinates{ false };
};

app_preferences default_preferences();
app_preferences load_preferences(const std::filesystem::path& file);
void save_preferences(const std::filesystem::path& file, const app_preferences& preferences);

class app_settings {
	mutable std::mutex mutex_;
	app_preferences preferences_;
	uint64_t tile_revision_{ 0 };
	app_settings();
public:
	static app_settings& instance();
	app_preferences snapshot() const;
	uint64_t tile_revision() const;
	void update(const app_preferences& preferences);
};

struct cache_usage { uint64_t bytes{ 0 }; size_t files{ 0 }; };

class tile_cache {
	struct entry { uint64_t bytes; std::filesystem::file_time_type used; };
	mutable std::mutex mutex_;
	std::filesystem::path root_;
	uint64_t limit_, bytes_{ 0 }, generation_{ 0 };
	std::map<std::filesystem::path, entry> entries_;
	void scan();
	void trim();
	std::filesystem::path tile_path(map_style style, const std::string& filename) const;
public:
	tile_cache(std::filesystem::path root, uint64_t limit);
	uint64_t generation() const;
	cache_usage usage() const;
	std::vector<unsigned char> read(map_style style, const std::string& filename);
	void write(map_style style, const std::string& filename, const std::vector<unsigned char>& data, uint64_t generation);
	void configure(const std::filesystem::path& root, uint64_t limit, bool copy_existing = true);
	void clear();
};

tile_cache& shared_tile_cache();
