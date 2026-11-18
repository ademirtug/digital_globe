#pragma once

#include <glad/gl.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

class program {
	GLuint id_{ 0 };
	glm::dmat4 view_{ 1.0 };
public:
	program(const std::string& name, const std::string& vertex_path, const std::string& fragment_path);
	~program();
	program(const program&) = delete;
	program& operator=(const program&) = delete;
	void use();
	void setuniform(const std::string& name, const glm::mat4& value);
	void setuniform(const std::string& name, const glm::dmat4& value);
	void set_model(const glm::mat4& transform, glm::dvec3 origin);
	void setuniform(const std::string& name, const glm::vec3& value);
	void setuniform(const std::string& name, const glm::vec2& value);
	void setuniform(const std::string& name, float value);
	void setuniform(const std::string& name, int value);
};

class texture {
	std::vector<unsigned char> pixels_;
	int width_{ 0 }, height_{ 0 };
	GLuint id_{ 0 };
	void decode(const std::vector<unsigned char>& data);
public:
	texture(const std::filesystem::path& path);
	texture(const std::shared_ptr<std::vector<unsigned char>>& data);
	~texture();
	texture(const texture&) = delete;
	texture& operator=(const texture&) = delete;
	void upload();
	void activate();
};

struct vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

class mesh {
public:
	virtual ~mesh() = default;
	std::string name;
	std::vector<vertex> vertices;
	std::vector<glm::dvec3> positions;
	glm::dvec3 origin{ 0.0 };
	std::vector<int> indices;
	size_t size_of_indices{ 0 };
	glm::vec3 specular{ 0.0f };
	float shininess{ 16.0f };
};

class model {
protected:
	GLuint vbo_{ 0 }, ebo_{ 0 };
public:
	GLuint vao{ 0 };
	std::string path_;
	std::shared_ptr<mesh> m;
	std::shared_ptr<texture> tex;
	std::shared_ptr<program> prg;
	glm::mat4 transform{ 1.0f };
	glm::vec2 texture_scale{ 1.0f }, texture_offset{ 0.0f };
	virtual ~model();
	virtual void upload();
	virtual void draw();
	void attach_program(const std::shared_ptr<program>& p) { prg = p; }
};

class texture_model : public model {};

class directional_light {
public:
	directional_light(glm::vec3 direction) : position(direction) {}
	glm::vec3 position;
	glm::vec3 ambient{ 0.2f }, diffuse{ 0.9f }, specular{ 0.0f };
};
