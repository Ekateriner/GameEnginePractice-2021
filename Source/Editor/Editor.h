#pragma once
#include "../Code/Game.h"

#include <SDL.h>
#include <SDL_opengl.h>

class Editor {
public:
	Editor();
	~Editor();
	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;

	void Run();
	bool Update();

private:
	Game* m_pGame;
	std::unique_ptr<std::thread> m_pGameThread;

	SDL_Window* m_SDL_Window_GUI;
	SDL_GLContext m_GL_Context_GUI;
};
