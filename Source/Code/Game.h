#pragma once

#include "RenderEngine.h"
#include "FileSystem/FileSystem.h"
#include "ResourceManager.h"
#include "Input/InputHandler.h"
#include "ScriptSystem/ScriptSystem.h"
#include "EntityManager.h"
#include "GameTimer.h"
#include "flecs.h"
#include "LoadingSystem/LoadingSystem.h"
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/async_scope.hpp>

class Game
{
public:
	Game();
	~Game();
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;

	void Run();
	bool Update();
	bool GetQuit();
	void Quit();
	std::unordered_map<uint32_t, Entity> GetEntities();
	RenderEngine* GetRE();
	void UpdatePhysics(bool flag);
	cppcoro::task<> Save(std::string path, std::unordered_map<uint32_t, Entity> ent_queue);
	cppcoro::task<> SaveBits(std::string path, std::unordered_map<uint32_t, Entity> ent_queue);
	cppcoro::task<> LoadBits(std::string path);

	std::filesystem::path GetSavesRoot();

private:
	GameTimer m_Timer;
	flecs::world* m_pEcs;
	std::atomic<bool> physics_fl = true;
	std::atomic<bool> clear_enities_fl = false;
	std::atomic<bool> clear_enities_end_fl = false;
	std::atomic<bool> world_recreation_fl = false;
	std::atomic<bool> world_recreation_end_fl = false;
	//cppcoro::async_scope m_Scope;

	RenderEngine* m_pRenderEngine;
	FileSystem* m_pFileSystem;
	ResourceManager* m_pResourceManager;
	InputHandler* m_pInputHandler;
	ScriptSystem* m_pScriptSystem;
	EntityManager* m_pEntityManager;
	LoadingSystem* m_pLoadingSystem;
	cppcoro::static_thread_pool m_ThreadPool;

	void recreate_world();
	cppcoro::task<> process_events(cppcoro::io_service& ioService);
};

