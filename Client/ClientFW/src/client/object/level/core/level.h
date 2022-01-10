#pragma once
#include "client/object/core/base_object.h"

namespace client_fw
{
	class Actor;
	class ActorManager;
	struct EventKeyInfo;

	enum class eLevelState
	{
		kInGame, kPaused, kDead
	};

	class Level : public IBaseObject
	{
	public:
		Level(const std::string& name = "level");
		virtual ~Level();

		void InitializeLevel();
		virtual void Initialize() override {}

		void ShutdownLevel();
		virtual void Shutdown() override {}

		void UpdateLevel(float delta_time);
		virtual void Update(float delta_time) override {}

		void SpawnActor(const SPtr<Actor>& actor);

	protected:
		void RegisterPressedEvent(std::string_view name, std::vector<EventKeyInfo>&& keys,
			const std::function<bool()>& func, bool consumption = true);
		void RegisterReleasedEvent(std::string_view name, std::vector<EventKeyInfo>&& keys,
			const std::function<bool()>& func, bool consumption = true);

	private:
		void RegisterInputEvent(std::string_view name);

	protected:
		std::string m_name;
		eLevelState m_level_state;

	private:
		std::vector<std::string_view> m_registered_input_event;
		UPtr<ActorManager> m_actor_manager;
		bool m_is_runtime_level;

	public:
		const std::string& GetName() const { return m_name; }
		eLevelState GetLevelState() const { return m_level_state; }
		void SetLevelState(eLevelState level_state) { m_level_state = level_state; }
		void SetRuntime() { m_is_runtime_level = true; }
		bool IsRuntime() const { return m_is_runtime_level; }
	};
}

