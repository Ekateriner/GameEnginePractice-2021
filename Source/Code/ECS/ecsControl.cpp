#include "ecsControl.h"
#include "ecsSystems.h"
#include "ecsScript.h"
#include "ecsPhys.h"
#include "flecs.h"
#include "../Input/InputHandler.h"

void register_ecs_control_systems(flecs::world* ecs)
{
	static auto inputQuery = ecs->query<InputHandlerPtr>();
	ecs->system<const Controllable, ScriptNodeComponent, CameraPosition>().kind(flecs::PreUpdate)
		.each([&](flecs::entity e, const Controllable&, ScriptNodeComponent& scriptNode, CameraPosition& cameraPos)
			{
				Ogre::Vector3 vCameraPosition = scriptNode.ptr->GetCameraPosition();
				cameraPos.x = vCameraPosition.x;
				cameraPos.y = vCameraPosition.y;
				cameraPos.z = vCameraPosition.z;
			});

	ecs->system<const Controllable, ScriptNodeComponent, Position>().kind(flecs::PreUpdate)
		.each([&](flecs::entity e, const Controllable&, ScriptNodeComponent& scriptNode, Position& pos)
			{
				Ogre::Vector3 vPosition = scriptNode.ptr->GetPosition();
				pos.x = vPosition.x;
				pos.y = vPosition.y;
				pos.z = vPosition.z;
			});

	ecs->system<const Controllable, ScriptNodeComponent, Scale>().kind(flecs::PreUpdate)
		.each([&](flecs::entity e, const Controllable&, ScriptNodeComponent& scriptNode, Scale& scal)
	{
		Ogre::Vector3 vScale = scriptNode.ptr->GetScale();
		scal.x = vScale.x;
		scal.y = vScale.y;
		scal.z = vScale.z;
	});


	ecs->system<ScriptNodeComponent, Orientation>().kind(flecs::PreUpdate)
		.each([&](flecs::entity e, ScriptNodeComponent& scriptNode, Orientation& orient)
			{
				Ogre::Quaternion orientation = scriptNode.ptr->GetOrientation();
				orient.x = orientation.x;
				orient.y = orientation.y;
				orient.z = orientation.z;
				orient.w = orientation.w;
			});
}

