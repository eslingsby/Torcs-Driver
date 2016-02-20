#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <string>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include <vector>
#include <array>
#include <mutex>

struct Line{
	const glm::vec2 start;
	const glm::vec2 end;
	const glm::vec3 colour;

	Line(const glm::vec2& start, const glm::vec2& end, const glm::vec3& colour) :
		start(start),
		end(end),
		colour(colour){
	}
};

struct Point{
	const glm::vec2 point;
	const float size;
	const glm::vec3 colour;

	Point(const glm::vec2& point, float size, const glm::vec3& colour) :
		point(point),
		size(size),
		colour(colour){
	}
};

struct Graph{
	bool active = false;

	float xLength = 1000.f;
	float xOffset = 0.f;

	float yMin = -1.f;
	float yMax = 1.f;

	bool scrolling = false;

	std::vector<const glm::vec2> points;
};

class Renderer{
	std::string _title = "SDL";
	glm::vec2 _size = { 640, 480 };

	float _draw = 1024.f;
	float _zoom = 1.f;

	glm::vec2 _position;
	glm::quat _rotation;

	SDL_Window* _window = nullptr;
	SDL_GLContext _context = 0;

	bool _running = false;

	std::mutex _mutex;

	std::vector<Line> _lineBuffer;
	std::vector<Point> _pointBuffer;

	std::array<Graph, 8> _graphs;

	std::array<glm::vec3, 8> _layerColours = { {

		glm::vec3(0.f, 1.f, 1.f),// 011 C
		glm::vec3(1.f, 0.f, 1.f),// 101 M
		glm::vec3(1.f, 1.f, 0.f),// 110 Y
		glm::vec3(1.f, 1.f, 1.f),// 111 K

		glm::vec3(1.f, 0.f, 0.f),// 100 R
		glm::vec3(0.f, 1.f, 0.f),// 010 G
		glm::vec3(0.f, 0.f, 1.f),// 001 B
		glm::vec3(0.f, 0.f, 0.f) // 000 W
		
	} };

	void _drawLines();
	void _drawPoints();
	void _drawGraphs();

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

	void drawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& colour = { 1.f, 1.f, 1.f });
	void drawPoint(const glm::vec2& point, const glm::vec3& colour = { 1.f, 1.f, 1.f });

	void setGraph(unsigned int layer, float xLength, float yMin, float yMax, bool scrolling = false);
	void drawGraph(const glm::vec2& point, unsigned int layer);
};