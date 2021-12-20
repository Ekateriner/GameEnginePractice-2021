#include "Editor.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "../Code/EntityManager.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>

Editor::Editor():
	m_Quit(false)
{
	SDL_Init(SDL_INIT_VIDEO);
	gl3wInit();

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
	m_SDL_Window_GUI = SDL_CreateWindow("Instruments", 0, 28, 566, 740, window_flags);

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

void Editor::Update() {
	SDL_GL_MakeCurrent(m_SDL_Window_GUI, m_GL_Context_GUI);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	ProcessInput();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_SDL_Window_GUI);
	ImGui::NewFrame();

	ImGuiWindow();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
	SDL_GL_SwapWindow(m_SDL_Window_GUI);

	m_Quit = m_Quit || m_pGame->GetQuit();
}

void Editor::Run() {
	m_pGameThread = std::unique_ptr<std::thread>(new std::thread(&Game::Run, m_pGame));
	while (!m_Quit)
	{
		m_pGame->UpdatePhysics(ImGuiState.GameField.play);
		Update();
	}
	m_pGameThread->join();
}

void Editor::ProcessInput(){
	SDL_Event event;
	if (SDL_PollEvent(&event)) {
		if (event.window.windowID != SDL_GetWindowID(m_SDL_Window_GUI)) {
			SDL_PushEvent(&event);
		}
		else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
			m_Quit = true;
			m_pGame->Quit();
		}
		ImGui_ImplSDL2_ProcessEvent(&event);
	}
}

void Editor::ImGuiWindow() {

	//ImGui::ShowDemoWindow(&m_Quit);
	bool general_fl = true;
	ImGui::Begin("General", &general_fl, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O")) { 
				ImGuiState.Open.active = true;
			}
			if (ImGui::MenuItem("Save", "Ctrl+S")) { 
				ImGuiState.Save.active = true;
			}
			if (ImGui::MenuItem("Close", "Ctrl+W")) { m_Quit; }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Object"))
		{
			if (ImGui::MenuItem("Add..", "Ctrl+A")) {
				ImGuiState.Add.active = true;
			}
			if (ImGui::MenuItem("Objects window", "Ctrl+I")) {
				ImGuiState.Objects.active = true;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Game"))
		{
			if (!ImGuiState.GameField.play) {
				if (ImGui::MenuItem("Play", "Ctrl+P")) {
					ImGuiState.GameField.play = true;
				}
			}
			else {
				if (ImGui::MenuItem("Stop", "Ctrl+P")) {
					ImGuiState.GameField.play = false;
				}
			}
			if (ImGui::MenuItem("Export", "Ctrl+E")) {
				ImGuiState.GameField.exp = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	//// Edit a color (stored as ~4 floats)
	//ImGui::ColorEdit4("Color", my_color);

	//// Plot some values
	//const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
	//ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

	//// Display contents in a scrolling region
	//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
	//ImGui::BeginChild("Scrolling");
	//for (int n = 0; n < 50; n++)
	//	ImGui::Text("%04d: Some text", n);
	//ImGui::EndChild();
	ImGui::End();

	if (ImGuiState.Save.active) {
		ImGui::Begin("Save", &ImGuiState.Save.active, ImGuiWindowFlags_MenuBar);

		ImGuiState.Save.file_path.resize(100);
		ImGui::InputText("Path to file", ImGuiState.Save.file_path.data(), ImGuiState.Save.file_path.size());

		bool fl = false;
		if (ImGui::Button("Save")) {
			fl = ImGuiState.Save.file_path.empty();
		}
		if (!fl) {
			// Save
			ImGuiState.Save.active = false;
		}

		ImGui::End();
	}

	if (ImGuiState.Open.active) {
		ImGui::Begin("Open", &ImGuiState.Open.active, ImGuiWindowFlags_MenuBar);

		ImGuiState.Open.file_path.resize(100);
		ImGui::InputText("Path to file", ImGuiState.Open.file_path.data(), ImGuiState.Open.file_path.size());

		bool fl = false;
		if (ImGui::Button("Open")) {
			fl = ImGuiState.Open.file_path.empty();
		}
		if (!fl) {
			//Check Open and Open
			ImGuiState.Open.active = false;
		}

		ImGui::End();
	}

	if (ImGuiState.Add.active) {
		ImGui::Begin("Add", &ImGuiState.Add.active, ImGuiWindowFlags_MenuBar);

		ImGuiState.Add.file_path.resize(100);
		ImGui::InputText("Path to file", ImGuiState.Add.file_path.data(), ImGuiState.Add.file_path.size());

		bool fl = false;
		if (ImGui::Button("Add")) {
			fl = ImGuiState.Add.file_path.empty();
		}
		if (!fl) {
			//Check Open and Open
			ImGuiState.Add.active = false;
		}

		ImGui::End();
	}

	if (ImGuiState.GameField.exp) {
		ImGui::Begin("Export", &ImGuiState.GameField.exp, ImGuiWindowFlags_MenuBar);

		ImGuiState.GameField.file_path.resize(100);
		ImGui::InputText("Path to file", ImGuiState.GameField.file_path.data(), ImGuiState.GameField.file_path.size());

		bool fl = false;
		if (ImGui::Button("Add")) {
			fl = ImGuiState.GameField.file_path.empty();
		}
		if (!fl) {
			//Check Open and Open
			ImGuiState.GameField.exp = false;
		}

		ImGui::End();
	}

	//ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

	if (ImGuiState.Objects.active) {
		GetObjects();

		ImGui::Begin("Objects", &ImGuiState.Objects.active, ImGuiWindowFlags_MenuBar);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			// Iterate placeholder objects (all the same data)
			for (auto &[ind, obj] : ImGuiState.Objects.objects)
			{
			//	ShowPlaceholderObject("Object", obj_i);
				ImGui::PushID(ind);

			//	// Text and Tree nodes are less high than framed widgets, using AlignTextToFramePadding() we add vertical spacing to make the tree lines equal high.
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				bool node_open = ImGui::TreeNode("Object", "id = %u, %s", ind, obj.name.data());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("Object_%u Settings", ind);

				if (node_open)
				{
					int i = 0;
					i = ImGuiPositionEntry(obj, i);
					i = ImGuiScaleEntry(obj, i);
					//i = ImGuiRotationEntry(obj, i);
					ImGui::Text("Script file: %s", obj.script_file.data());

					ImGui::TreePop();
				}
			    ImGui::PopID();
				//ImGui::Separator();
			}
			
			ImGui::EndTable();
			if (!ImGuiState.GameField.play)
				UpdateObjects();
		}
		ImGui::PopStyleVar();

		ImGui::End();
	}
}

int Editor::ImGuiPositionEntry(Object& obj, int start) {
	int i = start;
	ImGui::PushID(i);
	
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
	ImGui::TreeNodeEx("Field", flags, "Position.x", i);

	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::DragFloat("##value", &obj.pos_x, 0.01f);
	ImGui::NextColumn();
	
	ImGui::PopID();
	i += 1;
	ImGui::PushID(i);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TreeNodeEx("Field", flags, "Position.y", i);

	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::DragFloat("##value", &obj.pos_y, 0.01f);
	ImGui::NextColumn();

	ImGui::PopID();
	i += 1;
	ImGui::PushID(i);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TreeNodeEx("Field", flags, "Position.z", i);

	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::DragFloat("##value", &obj.pos_z, 0.01f);
	ImGui::NextColumn();

	ImGui::PopID();
	i += 1;
	return i;
}

int Editor::ImGuiScaleEntry(Object& obj, int start) {
	int i = start;
	ImGui::PushID(i);
	
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
	ImGui::TreeNodeEx("Field", flags, "Scale.x", i);

	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::DragFloat("##value", &obj.scale_x, 0.01f);
	ImGui::NextColumn();

	ImGui::PopID();
	i += 1;
	ImGui::PushID(i);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TreeNodeEx("Field", flags, "Scale.y", i);

	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::DragFloat("##value", &obj.scale_y, 0.01f);
	ImGui::NextColumn();

	ImGui::PopID();
	i += 1;
	ImGui::PushID(i);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TreeNodeEx("Field", flags, "Scale.z", i);

	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::DragFloat("##value", &obj.scale_z, 0.01f);
	ImGui::NextColumn();

	ImGui::PopID();
	i += 1;
	return i;
}

int Editor::ImGuiRotationEntry(Object& obj, int start) {
	int i = start;
	ImGui::PushID(i);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
	ImGui::TreeNodeEx("Field", flags, "Rotation.x", i);

	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::DragFloat("##value", &obj.rotate_x, 0.01f);
	ImGui::NextColumn();

	ImGui::PopID();
	i += 1;
	ImGui::PushID(i);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TreeNodeEx("Field", flags, "Rotation.y", i);

	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::DragFloat("##value", &obj.rotate_y, 0.01f);
	ImGui::NextColumn();

	ImGui::PopID();
	i += 1;
	ImGui::PushID(i);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TreeNodeEx("Field", flags, "Rotation.z", i);

	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::DragFloat("##value", &obj.rotate_z, 0.01f);
	ImGui::NextColumn();

	ImGui::PopID();
	i += 1;

	if (ImGui::Button("Apply rotation")) {
		//Applying
	}
	return i;
}

void Editor::GetObjects() {
	//ImGuiState.Objects.objects.clear();
	auto entities = m_pGame->GetEntities();

	for (const auto& [ind, entity] : entities) {
		if (ImGuiState.Objects.objects.find(ind) == ImGuiState.Objects.objects.end()) {
			Object new_obj;
			new_obj.name = entity.pRenderNode->GetMeshName().c_str();
			new_obj.pos_x = entity.pRenderNode->GetPosition().x;
			new_obj.pos_y = entity.pRenderNode->GetPosition().y;
			new_obj.pos_z = entity.pRenderNode->GetPosition().z;
			new_obj.scale_x = entity.pRenderNode->GetScale().x;
			new_obj.scale_y = entity.pRenderNode->GetScale().y;
			new_obj.scale_z = entity.pRenderNode->GetScale().z;
			new_obj.rotate_x = 0;
			new_obj.rotate_y = 0;
			new_obj.rotate_z = 0;
			new_obj.script_file = entity.pScriptNode->GetFilePath();
			
			ImGuiState.Objects.objects[ind] = new_obj;
		}
	}
	/*for (auto it = ImGuiState.Objects.objects.begin(); it != ImGuiState.Objects.objects.end();) {
		if (entities.find(it->first) == entities.end()) {
			it = ImGuiState.Objects.objects.erase(it);
		}
		else {
			it++;
		}
	}*/
}

void Editor::UpdateObjects() {
	auto entities = m_pGame->GetEntities();

	for (const auto& [ind, obj] : ImGuiState.Objects.objects) {
		if (entities.find(ind) != entities.end()) {
			m_pGame->GetRE()->AddUpdate(
				entities[ind].pRenderNode, 
				Ogre::Vector3(obj.pos_x, obj.pos_y, obj.pos_z), 
				Ogre::Vector3(obj.scale_x, obj.scale_y, obj.scale_z)
			);
		}
	}
}