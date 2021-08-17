#pragma once
#include "Manager.h"
#include "Scene.h"
#include "System.h"
#include "SceneManager.h"

#include <unordered_map>
#include <functional>
#include <vector>
#include <algorithm>


namespace VEngine {

	class SystemManager : public Manager<SystemManager>
	{
	public:
		SystemManager(void) { };
		~SystemManager(void) {
	
		};

		void shutdown()
		{
			for (auto* system : m_systems)
			{
				system->shutdown();
			}
		}

		void unregisterSystem(System* system);
				
		void enableSystem(System* system);

		void disableSystem(System* system);

		System* registerSystem(System* system);

		void tick();

		
		
	private:
		std::vector<System*> m_systems;
		std::vector<System*> m_disabledSystems;
	};

}

