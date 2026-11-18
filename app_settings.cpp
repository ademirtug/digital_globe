#include "app_settings.h"
#include "util.h"
#include "platform.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>

namespace {
	constexpr map_style styles[]{ map_style::city, map_style::terrain, map_style::satellite };
	bool tile_filename(const std::filesystem::path& path) {
		auto name = path.filename().u8string();
		return name.size() > 4 && name.size() <= 27 && name.ends_with(u8".png") &&
			name.find_first_not_of(u8"abcd") == name.size() - 4;
	}
	std::string utf8(const std::filesystem::path& path) {
		auto value = path.u8string();
		return { reinterpret_cast<const char*>(value.data()), value.size() };
	}
}

const map_source& source_for(map_style style) {
	static const map_source sources[]{
		{ "osm", L"City map", L"Map data: \u00a9 OpenStreetMap contributors\nhttps://www.openstreetmap.org/copyright", 19 },
		{ "terrain", L"Terrain map", L"Esri World Topographic Map\nSources: Esri, HERE, Garmin, Intermap, increment P Corp., GEBCO, USGS, FAO, NPS, NRCan, GeoBase, IGN, Kadaster NL, Ordnance Survey, Esri Japan, METI, Esri China (Hong Kong), OpenStreetMap contributors, and the GIS User Community.\nhttps://services.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer", 19 },
		{ "satellite", L"Satellite imagery", L"Esri World Imagery\nSource: Esri, Vantor, Earthstar Geographics, and the GIS User Community.\nhttps://services.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer", 19 }
	};
	auto index = static_cast<unsigned>(style);
	return sources[index < 3 ? index : 0];
}

std::string map_tile_url(map_style style, const std::string& plate_path) {
	auto square = path_to_square(plate_path);
	auto x = square.x / 256;
	auto y = (size_t(1) << plate_path.size()) - 1 - square.y / 256;
	if (style == map_style::city) return std::format("https://tile.openstreetmap.org/{}/{}/{}.png", plate_path.size(), x, y);
	return std::format("https://services.arcgisonline.com/ArcGIS/rest/services/{}/MapServer/tile/{}/{}/{}",
		style == map_style::terrain ? "World_Topo_Map" : "World_Imagery", plate_path.size(), y, x);
}

std::filesystem::path application_data_directory() {
	return configuration_directory();
}

app_preferences default_preferences() {
	app_preferences preferences;
	preferences.cache_directory = default_cache_directory();
	return preferences;
}

app_preferences load_preferences(const std::filesystem::path& file) {
	auto preferences = default_preferences();
	std::ifstream input(file, std::ios::binary);
	std::string line;
	while (std::getline(input, line)) {
		auto separator = line.find('=');
		if (separator == std::string::npos) continue;
		auto key = line.substr(0, separator), value = line.substr(separator + 1);
		if (!value.empty() && value.back() == '\r') value.pop_back();
		try {
			if (key == "cache_directory") {
				auto path = std::filesystem::u8path(value);
				if (path.is_absolute()) preferences.cache_directory = path.lexically_normal();
			}
			else if (key == "cache_limit_mb") {
				size_t end = 0;
				auto limit = std::stoull(value, &end);
				if (end == value.size() && limit <= 1048576 && !value.starts_with('-')) preferences.cache_limit_mb = limit;
			}
			else if (key == "style" && value.size() == 1 && value[0] >= '0' && value[0] <= '2') preferences.style = map_style(value[0] - '0');
			else if (value == "0" || value == "1") {
				if (key == "offline") preferences.offline = value == "1";
				else if (key == "show_stars") preferences.show_stars = value == "1";
				else if (key == "vsync") preferences.vsync = value == "1";
				else if (key == "decimal_coordinates") preferences.decimal_coordinates = value == "1";
			}
		}
		catch (const std::exception&) { /* Ignore an invalid setting and keep its default. */ }
	}
	return preferences;
}

void save_preferences(const std::filesystem::path& file, const app_preferences& preferences) {
	if (file.empty() || !file.is_absolute()) throw std::runtime_error("The settings folder is unavailable.");
	std::filesystem::create_directories(file.parent_path());
	auto temporary = file;
	temporary += L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	output << "style=" << unsigned(preferences.style) << "\ncache_directory=" << utf8(preferences.cache_directory)
		<< "\ncache_limit_mb=" << preferences.cache_limit_mb << "\noffline=" << preferences.offline
		<< "\nshow_stars=" << preferences.show_stars << "\nvsync=" << preferences.vsync
		<< "\ndecimal_coordinates=" << preferences.decimal_coordinates << '\n';
	output.close();
	if (!output || !atomic_replace(temporary, file))
		throw std::runtime_error("Could not save settings. Check that your app data folder is writable.");
}

app_settings::app_settings() {
	auto directory = application_data_directory();
	preferences_ = directory.empty() ? default_preferences() : load_preferences(directory / L"settings.ini");
}
app_settings& app_settings::instance() { static app_settings settings; return settings; }
app_preferences app_settings::snapshot() const { std::scoped_lock lock(mutex_); return preferences_; }
uint64_t app_settings::tile_revision() const { std::scoped_lock lock(mutex_); return tile_revision_; }
void app_settings::update(const app_preferences& preferences) {
	auto directory = application_data_directory();
	if (directory.empty()) throw std::runtime_error("Could not locate your app data folder.");
	save_preferences(directory / L"settings.ini", preferences);
	std::scoped_lock lock(mutex_);
	if (preferences.style != preferences_.style || preferences.offline != preferences_.offline || preferences.cache_directory != preferences_.cache_directory) ++tile_revision_;
	preferences_ = preferences;
}

tile_cache::tile_cache(std::filesystem::path root, uint64_t limit) : root_(std::move(root)), limit_(limit) { scan(); trim(); }
uint64_t tile_cache::generation() const { std::scoped_lock lock(mutex_); return generation_; }
cache_usage tile_cache::usage() const { std::scoped_lock lock(mutex_); return { bytes_, entries_.size() }; }
std::filesystem::path tile_cache::tile_path(map_style style, const std::string& filename) const {
	if (root_.empty() || !root_.is_absolute() || !tile_filename(filename)) return {};
	return root_ / source_for(style).directory / filename;
}

void tile_cache::scan() {
	entries_.clear(); bytes_ = 0;
	if (root_.empty() || !root_.is_absolute()) return;
	for (auto style : styles) {
		auto directory = root_ / source_for(style).directory;
		if (!ordinary_directory(directory)) continue;
		std::error_code error;
		for (std::filesystem::directory_iterator it(directory, error), end; !error && it != end; it.increment(error)) {
			if (!it->is_regular_file(error) || it->is_symlink(error) || !tile_filename(it->path().filename())) continue;
			auto size = it->file_size(error);
			if (error) break;
			auto time = it->last_write_time(error);
			if (error) break;
			entries_.emplace(it->path(), entry{ size, time }); bytes_ += size;
		}
	}
}

void tile_cache::trim() {
	while (bytes_ > limit_ && !entries_.empty()) {
		auto oldest = std::min_element(entries_.begin(), entries_.end(), [](const auto& a, const auto& b) { return a.second.used < b.second.used; });
		std::error_code error;
		std::filesystem::remove(oldest->first, error);
		if (error) break;
		bytes_ -= oldest->second.bytes;
		entries_.erase(oldest);
	}
}

std::vector<unsigned char> tile_cache::read(map_style style, const std::string& filename) {
	std::scoped_lock lock(mutex_);
	auto path = tile_path(style, filename);
	if (path.empty() || !ordinary_directory(path.parent_path())) return {};
	std::ifstream input(path, std::ios::binary);
	if (!input) return {};
	std::vector<unsigned char> bytes{ std::istreambuf_iterator<char>(input), {} };
	if (input.bad()) return {};
	std::error_code error;
	auto now = std::filesystem::file_time_type::clock::now();
	std::filesystem::last_write_time(path, now, error);
	if (entries_.contains(path)) entries_.at(path).used = now;
	return bytes;
}

void tile_cache::write(map_style style, const std::string& filename, const std::vector<unsigned char>& data, uint64_t generation) {
	std::scoped_lock lock(mutex_);
	if (generation != generation_ || data.empty() || data.size() > limit_) return;
	auto path = tile_path(style, filename);
	if (path.empty()) return;
	std::error_code error;
	std::filesystem::create_directories(path.parent_path(), error);
	if (error || !ordinary_directory(path.parent_path())) return;
	auto temporary = path; temporary += L".part";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	output.write(reinterpret_cast<const char*>(data.data()), data.size()); output.close();
	if (!output || !atomic_replace(temporary, path)) {
		std::filesystem::remove(temporary, error); return;
	}
	if (entries_.contains(path)) bytes_ -= entries_.at(path).bytes;
	entries_[path] = { data.size(), std::filesystem::file_time_type::clock::now() };
	bytes_ += data.size();
	trim();
}

void tile_cache::configure(const std::filesystem::path& root, uint64_t limit, bool copy_existing) {
	std::scoped_lock lock(mutex_);
	if (root.empty() || !root.is_absolute() || root == root.root_path()) throw std::runtime_error("Choose an app-specific cache folder.");
	std::filesystem::create_directories(root);
	if (!ordinary_directory(root)) throw std::runtime_error("Choose a regular folder for the map cache.");
	auto probe = root / L".digital_globe_write_test";
	{ std::ofstream output(probe); if (!output) throw std::runtime_error("The selected cache folder is not writable."); }
	std::filesystem::remove(probe);
	if (root != root_ && copy_existing) {
		for (const auto& [path, item] : entries_) {
			auto destination = root / path.parent_path().filename() / path.filename();
			std::filesystem::create_directories(destination.parent_path());
			if (!ordinary_directory(destination.parent_path())) throw std::runtime_error("A cache subfolder is not a regular directory.");
			std::filesystem::copy_file(path, destination, std::filesystem::copy_options::skip_existing);
		}
	}
	root_ = root; limit_ = limit; ++generation_;
	scan(); trim();
}

void tile_cache::clear() {
	std::scoped_lock lock(mutex_);
	++generation_; // Downloads started before clearing cannot recreate the cache.
	scan();
	std::error_code first_error;
	for (const auto& [path, item] : entries_) {
		std::error_code error; std::filesystem::remove(path, error);
		if (error && !first_error) first_error = error;
	}
	scan();
	if (first_error) throw std::system_error(first_error, "Some cached tiles could not be removed");
}

tile_cache& shared_tile_cache() {
	static tile_cache cache(app_settings::instance().snapshot().cache_directory, app_settings::instance().snapshot().cache_limit_mb * 1024 * 1024);
	return cache;
}
