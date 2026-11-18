#include "sky.h"
#include "platform.h"
#include <array>
#include <stdexcept>
#include <glm/gtc/matrix_inverse.hpp>

sky_background::sky_background() {
	program_ = std::make_shared<program>("sky", "shaders/sky.vert", "shaders/sky.frag");
	image_ = std::make_shared<texture>(resource_path("assets/sky.png"));
	image_->upload();
	image_->activate();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glGenVertexArrays(1, &vao_);
}

sky_background::~sky_background() { if (vao_) glDeleteVertexArrays(1, &vao_); }

void sky_background::draw(const glm::dmat4& view, const glm::mat4& projection) {
	GLboolean depth = glIsEnabled(GL_DEPTH_TEST), cull = glIsEnabled(GL_CULL_FACE), blend = glIsEnabled(GL_BLEND), depth_write;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_write);
	GLint previous_vao, previous_program;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_vao);
	glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	program_->use();
	program_->setuniform("inverse_projection", glm::inverse(projection));
	// Only rotation affects the distant sky. Moving toward Earth cannot move it.
	program_->setuniform("inverse_view", glm::mat4(glm::transpose(glm::dmat3(view))));
	program_->setuniform("sky_image", 0);
	image_->activate();
	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(previous_vao);
	glUseProgram(previous_program);
	glDepthMask(depth_write);
	if (depth) glEnable(GL_DEPTH_TEST);
	if (cull) glEnable(GL_CULL_FACE);
	if (blend) glEnable(GL_BLEND);
}
