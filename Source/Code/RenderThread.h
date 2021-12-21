#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <functional>

#include "ProjectDefines.h"
#include "MTQueue.hpp"


class RenderEngine;
class RenderNode;

//enum RenderCommand : UINT32
//{
//	eRC_Unknown = 0,
//	eRC_Init,
//	eRC_SetupDefaultCamera,
//	eRC_SetupDefaultCompositor,
//	eRC_LoadDefaultResources,
//	eRC_SetupDefaultLight,
//	eRC_BeginFrame,
//	eRC_CreateSceneNode,
//	eRC_EndFrame
//};

class RenderThread
{
public:
	RenderThread(RenderEngine* pRenderEngine);
	~RenderThread();

	void Start();
	void Run();

	void RC_Init();
	void RC_SetupDefaultCamera();
	void RC_SetupDefaultCompositor();
	void RC_LoadDefaultResources();
	void RC_SetupDefaultLight();
	void RC_BeginFrame();
	void RC_EndFrame();
	void RC_CreateSceneNode(RenderNode* pRenderNode);
	void SignalRenderThread();
	//void RC_UpdateSceneNode(RenderNode* pRenderNode, Ogre::Vector3 position, Ogre::Vector3 scale);

private:
	threadID m_nRenderThreadId;
	threadID m_nMainThreadId;

	std::atomic<UINT32> m_nFlush;

	std::unique_ptr<std::thread> m_pThread;

	RenderEngine* m_pRenderEngine;

	MTQueue<std::function<void(void)>> m_Commands;

	//inline void AddCommand();
	bool IsRenderThread();
	void ProcessCommands();

	inline bool CheckFlushCond();
	inline void SignalMainThread();
	void WaitForMainThreadSignal();
	void WaitForRenderThreadSignal();
	void SyncMainWithRender();
};

