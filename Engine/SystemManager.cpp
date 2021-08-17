#include "SystemManager.h"

#include "SceneManager.h"
namespace VEngine {

	System* SystemManager::registerSystem(System* system)
	{
		m_systems.push_back(system);
		system->init();

		return system;
	}

	void SystemManager::unregisterSystem(System* system)
	{

		m_systems.erase(std::remove(m_systems.begin(), m_systems.end(), system), m_systems.end());
		system->shutdown();
	}

	void SystemManager::enableSystem(System* system)
	{
		auto it = std::find(m_disabledSystems.begin(), m_disabledSystems.end(), system);
		if (it != m_disabledSystems.end())
		{
			m_disabledSystems.erase(it);
			m_systems.push_back(system);
		}
	}

	void SystemManager::disableSystem(System* system)
	{
		auto it = std::find(m_systems.begin(), m_systems.end(), system);
		if (it != m_systems.end())
		{
			m_systems.erase(it);
			m_disabledSystems.push_back(system);
		}
	}

	void SystemManager::tick()
	{
		SceneManager::get().getScene()->cleanup();

		for (auto* system : m_systems)
		{			
			system->tick();
		}
	}

}