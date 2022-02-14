#include "stdafx.h"
#include "client/physics/core/physics_world.h"
#include "client/physics/collision/collision_checker.h"

namespace client_fw
{
	PhysicsWorld::PhysicsWorld()
	{
	}

	PhysicsWorld::~PhysicsWorld()
	{
	}

	bool PhysicsWorld::Initialize()
	{
		return true;
	}

	void PhysicsWorld::Shutdown()
	{
	}

	void PhysicsWorld::Update(float delta_time)
	{
		m_collision_checker->CheckCollisions();
	}
}
