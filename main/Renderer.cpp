#include "Renderer.hpp"

#include <glm\mat4x4.hpp>
#include <array>
#include "Utils.hpp"

bool Renderer::init(){
	close();

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		return false;

	_window = SDL_CreateWindow(_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _size.x, _size.y, SDL_WINDOW_OPENGL);

	if (!_window){
		close();
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	_context = SDL_GL_CreateContext(_window);

	if (!_context){
		close();
		return false;
	}

	_reshape();

	_running = true;
	return true;
}

void Renderer::close(){
	if (_running){
		SDL_GL_DeleteContext(_context);
		SDL_DestroyWindow(_window);

		SDL_Quit();

		_running = false;
	}
}

Renderer& Renderer::get(){
	static Renderer instance;
	return instance;
}

void Renderer::_flip(){
	SDL_GL_SwapWindow(_window);
	glClearColor(0.5f, 0.5f, 0.5f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::_reshape(){
	glViewport(0, 0, _size.x, _size.y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, _size.x * _zoom, 0, _size.y * _zoom, -_draw, _draw);

	glTranslatef((_size.x / 2) * _zoom, (_size.y / 2) * _zoom, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glm::quat rotation = glm::quat(glm::vec3(0, 0, glm::eulerAngles(_rotation).z));

	glMultMatrixf(&glm::mat4_cast(rotation)[0][0]);

	glTranslatef(_position.x, _position.y, 0);
}


void Renderer::_drawLines(){
	// Draw track sensors
	for (Line line : _lineBuffer){
		glBegin(GL_LINES);

		glColor3fv(&line.colour[0]);

		glVertex2fv(&(line.start)[0]);
		glVertex2fv(&(line.end)[0]);

		glEnd();
	}

	_lineBuffer.clear();
}

void Renderer::_drawPoints(){
	// Draw opponent sensors
	for (Point point : _pointBuffer){
		glPointSize(point.size);

		glBegin(GL_POINTS);

		glColor3fv(&point.colour[0]);

		glVertex2fv(&(point.point)[0]);

		glEnd();
	}

	_pointBuffer.clear();
}

void Renderer::_drawGraphs(){
	glm::vec2 corner = { -_size.x / 2.f, _size.y / 2.f };
	glm::vec2 padding = { 16.f, -16.f };

	float height = 64.f;

	glm::vec2 start = corner + padding + glm::vec2(0, -height / 2.f);
	glm::vec2 end = start + glm::vec2(_size.x - padding.x * 2.f, 0.f);

	glm::vec2 bottom = corner + padding;
	glm::vec2 top = bottom + glm::vec2(0, -height);

	// Drawing graph axes
	glColor3f(0.75f, 0.75f, 0.75f);

	glBegin(GL_LINES);

	glVertex2fv(&(top - glm::vec2(0.f, 1.f))[0]);
	glVertex2fv(&bottom[0]);
	
	glVertex2fv(&start[0]);
	glVertex2fv(&end[0]);

	glVertex2f(end.x, top.y - 1.f);
	glVertex2f(end.x, bottom.y);

	glEnd();

	for (unsigned int i = 0; i < 8; i++){
		if (!_graphs[i].active)
			continue;

		if (!_graphs[i].points.size())
			continue;

		// Changing offset stepping layers
		if (!_graphs[i].scrolling){
			glm::vec2 last = *(_graphs[i].points.end() - 1);
			
			if (last.x > _graphs[i].xLength + _graphs[i].xOffset){
				_graphs[i].xOffset += last.x - _graphs[i].xOffset;
			
				_graphs[i].points.clear();
				_graphs[i].points.push_back(last);
			}
		}
		
		// Changing offset for scrolling layers
		else if (_graphs[i].scrolling){
			if (_graphs[i].points.size() < 2)
				continue;

			glm::vec2 last = *(_graphs[i].points.end() - 1);
			glm::vec2 prev = *(_graphs[i].points.end() - 2);

			float lastX = changeRange(0, _graphs[i].xLength, start.x, end.x, last.x - _graphs[i].xOffset);

			while (lastX > end.x){
				_graphs[i].xOffset += last.x - prev.x;

				lastX = changeRange(0, _graphs[i].xLength, start.x, end.x, last.x - _graphs[i].xOffset);
			}

			glm::vec2 first = *(_graphs[i].points.begin());

			float firstX = changeRange(0, _graphs[i].xLength, start.x, end.x, first.x - _graphs[i].xOffset);

			while (firstX < start.x){
				_graphs[i].points.erase(_graphs[i].points.begin());

				first = *(_graphs[i].points.begin());
				firstX = changeRange(0, _graphs[i].xLength, start.x, end.x, first.x - _graphs[i].xOffset);
			}
		}

		// Drawing lines
		glColor3fv(&_layerColours[i][0]);

		glBegin(GL_LINE_STRIP);

		for (glm::vec2 point : _graphs[i].points){
			float x = changeRange(0, _graphs[i].xLength, start.x, end.x, point.x - _graphs[i].xOffset);
			float y = changeRange(_graphs[i].yMin, _graphs[i].yMax, top.y, bottom.y, point.y);

			glVertex2f(x, y);
		}

		glEnd();
	}
}

void Renderer::update(){
	SDL_Event event;
	while (SDL_PollEvent(&event)){
		if (event.type == SDL_QUIT){
			close();
			return;
		}
	}

	_flip();
	_reshape();

	_mutex.lock();

	SDL_SetWindowTitle(_window, _title.c_str());
	
	_drawLines();
	_drawPoints();

	_drawGraphs();
	
	_mutex.unlock();
}

void Renderer::setPosition(const glm::vec2& position){
	_position = position;
}

void Renderer::setRotation(float rotation){
	_rotation = glm::quat(glm::vec3(0, 0, glm::radians(rotation)));
}

void Renderer::setZoom(float zoom){
	_zoom = zoom;
}

void Renderer::setWindowTitle(const std::string& title){
	_title = title;
}

void Renderer::setWindowSize(const glm::vec2& size){
	_size = size;

	if (_running)
		init();
}

bool Renderer::running(){
	return _running;
}

void Renderer::drawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& colour){
	_mutex.lock();

	//if (_running)
		_lineBuffer.push_back(Line(start, end, colour));

	_mutex.unlock();
}

void Renderer::drawPoint(const glm::vec2& point, const glm::vec3& colour){
	_mutex.lock();

	//if (_running)
		_pointBuffer.push_back(Point(point, 5.f, colour));

	_mutex.unlock();
}


void Renderer::setGraph(unsigned int layer, float xLength, float yMin, float yMax, bool scrolling){
	_mutex.lock();

	//if (_running){

		if (layer >= 8)
			return;

		_graphs[layer].points.clear();

		_graphs[layer].active = true;

		_graphs[layer].xLength = xLength;
		_graphs[layer].yMin = yMin;
		_graphs[layer].yMax = yMax;

		_graphs[layer].scrolling = scrolling;

	//}

	_mutex.unlock();
}

void Renderer::drawGraph(const glm::vec2& point, unsigned int layer){
	_mutex.lock();

	//if (_running){

		if (layer >= 8)
			return;

		_graphs[layer].points.push_back(point);

	//}

	_mutex.unlock();
}