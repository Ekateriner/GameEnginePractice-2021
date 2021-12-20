#include "RenderEngine.h"

// Creating Critical section interface
//std::mutex RC_CriticalSection;
//#define LOADINGCOMMAND_CRITICAL_SECTION std::scoped_lock<std::mutex> criticalSection (RC_CriticalSection);

// Function to run render thread
static unsigned RunThisThread(void* thisPtr)
{
	RenderThread* const self = (RenderThread*)thisPtr;
	self->Run();

	return 0;
}

RenderThread::RenderThread(RenderEngine* pRenderEngine) :
	m_pRenderEngine(pRenderEngine),
	m_nRenderThreadId(0),
	//m_nCurrentFrame(0),
	//m_nFrameFill(1),
	//m_nFlush(0),
	m_pThread(nullptr)
{
	m_nMainThreadId = ::GetCurrentThreadId();
	m_nFlush.store(0);
	m_Commands.clear();
}

RenderThread::~RenderThread()
{

}

// Render Loop
void RenderThread::Run()
{
	m_nRenderThreadId = ::GetCurrentThreadId();

	while (true)
	{
		WaitForMainThreadSignal();

		RC_BeginFrame();

		ProcessCommands();

		SignalMainThread();

		m_pRenderEngine->Update();

		RC_EndFrame();

		if (m_pRenderEngine->GetQuit())
			break;
	}
}

void RenderThread::Start()
{
	if (!m_pThread)
	{
		m_pThread = std::unique_ptr<std::thread>(new std::thread(RunThisThread, this));
	}
}

bool RenderThread::IsRenderThread()
{
	return m_nRenderThreadId == ::GetCurrentThreadId();
}

// We have 1 queue. One we fill, another - execute
//void RenderThread::NextFrame()
//{
//	m_nCurrentFrame = (m_nCurrentFrame + 1) & 1;
//	m_nFrameFill = (m_nFrameFill + 1) & 1;
//}

bool RenderThread::CheckFlushCond()
{
	return m_nFlush.load();
}

// Signal main thread, that he can continue his work
void RenderThread::SignalMainThread()
{
	m_nFlush.store(0);
}

// Signal render thread, that he can continue his work
void RenderThread::SignalRenderThread()
{
	m_nFlush.store(1);
}

// Process commands that render thread received from main thread
void RenderThread::ProcessCommands()
{
	assert(IsRenderThread());

	if (!CheckFlushCond())
		return;

	for (auto command : m_Commands.get_queue()) {
		command();
	}
}

void RenderThread::RC_Init()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_Init();
		return;
	}

	m_Commands.add_command(std::bind(&RenderEngine::RT_Init, m_pRenderEngine));
}

void RenderThread::RC_SetupDefaultCamera()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_SetupDefaultCamera();
		return;
	}

	m_Commands.add_command(std::bind(&RenderEngine::RT_SetupDefaultCamera, m_pRenderEngine));
}

void RenderThread::RC_SetupDefaultCompositor()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_SetupDefaultCompositor();
		return;
	}

	m_Commands.add_command(std::bind(&RenderEngine::RT_SetupDefaultCompositor, m_pRenderEngine));
}

void RenderThread::RC_LoadDefaultResources()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_LoadDefaultResources();
		return;
	}

	m_Commands.add_command(std::bind(&RenderEngine::RT_LoadDefaultResources, m_pRenderEngine));
}

void RenderThread::RC_SetupDefaultLight()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_SetupDefaultLight();
		return;
	}

	m_Commands.add_command(std::bind(&RenderEngine::RT_SetupDefaultLight, m_pRenderEngine));
}

void RenderThread::RC_CreateSceneNode(RenderNode* pRenderNode)
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_CreateSceneNode(pRenderNode);
		return;
	}

	m_Commands.add_command(std::bind(&RenderEngine::RT_CreateSceneNode, m_pRenderEngine, pRenderNode));
}

//void RenderThread::RC_UpdateSceneNode(RenderNode* pRenderNode, Ogre::Vector3 position, Ogre::Vector3 scale)
//{
//	if (IsRenderThread())
//	{
//		m_pRenderEngine->RT_UpdateSceneNode(pRenderNode, position, scale);
//		return;
//	}
//
//	m_Commands.add_command(std::bind(&RenderEngine::RT_UpdateSceneNode, m_pRenderEngine, pRenderNode, position, scale));
//}


void RenderThread::RC_BeginFrame()
{

}

void RenderThread::RC_EndFrame()
{
	if (IsRenderThread())
		return;

	SyncMainWithRender();
}

void RenderThread::SyncMainWithRender()
{
	assert(!IsRenderThread());

	WaitForRenderThreadSignal();

	SignalRenderThread();
}

// Wait signal from main thread
void RenderThread::WaitForMainThreadSignal()
{
	assert(IsRenderThread());

	while (!m_nFlush.load() && !m_pRenderEngine->GetQuit())
	{
	}
}

// Wait signal from render thread
void RenderThread::WaitForRenderThreadSignal()
{
	assert(!IsRenderThread());

	while (m_nFlush.load() && !m_pRenderEngine->GetQuit())
	{
	}
}

