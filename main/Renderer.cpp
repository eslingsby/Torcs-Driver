#include "Renderer.hpp"

#include <glm\mat4x4.hpp>

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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
	SDL_SetWindowTitle(_window, title.c_str());
}

void Renderer::setWindowSize(const glm::vec2& size){
	_size = size;

	if (_running)
		init();
}

bool Renderer::running(){
	return _running;
}

void Renderer::drawLine(const glm::vec3& start, const glm::vec3& end){

}

void Renderer::drawPoint(const glm::vec3& point, float size){

}