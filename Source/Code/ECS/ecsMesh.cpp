#include "ecsMesh.h"
#include "ecsSystems.h"
#include "ecsScript.h"
#include "ecsPhys.h"
#include "flecs.h"
#include "../RenderEngine.h"
//#include "../ScriptSystem/ScriptNode.h"

void register_ecs_mesh_systems(flecs::world* ecs)
{
	ecs->system<RenderNodeComponent, const CameraPosition>().kind(flecs::PostUpdate)
		.each([&](RenderNodeComponent& renderNode, const CameraPosition& cameraPos)
			{
				renderNode.ptr->SetCameraPosition(cameraPos);
				renderNode.ptr->EnableCamera();
			});

	ecs->system<RenderNodeComponent, const Position>().kind(flecs::PostUpdate)
		.each([&](RenderNodeComponent& renderNode, const Position& pos)
			{
				renderNode.ptr->SetPosition(pos);
			});

	ecs->system<RenderNodeComponent, const Scale>().kind(flecs::PostUpdate)
		.each([&](RenderNodeComponent& renderNode, const Scale& scal)
			{
				renderNode.ptr->SetScale(scal);
			});

	ecs->system<RenderNodeComponent, const Orientation>().kind(flecs::PostUpdate)
		.each([&](RenderNodeComponent& renderNode, const Orientation& orient)
			{
				renderNode.ptr->SetOrientation(orient);
			});
}

