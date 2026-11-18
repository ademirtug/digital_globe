#pragma once

#include <string>
#include "http_client.h"
#include "model.h"
#include "stb_image.h"
#include "util.h"
#include "app_settings.h"

class null_store {
public:
	std::shared_ptr<texture> get(const std::string& plate_path, map_style = map_style::city) {
		return nullptr;
	}
	void put(const std::string& file_name, const std::shared_ptr< std::vector<unsigned char> >& data, map_style, uint64_t) {
		//do nothing
	}
};

class disk_store {
public:
	std::shared_ptr<texture> get(const std::string& plate_path, map_style style = map_style::city) {
		auto data = std::make_shared<std::vector<unsigned char>>(shared_tile_cache().read(style, plate_path + ".png"));
		if (data->empty()) return nullptr;
		return std::make_shared<texture>(data);
	}
	void put(const std::string& file_name, const std::shared_ptr<std::vector<unsigned char>>& data, map_style style, uint64_t generation) {
		shared_tile_cache().write(style, file_name, *data, generation);
	}
};

template<typename T>
class open_street_map {
	T permanent_store_;
public:
	std::shared_ptr<texture> get(const std::string& plate_path, map_style style, bool offline) {
		auto generation = shared_tile_cache().generation();
		std::shared_ptr<texture> tex = permanent_store_.get(plate_path, style);
		if (tex != nullptr)
			return tex;
		if (offline) throw std::runtime_error("Tile is not available in the offline cache.");
		auto data = http_client::get_binary_page(map_tile_url(style, plate_path));
		int width, height, channels;
		if (!data->empty() && stbi_info_from_memory(data->data(), (int)data->size(), &width, &height, &channels)) {
			permanent_store_.put(plate_path + ".png", data, style, generation);
		}
		else throw std::runtime_error("The map service did not return a valid tile. Check your connection or try another map style.");
		return std::make_shared<texture>(data);
	}
	std::shared_ptr<texture> get(const std::string& plate_path) {
		auto preferences = app_settings::instance().snapshot();
		return get(plate_path, preferences.style, preferences.offline);
	}
};

template<typename T>
class map_provider {
	T provider;
public:
	std::shared_ptr<texture> get(const std::string path) {
		return provider.get(path);
	}

};
