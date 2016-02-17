#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <string>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include <glm\gtc\quaternion.hpp>
#include <vector>
#include <mutex>

class Renderer{
	std::string _title = "SDL";
	glm::vec2 _size = { 640, 480 };

	float _draw = 1024.f;
	float _zoom = 1.f;

	glm::vec2 _position;
	glm::quat _rotation;

	SDL_Window* _window = nullptr;
	SDL_GLContext _context = 0;

	std::vector<glm::vec4> _lineBuffer;

	bool _running = false;

	std::mutex _mutex;

	void _flip();
	void _reshape();

public:
	bool init();
	void close();

	static Renderer& get();

	void update();

	void setPosition(const glm::vec2& position);
	void setRotation(float rotation);
	void setZoom(float zoom);

	void setWindowTitle(const std::string& title);
	void setWindowSize(const glm::vec2& size);

	bool running();

	void drawLine(const glm::vec2& start, const glm::vec2& end);
	void drawPoint(const glm::vec2& point);
};