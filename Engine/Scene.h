#pragma once
#include "Entity.h"
#include "Component.h"
#include <vector>
#include <functional>
#include "EventManager.h"

namespace VEngine {

	class Scene
	{
	public:
		Scene()
		{
			lastEntityId = 0;
		}

		~Scene();

		Entity* createEntity()
		{
			Entity* ent = new Entity();
			ent->setId(lastEntityId);
			++lastEntityId;
			
			m_entities.push_back(ent);

			EventManager::get().emit<Events::OnEntityCreated>({ ent });

			return ent;
		}

		void initEntity(Entity* ent)
		{
			ent->init();
			EventManager::get().emit<Events::OnEntityInit>({ ent });
		}

		void initAll()
		{
			for (auto ent : m_entities)
			{
				if (!ent->getWasInit())
				{
					ent->init();
					EventManager::get().emit<Events::OnEntityInit>({ ent });
				}
			}
		}

		void init(Entity* e)
		{
			e->init();
			EventManager::get().emit<Events::OnEntityInit>({ e });
		}

		void removeEntity(Entity* ent, bool immediate = false);

		bool cleanup();

		void destroy();
		
		size_t getEntityCount() const
		{
			return m_entities.size();
		}

		Entity* getByIndex(size_t idx)
		{
			if (idx >= getEntityCount())
				return nullptr;

			return m_entities[idx];
		}

	private:

		std::vector<Entity*> m_entities;

		int lastEntityId = 0;
	};
}