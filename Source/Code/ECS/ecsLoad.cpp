#include "ecsLoad.h"
#include "ecsPhys.h"
#include "ecsControl.h"
#include "ecsMesh.h"
#include "ecsScript.h"
#include "flecs.h"
#include "../RenderEngine.h"
//#include "../ScriptSystem/ScriptNode.h"

void register_ecs_load_systems(flecs::world* ecs)
{
	ecs->system<RenderNodeComponent, ScriptNodeComponent, CameraPosition>().kind(flecs::OnLoad)
		.each([&](RenderNodeComponent& renderNode, ScriptNodeComponent& scriptNode, CameraPosition& cameraPos)
	{
		Ogre::Vector3 vect = renderNode.ptr->GetCameraPosition();
		cameraPos.x = vect.x;
		cameraPos.y = vect.y;
		cameraPos.z = vect.z;
		//renderNode.ptr->EnableCamera();
	});

	ecs->system<RenderNodeComponent, ScriptNodeComponent, Position>().kind(flecs::OnLoad)
		.each([&](RenderNodeComponent& renderNode, ScriptNodeComponent& scriptNode, Position& pos)
	{
		Ogre::Vector3 vect = renderNode.ptr->GetPosition();
		pos.x = vect.x;
		pos.y = vect.y;
		pos.z = vect.z;
		scriptNode.ptr->SetPosition(vect);
	});

	ecs->system<RenderNodeComponent, ScriptNodeComponent, Scale>().kind(flecs::OnLoad)
		.each([&](RenderNodeComponent& renderNode, ScriptNodeComponent& scriptNode, Scale& scal)
	{
		Ogre::Vector3 vect = renderNode.ptr->GetScale();
		scal.x = vect.x;
		scal.y = vect.y;
		scal.z = vect.z;
		scriptNode.ptr->SetScale(vect);
	});

	ecs->system<RenderNodeComponent, ScriptNodeComponent, Orientation>().kind(flecs::OnLoad)
		.each([&](RenderNodeComponent& renderNode, ScriptNodeComponent& scriptNode, Orientation& orient)
	{
		Ogre::Quaternion quat = renderNode.ptr->GetOrientation();
		orient.x = quat.x;
		orient.y = quat.y;
		orient.z = quat.z;
		orient.w = quat.w;
	});
}