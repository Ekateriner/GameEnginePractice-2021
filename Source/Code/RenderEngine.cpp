#include "RenderEngine.h"
#include "ProjectDefines.h"

#include <SDL_syswm.h>

RenderEngine::RenderEngine(ResourceManager* pResourceManager):
	m_pRoot(nullptr),
	m_pRenderWindow(nullptr),
	m_pSceneManager(nullptr),
	m_pD3D11Plugin(nullptr),
	//m_pGL3PlusPlugin(nullptr),
	m_pCamera(nullptr),
	m_pWorkspace(nullptr),
	m_pRT(nullptr),
	m_pResourceManager(pResourceManager)
//	m_UseGUI(gui)
{
	// Creating window
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

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	m_SDL_Window = SDL_CreateWindow("Game", 566, 28, 800, 740, window_flags);

	SDL_GL_SetSwapInterval(1);

	m_pRT = new RenderThread(this);

	m_pRT->RC_Init();
	m_pRT->RC_SetupDefaultCamera();
	m_pRT->RC_SetupDefaultCompositor();
	m_pRT->RC_LoadDefaultResources();
	m_pRT->RC_SetupDefaultLight();

	m_pRT->Start();
	m_bQuit.store(false);
}

RenderEngine::~RenderEngine()
{
	SDL_GL_DeleteContext(m_GL_Context);
	SDL_DestroyWindow(m_SDL_Window);
	SDL_Quit();

	SAFE_OGRE_DELETE(m_pRoot);
}

bool RenderEngine::SetOgreConfig()
{
#ifdef _DEBUG
	constexpr bool bAlwaysShowConfigWindow = true;
	if (bAlwaysShowConfigWindow || !m_pRoot->restoreConfig())
#else
	if (!m_pRoot->restoreConfig())
#endif
	{
		if (!m_pRoot->showConfigDialog())
		{
			return false;
		}
	}

	return true;
}

void RenderEngine::Update()
{
	//m_GL_Context = SDL_GL_CreateContext(m_SDL_Window);
	SDL_GL_MakeCurrent(m_SDL_Window, m_GL_Context);
	Ogre::WindowEventUtilities::messagePump();

	for (auto up_info : UpdateQueue.get_queue()) {
		up_info.pRenderNode->SetPosition(up_info.new_position);
		up_info.pRenderNode->SetScale(up_info.new_scale);
	}

	for (RenderNode* pRenderNode : m_RenderNodes)
	{
		if (pRenderNode->GetStatic())
			continue;

		Ogre::Vector3 vPosition = pRenderNode->GetPosition();
		pRenderNode->GetSceneNode()->setPosition(vPosition);
		
		Ogre::Vector3 vScale = pRenderNode->GetScale();
		pRenderNode->GetSceneNode()->setScale(vScale);

		Ogre::Quaternion orientation = pRenderNode->GetOrientation();
		pRenderNode->GetSceneNode()->setOrientation(orientation);

		if (pRenderNode->IsCameraEnabled())
		{
			m_pCamera->setPosition(pRenderNode->GetCameraPosition());
			m_pCamera->lookAt(vPosition);
		}
	}

	if (m_pRenderWindow->isVisible())
		if (m_pRenderWindow->isClosed())
			m_bQuit.store(true);
		else 
			m_bQuit.store(m_bQuit.load() | !m_pRoot->renderOneFrame());

	/*if (m_bQuit)
		m_pRT->RC_End();*/
}

void RenderEngine::ProcessInput() {
	SDL_Event event;
	if (SDL_PollEvent(&event)) {
		if (event.window.windowID != SDL_GetWindowID(m_SDL_Window)) {
			SDL_PushEvent(&event);
		}
		else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
			m_bQuit.store(true);
		}
	}
}

void RenderEngine::RT_Init()
{
	m_pRoot = OGRE_NEW Ogre::Root();
	m_pD3D11Plugin = OGRE_NEW Ogre::D3D11Plugin();
	//m_pGL3PlusPlugin = OGRE_NEW Ogre::GL3PlusPlugin();

	m_pRoot->installPlugin(m_pD3D11Plugin);
	//m_pRoot->installPlugin(m_pGL3PlusPlugin);


	if (!SetOgreConfig())
	{
		m_bQuit.store(true);
		return;
	}

	m_pRoot->initialise(false);

	m_GL_Context = SDL_GL_CreateContext(m_SDL_Window);
	SDL_GL_MakeCurrent(m_SDL_Window, m_GL_Context);

	Ogre::uint32 width = 800;
	Ogre::uint32 height = 740;
	Ogre::String sTitleName = "Game Engine";
	Ogre::NameValuePairList params;

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(m_SDL_Window, &info);

	params["externalWindowHandle"] = Ogre::StringConverter::toString(reinterpret_cast<size_t>(info.info.win.window));
	params["externalGLContent"] = Ogre::StringConverter::toString(reinterpret_cast<size_t>(info.info.win.hdc));
	params["externalGLControl"] = Ogre::String("True");

	m_pRenderWindow = Ogre::Root::getSingleton().createRenderWindow(sTitleName, width, height, false, &params);

	// Scene manager
	m_pSceneManager = m_pRoot->createSceneManager(Ogre::SceneType::ST_GENERIC, 1);
}

void RenderEngine::RT_SetupDefaultCamera()
{
	m_pCamera = m_pSceneManager->createCamera("Main Camera");

	m_pCamera->setPosition(Ogre::Vector3(0, 10, 35));
	m_pCamera->lookAt(Ogre::Vector3(0, 0, 0));
	m_pCamera->setNearClipDistance(0.2f);
	m_pCamera->setFarClipDistance(1000.0f);
	m_pCamera->setAutoAspectRatio(true);
}

void RenderEngine::RT_SetupDefaultCompositor()
{
	Ogre::CompositorManager2* compositorManager = m_pRoot->getCompositorManager2();

	const Ogre::String workspaceName("WorkSpace");

	if (!compositorManager->hasWorkspaceDefinition(workspaceName))
	{
		compositorManager->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue::Blue);
	}

	m_pWorkspace = compositorManager->addWorkspace(m_pSceneManager, m_pRenderWindow->getTexture(), m_pCamera, workspaceName, true);
}

void RenderEngine::RT_LoadDefaultResources()
{
	m_pResourceManager->LoadOgreResources("resources.cfg");
}

void RenderEngine::RT_CreateSceneNode(RenderNode* pRenderNode)
{
	//Create an Item with the model we just imported.
	//Notice we use the name of the imported model. We could also use the overload
	//with the mesh pointer:
	Ogre::String strImportedMeshName = pRenderNode->GetMeshName() + "v1";
	if (!Ogre::MeshManager::getSingleton().resourceExists(strImportedMeshName))
		ImportV1Mesh(pRenderNode->GetMeshName());

	Ogre::Item* item = m_pSceneManager->createItem(strImportedMeshName,
		Ogre::ResourceGroupManager::
		AUTODETECT_RESOURCE_GROUP_NAME,
		Ogre::SCENE_DYNAMIC);
	Ogre::SceneNode* pSceneNode = m_pSceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->
		createChildSceneNode(Ogre::SCENE_DYNAMIC);
	pSceneNode->attachObject(item);
	//pSceneNode->scale(0.1f, 0.1f, 0.1f); // TODO: move out to ecs
	
	pRenderNode->SetSceneNode(pSceneNode);

	m_RenderNodes.push_back(pRenderNode);
}

//void RenderEngine::RT_UpdateSceneNode(RenderNode* pRenderNode, Ogre::Vector3 position, Ogre::Vector3 scale)
//{
//	pRenderNode->SetPosition(position);
//	pRenderNode->SetScale(scale);
//}

void RenderEngine::AddUpdate(RenderNode* pRenderNode, Ogre::Vector3 position, Ogre::Vector3 scale) {
	UpdateInfo up_info;
	up_info.pRenderNode = pRenderNode;
	up_info.new_position = position;
	up_info.new_scale = scale;
	UpdateQueue.add_command(up_info);
}

void RenderEngine::ImportV1Mesh(Ogre::String strMeshName)
{
	//Load the v1 mesh. Notice the v1 namespace
	//Also notice the HBU_STATIC flag; since the HBU_WRITE_ONLY
	//bit would prohibit us from reading the data for importing.
	Ogre::String strImportedMeshName = strMeshName + "v1";

	Ogre::v1::MeshPtr v1Mesh;
	Ogre::MeshPtr v2Mesh;

	v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
		strMeshName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
		Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);

	//Create a v2 mesh to import to, with a different name (arbitrary).
	v2Mesh = Ogre::MeshManager::getSingleton().createManual(
		strImportedMeshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	bool halfPosition = true;
	bool halfUVs = true;
	bool useQtangents = true;

	//Import the v1 mesh to v2
	v2Mesh->importV1(v1Mesh.get(), halfPosition, halfUVs, useQtangents);

	//We don't need the v1 mesh. Free CPU memory, get it out of the GPU.
	//Leave it loaded if you want to use athene with v1 Entity.
	v1Mesh->unload();
}

void RenderEngine::RT_SetupDefaultLight()
{
	// Directional lightning
	Ogre::Light* light = m_pSceneManager->createLight();
	Ogre::SceneNode* lightNode = m_pSceneManager->getRootSceneNode()->createChildSceneNode();
	lightNode->attachObject(light);
	light->setPowerScale(Ogre::Math::PI); //Since we don't do HDR, counter the PBS' division by PI
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());
}

void RenderEngine::Recreate() {
	m_RenderNodes.clear();
	m_pSceneManager->clearScene(true);

	auto cameraIter = m_pSceneManager->getCameraIterator();
	while (cameraIter.hasMoreElements()) {
		m_pCamera = cameraIter.getNext();
		m_pCamera->setPosition(Ogre::Vector3(0, 10, 35));
		m_pCamera->lookAt(Ogre::Vector3(0, 0, 0));
		m_pCamera->setNearClipDistance(0.2f);
		m_pCamera->setFarClipDistance(1000.0f);
		m_pCamera->setAutoAspectRatio(true);
	}
	RT_SetupDefaultLight();
}
