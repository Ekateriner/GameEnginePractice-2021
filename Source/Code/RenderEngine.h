#pragma once

#include "Ogre.h"
#include "OgreRoot.h"
#include "OgreWindow.h"
#include "OgreItem.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "Compositor/OgreCompositorManager2.h"

#include "RenderSystems/Direct3D11/OgreD3D11Plugin.h"
//#include "RenderSystems/GL3Plus/OgreGL3PlusPlugin.h"

#include "RenderThread.h"
#include "RenderNode.h"
#include "ResourceManager.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <cppcoro/static_thread_pool.hpp>

class RenderEngine
{
	friend class RenderThread;

public:
	//const bool m_UseGUI;

	RenderEngine(ResourceManager* pResourceManager);
	~RenderEngine();
	RenderEngine(const RenderEngine&) = delete;
	RenderEngine& operator=(const RenderEngine&) = delete;

	void Update();

	bool GetQuit() { return m_bQuit; }
	void SetQuit(bool bQuit) { m_bQuit = bQuit; }

	RenderThread* GetRT() const { return m_pRT; }

private:
	bool SetOgreConfig();

	void RT_Init();
	void RT_SetupDefaultCamera();
	void RT_SetupDefaultCompositor();
	void RT_LoadDefaultResources();
	void RT_SetupDefaultLight();
	void RT_CreateSceneNode(RenderNode* pRenderNode);

	void ImportV1Mesh(Ogre::String strMeshName);

	Ogre::Root* m_pRoot;
	Ogre::Window* m_pRenderWindow;
	Ogre::SceneManager* m_pSceneManager;
	Ogre::Camera* m_pCamera;
	Ogre::CompositorWorkspace* m_pWorkspace;
	
	Ogre::D3D11Plugin* m_pD3D11Plugin;
	//Ogre::GL3PlusPlugin* m_pGL3PlusPlugin;

	RenderThread* m_pRT;
	ResourceManager* m_pResourceManager;
	cppcoro::static_thread_pool* m_pJobSystem;

	std::vector<RenderNode*> m_RenderNodes;
	SDL_Window* m_SDL_Window;
	SDL_GLContext m_GL_Context;

	bool m_bQuit;
};

