#pragma once

#include "Scene.h"

#include <algorithm>
namespace VEngine {
	bool Scene::cleanup()
	{
		size_t count = 0;
		m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(), [&, this](Entity* ent) {
			if (ent->isPendingDestroy())
			{
				EventManager::get().emit<Events::OnEntityDestroyed>({ ent });
				delete ent;
				++count;
				return true;
			}

			return false;
			}), m_entities.end());

		return count > 0;
	}

	void Scene::destroy()
	{
		size_t count = 0;
		m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(), [&, this](Entity* ent) {

				EventManager::get().emit<Events::OnEntityDestroyed>({ ent });
				delete ent;
				++count;
				return true;


			}), m_entities.end());

	}

	void Scene::removeEntity(Entity* ent, bool immediate)
	{
		if (ent == nullptr)
			return;

		if (ent->isPendingDestroy())
		{
			if (immediate)
			{
				EventManager::get().emit<Events::OnEntityDestroyed>({ ent });
				m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), ent), m_entities.end());
				delete ent;
			}

			return;
		}

		ent->remove();

		if (immediate)
		{
			EventManager::get().emit<Events::OnEntityDestroyed>({ ent });
			m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), ent), m_entities.end());
			delete ent;
		}
	}
}