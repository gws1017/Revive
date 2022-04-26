#pragma once

namespace client_fw
{
	class VisualOctree;
	class CollisionOctree;
	class SceneComponent;
	class RenderComponent;
	class CameraComponent;

	class OctreeManager
	{
	public:
		OctreeManager();
		~OctreeManager() = default;

		OctreeManager(const OctreeManager&) = delete;
		OctreeManager& operator=(const OctreeManager&) = delete;

		void Update();

		void RegisterVisualOctrees(std::vector<SPtr<VisualOctree>>&& octrees);
		void RegisterCollisionOctrees(std::vector<SPtr<CollisionOctree>>&& octrees);
		void UnregisterOctrees();

	private:
		UPtr<class VisualOctreeManager> m_visual_octree_manager;
		UPtr<class CollisionOctreeManager> m_collision_octree_manager;
	};

	class VisualOctreeManager
	{
	public:
		VisualOctreeManager();
		~VisualOctreeManager() = default;

		VisualOctreeManager(const VisualOctreeManager&) = delete;
		VisualOctreeManager& operator=(const VisualOctreeManager&) = delete;

		void RegisterOctrees(std::vector<SPtr<VisualOctree>>&& octrees);
		void UnregisterOctrees();

		void RegisterRenderComponent(const SPtr<RenderComponent>& render_comp);
		void UnregisterRenderComponent(const SPtr<RenderComponent>& render_comp);

	private:
		static VisualOctreeManager* s_instance;
		bool m_is_active = false;
		std::vector<SPtr<VisualOctree>> m_visual_octrees;
		std::vector<SPtr<RenderComponent>> m_movable_render_comps;

	public:
		static VisualOctreeManager& GetOctreeManager() { return *s_instance; }
		const std::vector<SPtr<VisualOctree>> GetVisualOctrees() const { return m_visual_octrees; }
		const std::vector<SPtr<RenderComponent>> GetMovableRenderComps() const { return m_movable_render_comps; }
	};

	class CollisionOctreeManager
	{
	public:
		CollisionOctreeManager();
		~CollisionOctreeManager() = default;

		CollisionOctreeManager(const CollisionOctreeManager&) = delete;
		CollisionOctreeManager& operator=(const CollisionOctreeManager&) = delete;

		void Update();

		void RegisterOctrees(std::vector<SPtr<CollisionOctree>>&& octrees);
		void UnregisterOctrees();

		void RegisterSceneComponent(const SPtr<SceneComponent>& scene_comp);
		void UnregisterSceneComponent(const SPtr<SceneComponent>& scene_comp);
		void ReregisterSceneComponent(const SPtr<SceneComponent>& scene_comp);

	private:
		static CollisionOctreeManager* s_instance;
		bool m_is_active = false;
		bool m_checking_collision = false;
		std::vector<SPtr<CollisionOctree>> m_collision_octrees;

		std::vector<SPtr<SceneComponent>> m_ready_registered_scene_comp;
		std::vector<SPtr<SceneComponent>> m_ready_unregistered_scene_comp;

	public:
		static CollisionOctreeManager& GetOctreeManager() { return *s_instance; }
		const std::vector<SPtr<CollisionOctree>> GetCollisionOctrees() const { return m_collision_octrees; }
		void CheckCollsion(bool check) { m_checking_collision = check; }
	};
}


