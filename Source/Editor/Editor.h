#pragma once
#include "GL/gl3w.h"

#include "../Code/Game.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <cppcoro/async_scope.hpp>

class Editor {
public:
	Editor();
	~Editor();
	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;

	void Run();
	void Update();

private:
	struct Object {
		std::string name;
		//uint32_t index;

		float pos_x;
		float pos_y;
		float pos_z;

		float rotate_x;
		float rotate_y;
		float rotate_z;

		float scale_x;
		float scale_y;
		float scale_z;

		std::string script_file;
	};

	void ProcessInput();
	void ImGuiWindow();
	int ImGuiPositionEntry(Object& obj, int start = 0);
	int ImGuiScaleEntry(Object& obj, int start = 0);
	int ImGuiRotationEntry(Object& obj, int start = 0);
	void GetObjects();
	void UpdateObjects();

	struct {
		bool general_fl = true;

		struct {
			bool active = false;
			std::string file_path;

			bool error = false;
		} Open;

		struct {
			bool active = false;
			std::string file_path;

			bool error = false;
		} Save;

		struct {
			bool active = false;
			std::string file_path;

			bool error = false;
		} Add;

		struct {
			bool play = false;
			bool exp = false;
			std::string file_path = "initialScene.xml";

			bool error = false;
		} GameField;

		struct {
			bool active;
			std::unordered_map<uint32_t, Object> objects;
		} Objects;

	} ImGuiState;
	
	Game* m_pGame;
	std::unique_ptr<std::thread> m_pGameThread;
	cppcoro::async_scope* m_pScopeLoad;
	cppcoro::async_scope* m_pScopeSave;

	SDL_Window* m_SDL_Window_GUI;
	SDL_GLContext m_GL_Context_GUI;
	bool m_Quit = false;
};
