#pragma once

#include "model.h"

class sky_background {
	std::shared_ptr<program> program_;
	std::shared_ptr<texture> image_;
	GLuint vao_{ 0 };
public:
	sky_background();
	~sky_background();
	sky_background(const sky_background&) = delete;
	sky_background& operator=(const sky_background&) = delete;
	void draw(const glm::dmat4& view, const glm::mat4& projection);
};
