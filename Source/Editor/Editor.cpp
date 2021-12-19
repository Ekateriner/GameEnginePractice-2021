#include "Editor.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <SDL_syswm.h>

Editor::Editor() {
	SDL_Init(SDL_INIT_VIDEO);

	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	m_SDL_Window_GUI = SDL_CreateWindow("Instruments", 0, 20, 566, 748, window_flags);

	m_GL_Context_GUI = SDL_GL_CreateContext(m_SDL_Window_GUI);
	SDL_GL_MakeCurrent(m_SDL_Window_GUI, m_GL_Context_GUI);
	SDL_GL_SetSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	ImGui_ImplSDL2_InitForOpenGL(m_SDL_Window_GUI, m_GL_Context_GUI);
	bool f = ImGui_ImplOpenGL3_Init(glsl_version);

	m_pGame = new Game();
	m_pGameThread = nullptr;
}

Editor::~Editor() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(m_GL_Context_GUI);
	SDL_DestroyWindow(m_SDL_Window_GUI);
	SDL_Quit();
}

bool Editor::Update() {
	m_GL_Context_GUI = SDL_GL_CreateContext(m_SDL_Window_GUI);
	SDL_GL_MakeCurrent(m_SDL_Window_GUI, m_GL_Context_GUI);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	bool f = true;
	ImGui::ShowDemoWindow(&f);

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(m_SDL_Window_GUI);

	return !m_pGame->GetQuit();
}

void Editor::Run() {
	//m_pGameThread = std::unique_ptr<std::thread>(new std::thread(&Game::Run, m_pGame));
	while (true)
	{
		if (!Update())
			break;
	}
	//m_pGameThread->join();
}