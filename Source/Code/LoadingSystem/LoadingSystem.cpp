#include "LoadingSystem.h"
#include "OgreSceneManager.h"
#include "tinyxml.h"
//#include "json.hpp"
#include <cppcoro/read_only_file.hpp>
#include <cppcoro/write_only_file.hpp>
#include <regex>
#include <iostream>
//#include <format>

LoadingSystem::LoadingSystem(EntityManager* pEntityManager, const std::filesystem::path scriptsRoot) :
	m_pEntityManager(pEntityManager),
	m_strSavesRootPath(scriptsRoot)
{

}

cppcoro::task<> LoadingSystem::LoadFromXML(const std::filesystem::path fileName)
{
	const auto pathName = m_strSavesRootPath / fileName;
	TiXmlDocument doc(pathName.u8string().c_str());

	if (doc.LoadFile())
	{
		//add check format
		const auto elem = doc.FirstChildElement("scene");
		for (TiXmlElement* e = elem->FirstChildElement("character"); e != nullptr; e = e->NextSiblingElement("character"))
		{
			EntityInfo currentCharacter;
			currentCharacter.meshName = e->Attribute("meshName");
			currentCharacter.scriptName = e->Attribute("scriptName");
			currentCharacter.position = ParsePosition(e->Attribute("position"));
			currentCharacter.scale = ParseScale(e->Attribute("scale"));

			m_pEntityManager->CreateEntity(currentCharacter);
		}
	}

	co_return;
}

cppcoro::task<> LoadingSystem::SaveToXML(const std::filesystem::path fileName, std::unordered_map<uint32_t, Entity> ent_queue)
{
	const auto pathName = m_strSavesRootPath / fileName;
	std::filesystem::remove(pathName);
	TiXmlDocument doc(pathName.u8string().c_str());
	const auto elem = doc.FirstChildElement("scene");
	auto entityQueue = ent_queue;

	for (auto& entity : entityQueue)
	{
		for (TiXmlElement* e = elem->FirstChildElement("character"); e != NULL; e = e->NextSiblingElement("character"))
		{
			TiXmlElement* meshElement = e->FirstChildElement("meshName");
			meshElement->SetAttribute("meshName", entity.second.pScriptNode->GetMeshName().c_str());

			TiXmlElement* scriptElement = e->FirstChildElement("scriptName");
			scriptElement->SetAttribute("scriptName", entity.second.pScriptNode->GetFilePath().c_str());

			TiXmlElement* posElement = e->FirstChildElement("position");
			auto pos = entity.second.pRenderNode->GetPosition();
			char str_pos[35];
			posElement->SetAttribute("position", sprintf(str_pos, "%f,%f,%f", pos.x, pos.y, pos.z));

			TiXmlElement* scaleElement = e->FirstChildElement("scale");
			auto scale = entity.second.pRenderNode->GetScale();
			char str_scale[35];
			scaleElement->SetAttribute("scale", sprintf(str_scale, "%f,%f,%f", scale.x, scale.y, scale.z));
		}
	}

	doc.SaveFile();

	co_return;
}

//cppcoro::task<> LoadingSystem::LoadFromJson(const std::filesystem::path fileName)
//{
//	const auto pathName = m_strSavesRootPath.append(fileName);
//	auto file = cppcoro::read_only_file::open(m_ioService, pathName.u8string());
//
//
//}
//
//cppcoro::task<> LoadingSystem::SaveToJson(const std::filesystem::path fileName)
//{
//	const auto pathName = m_strSavesRootPath.append(fileName);
//	auto file = cppcoro::write_only_file::open(m_ioService, pathName.u8string());
//}

cppcoro::task<> LoadingSystem::LoadFromBits(const std::filesystem::path fileName, cppcoro::io_service& ioService)
{
	cppcoro::io_work_scope ioScope(ioService);
	const auto pathName = m_strSavesRootPath / fileName;
	auto file = cppcoro::read_only_file::open(ioService, pathName.u8string());

	uint32_t entities_count;
	auto bytes_read = co_await file.read(0, &entities_count, sizeof(uint32_t));
	
	for (uint32_t i = 0; i < entities_count; i++) {
		EntityInfo entity_info;
		uint32_t str_size;
		bytes_read += co_await file.read(bytes_read, &str_size, sizeof(uint32_t));
		entity_info.meshName.resize(str_size);
		bytes_read += co_await file.read(bytes_read, entity_info.meshName.data(), entity_info.meshName.size());

		bytes_read += co_await file.read(bytes_read, &str_size, sizeof(uint32_t));
		entity_info.scriptName.resize(str_size);
		bytes_read += co_await file.read(bytes_read, entity_info.scriptName.data(), entity_info.scriptName.size());

		bytes_read += co_await file.read(bytes_read, &entity_info.position, sizeof(Ogre::Vector3));
		bytes_read += co_await file.read(bytes_read, &entity_info.scale, sizeof(Ogre::Vector3));
		bytes_read += co_await file.read(bytes_read, &entity_info.rotation, sizeof(Ogre::Quaternion));

		m_pEntityManager->CreateEntity(entity_info);
	}

	co_return;
}

cppcoro::task<> LoadingSystem::SaveToBits(const std::filesystem::path fileName, std::unordered_map<uint32_t, Entity> ent_queue, cppcoro::io_service& ioService)
{
	cppcoro::io_work_scope ioScope(ioService);
	const auto pathName = m_strSavesRootPath / fileName;
	std::filesystem::remove(pathName);
	auto file = cppcoro::write_only_file::open(ioService, pathName.u8string());

	uint32_t entities_count = ent_queue.size();
	auto bytes_write = co_await file.write(0, &entities_count, sizeof(uint32_t));
	for (auto& [ind, entity] : ent_queue) {
		uint32_t str_size = entity.pRenderNode->GetMeshName().size();
		bytes_write += co_await file.write(bytes_write, &str_size, sizeof(uint32_t));
		bytes_write += co_await file.write(bytes_write, entity.pRenderNode->GetMeshName().data(), str_size);

		str_size = entity.pScriptNode->GetFilePath().size();
		bytes_write += co_await file.write(bytes_write, &str_size, sizeof(uint32_t));
		bytes_write += co_await file.write(bytes_write, entity.pScriptNode->GetFilePath().data(), str_size);

		auto pos = entity.pRenderNode->GetPosition();
		bytes_write += co_await file.write(bytes_write, &pos, sizeof(Ogre::Vector3));
		auto scale = entity.pRenderNode->GetScale();
		bytes_write += co_await file.write(bytes_write, &scale, sizeof(Ogre::Vector3));
		auto orient = entity.pRenderNode->GetOrientation();
		bytes_write += co_await file.write(bytes_write, &orient, sizeof(Ogre::Quaternion));
	}

	co_return;
}

LoadingSystem::~LoadingSystem()
{

};

Ogre::Vector3 LoadingSystem::ParsePosition(const char* strPosition)
{
	std::regex regex("[+-]?([0-9]*[.])?[0-9]+");
	std::cmatch match;

	Ogre::Vector3 vPosition;

	std::regex_search(strPosition, match, regex);
	vPosition.x = std::stof(match[0]);

	strPosition = match.suffix().first;
	std::regex_search(strPosition, match, regex);
	vPosition.y = std::stof(match[0]);

	strPosition = match.suffix().first;
	std::regex_search(strPosition, match, regex);
	vPosition.z = std::stof(match[0]);

	return vPosition;
}

Ogre::Vector3 LoadingSystem::ParseScale(const char* strScale)
{
	return ParsePosition(strScale);
}