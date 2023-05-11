#pragma once

#include <string>
#include "http_client.h"
#include "include/../model.h"
#include "stb_image.h"
#include "util.h"

class null_store {
public:
	std::shared_ptr<texture> get(const std::string& plate_path) {
		return nullptr;
	}
	void put(const std::vector<unsigned char>& data) {
		//do nothing
	}
};

class disk_store {
public:
	std::shared_ptr<texture> get(const std::string& plate_path) {
		if (!std::filesystem::exists("C:\\mapdata\\" + plate_path + ".png")) {
			std::cout << "disk_store " << plate_path << " does not exist" << std::endl;
			return nullptr;
		}
		return std::make_shared<texture>("C:\\mapdata\\" + plate_path + ".png");
	}
	void put(const std::vector<unsigned char>& data) {
		//TODO save the file
	}
};

template<typename T>
class map_quest {
	T permanent_store_;
public:
	std::shared_ptr<texture> get(const std::string& plate_path) {
		std::shared_ptr<texture> tex = permanent_store_.get(plate_path);
		if (tex != nullptr)
			return tex;


		size_t map_size = (size_t)std::pow(2, plate_path.size()) * 256;
		box b = path_to_box(plate_path);
		auto ecef = merc_to_ecef({ b.x + b.a / 2, b.y + b.a / 2, 0 }, map_size);
		std::string http_request = std::format("https://www.mapquestapi.com/staticmap/v5/map?key={}&format=png&center={},{}&size=256,256&zoom={}&type=sat", 
			"ohT06mfFYTRpy9dV4SNdaFh8jsXUEnhc", merc_y_to_lat(b.y + b.a / 2, map_size), merc_x_to_lon(b.x+b.a/2, map_size), plate_path.size());

		std::cout << "map_quest::get -> " << plate_path << " " << std::endl;
		int w, h, c;
		auto png = http_client::get_binary_page(http_request);
		return std::make_shared<texture>(stbi_load_from_memory(png.data(), png.size(), &w, &h, &c, STBI_rgb_alpha));
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
