#pragma once

#include <string>
#include "Ogre.h"
#include <cppcoro/io_service.hpp>
#include <cppcoro/task.hpp>
#include <filesystem>
#include "../EntityManager.h"

class EntityManager;

class LoadingSystem
{
public:
	LoadingSystem(EntityManager* pEntityManager, const std::filesystem::path scriptsRoot);

	~LoadingSystem();
	cppcoro::task<> LoadFromXML(const std::filesystem::path fileName);
	cppcoro::task<> SaveToXML(const std::filesystem::path fileName, std::unordered_map<uint32_t, Entity> ent_queue);

	//cppcoro::task<> LoadFromJson(const std::filesystem::path fileName);
	//cppcoro::task<> SaveToJson(const std::filesystem::path fileName);

	cppcoro::task<> LoadFromBits(const std::filesystem::path fileName, cppcoro::io_service& ioService);
	cppcoro::task<> SaveToBits(const std::filesystem::path fileName, std::unordered_map<uint32_t, Entity> ent_queue, cppcoro::io_service& ioService);

private:

	EntityManager* m_pEntityManager;
	std::filesystem::path m_strSavesRootPath;

	Ogre::Vector3 ParsePosition(const char* strPosition);
	Ogre::Vector3 ParseScale(const char* strScale);
};

