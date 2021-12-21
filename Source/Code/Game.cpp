#include "Game.h"
#include "ECS/ecsMesh.h"
#include "ECS/ecsSystems.h"
#include "ECS/ecsPhys.h"
#include "ECS/ecsControl.h"
#include "ECS/ecsLoad.h"
#include <stdlib.h>
#include <filesystem>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/io_service.hpp>
#include <cppcoro/when_all_ready.hpp>

Game::Game() :
	m_ThreadPool(4)
{
	//m_ThreadPool = cppcoro::static_thread_pool(4);

	m_pEcs = new flecs::world();
	m_pFileSystem = new FileSystem();
	m_pResourceManager = new ResourceManager(m_pFileSystem->GetMediaRoot());
	m_pInputHandler = new InputHandler(m_pFileSystem->GetMediaRoot());
	m_pRenderEngine = new RenderEngine(m_pResourceManager);
	m_pScriptSystem = new ScriptSystem(m_pInputHandler, m_pFileSystem->GetScriptsRoot());
	m_pEntityManager = new EntityManager(m_pRenderEngine, m_pScriptSystem, m_pEcs);
	m_pLoadingSystem = new LoadingSystem(m_pEntityManager, m_pFileSystem->GetSavesRoot());

	m_Timer.Start();

	m_pEcs->entity("inputHandler")
		.set(InputHandlerPtr{ m_pInputHandler });
	m_pEcs->entity("scriptSystem")
		.set(ScriptSystemPtr{ m_pScriptSystem });

	cppcoro::task<> load_task = cppcoro::schedule_on(m_ThreadPool, std::move(m_pLoadingSystem->LoadFromXML("initialScene.xml")));
	cppcoro::sync_wait(std::move(load_task));
	
	register_ecs_mesh_systems(m_pEcs);
	register_ecs_control_systems(m_pEcs);
	register_ecs_phys_systems(m_pEcs);
	register_ecs_script_systems(m_pEcs);
	register_ecs_static_systems(m_pEcs);

	m_pEcs->progress();
	//m_pEcs->progress();

	register_ecs_load_systems(m_pEcs);
}

Game::~Game()
{
	SAFE_DELETE(m_pEcs);
	SAFE_DELETE(m_pFileSystem);
	SAFE_DELETE(m_pResourceManager);
	SAFE_DELETE(m_pInputHandler);
	SAFE_DELETE(m_pRenderEngine);
	SAFE_DELETE(m_pScriptSystem);
	SAFE_DELETE(m_pEntityManager);
	SAFE_DELETE(m_pLoadingSystem);
}

void Game::Run()
{
	m_Timer.Reset();

	while (true)
	{
		m_pRenderEngine->GetRT()->RC_BeginFrame();

		m_Timer.Tick();

		if (m_pInputHandler)
			m_pInputHandler->Update();

		if (!Update())
			break;

		m_pRenderEngine->GetRT()->RC_EndFrame();

		if (clear_enities_fl.load()) {
			recreate_world();

			clear_enities_end_fl.store(true);
			clear_enities_fl.store(false);

			while (!world_recreation_fl.load()) {
			}
			register_ecs_mesh_systems(m_pEcs);
			register_ecs_control_systems(m_pEcs);
			register_ecs_phys_systems(m_pEcs);
			register_ecs_script_systems(m_pEcs);
			register_ecs_static_systems(m_pEcs);

			m_pEcs->progress();
			register_ecs_load_systems(m_pEcs);

			world_recreation_end_fl.store(true);
			world_recreation_fl.store(false);
		}
		m_pRenderEngine->GetRT()->SignalRenderThread();
	}
}

bool Game::Update()
{
	if (physics_fl.load())
		m_pEcs->progress();
	return !m_pRenderEngine->GetQuit();
}

bool Game::GetQuit()
{
	return m_pRenderEngine->GetQuit();
}

void Game::Quit()
{
	m_pRenderEngine->SetQuit(true);
}

std::unordered_map<uint32_t, Entity> Game::GetEntities() {
	return m_pEntityManager->GetEntityQueue();
}

RenderEngine* Game::GetRE() {
	return m_pRenderEngine;
}

void Game::UpdatePhysics(bool flag) {
	physics_fl.store(flag);
}

cppcoro::task<> Game::Save(std::string path, std::unordered_map<uint32_t, Entity> ent_queue) {
	co_await cppcoro::schedule_on(m_ThreadPool, std::move(m_pLoadingSystem->SaveToXML(std::filesystem::path(path), ent_queue, m_pFileSystem->GetScriptsRoot())));
	co_return;
}

cppcoro::task<> Game::SaveBits(std::string path, std::unordered_map<uint32_t, Entity> ent_queue) {
	cppcoro::io_service ioService;
	co_await cppcoro::schedule_on(m_ThreadPool, cppcoro::when_all_ready(
		std::move(m_pLoadingSystem->SaveToBits(std::filesystem::path(path), ent_queue, ioService, m_pFileSystem->GetScriptsRoot())),
		process_events(ioService)));
	co_return ;
}

cppcoro::task<> Game::LoadBits(std::string path) {
	clear_enities_fl.store(true);
	while (!clear_enities_end_fl.load());
	clear_enities_end_fl.store(false);
	
	cppcoro::io_service ioService;
	co_await cppcoro::schedule_on(m_ThreadPool, cppcoro::when_all_ready(
		std::move(m_pLoadingSystem->LoadFromBits(std::filesystem::path(path), ioService)), 
		process_events(ioService)));
	
	world_recreation_fl.store(true);
	while (!world_recreation_end_fl.load());
	world_recreation_end_fl.store(true);
	co_return;
}

std::filesystem::path Game::GetSavesRoot() {
	return std::filesystem::path(m_pFileSystem->GetSavesRoot());
}

void Game::recreate_world() {
	m_pRenderEngine->Recreate();

	SAFE_DELETE(m_pLoadingSystem);
	SAFE_DELETE(m_pEntityManager);
	SAFE_DELETE(m_pEcs);

	m_pEcs = new flecs::world();
	m_pEntityManager = new EntityManager(m_pRenderEngine, m_pScriptSystem, m_pEcs);
	m_pLoadingSystem = new LoadingSystem(m_pEntityManager, m_pFileSystem->GetSavesRoot());

	m_Timer.Start();

	m_pEcs->entity("inputHandler")
		.set(InputHandlerPtr{ m_pInputHandler });
	m_pEcs->entity("scriptSystem")
		.set(ScriptSystemPtr{ m_pScriptSystem });

}

cppcoro::task<> Game::process_events(cppcoro::io_service & ioService) {
	ioService.process_events();
	co_return;
}