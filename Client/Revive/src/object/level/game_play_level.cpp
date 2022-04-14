#include <include/client_core.h>
#include <client/input/input.h>
#include <client/object/actor/default_pawn.h>
#include <client/object/actor/core/actor.h>
#include <client/object/actor/static_mesh_actor.h>
#include <client/util/octree/octree.h>
#include "object/level/game_play_level.h"
#include <client/object/level/gamemode/game_mode_base.h>
#include "object/gamemode/revive_game_mode.h"


namespace revive
{

	GamePlayLevel::GamePlayLevel()
		: Level("game play level")
	{
	}
	bool GamePlayLevel::Initialize()
	{
		/*auto police = CreateSPtr<SkeletonSoldier>();
		SpawnActor(police);*/

		/*for (int i = 0; i < 15; ++i)
		{
			auto police = CreateSPtr<StaticMeshActor>(eMobilityState::kStatic, "Contents/cliff_block_rock.obj");
			police->SetScale(300.0f); 
			police->SetPosition(Vec3{ -500.0f + i * 500.0f, 0.0f, 6000.0f  });
			SpawnActor(police);
		}*/
		
		m_actors = m_map_loader.LoadMap("Contents/map.txt",eMapLoadType::kClient);
		for (auto& actor : m_actors)
		{
			SpawnActor(actor);
		}
		

		Input::SetInputMode(eInputMode::kGameOnly);
		Input::SetHideCursor(true);

		return true;
	}
	void GamePlayLevel::Shutdown()
	{
		Input::SetHideCursor(false);
		Input::SetClipCursor(false);
		Input::SetInputMode(eInputMode::kUIOnly);
	}
	void GamePlayLevel::Update(float delta_time)
	{
		
	}
	UPtr<GameMode> GamePlayLevel::CreateGameMode() const
	{
		return CreateUPtr<ReviveGameMode>();
	}
	std::vector<SPtr<VisualOctree>> GamePlayLevel::CreateVisualOctrees() const
	{
		std::vector<SPtr<VisualOctree>> visual_octrees;
		//박스의 HalfWidth, 중심좌표
		visual_octrees.emplace_back(CreateSPtr<VisualOctree>(5000.0f,Vec3(2500.0f,0,2500.0f))); //Castle
		visual_octrees.emplace_back(CreateSPtr<VisualOctree>(5000.0f,Vec3(2500,0,7500.0f))); //Bridge + Spawn Area
		return visual_octrees;
	}
	std::vector<SPtr<CollisionOctree>> GamePlayLevel::CreateCollisionOctrees() const
	{
		std::vector<SPtr<CollisionOctree>> collision_octrees;
		collision_octrees.emplace_back(CreateSPtr<CollisionOctree>(5000.0f, Vec3(2500.0f, 0, 2500.0f)));
		collision_octrees.emplace_back(CreateSPtr<CollisionOctree>(5000.0f, Vec3(2500.0f, 0, 7500.0f)));
		collision_octrees.emplace_back(CreateSPtr<CollisionOctree>(5000.0f, Vec3(2500.0f, 0, 12500.0f))); //가로는 2500만큼 감싸고(양옆) 세로는 1000만큼 감싸야하지만 정사각형이므로 2500만큼 감싼다.
		return collision_octrees;
	}
}