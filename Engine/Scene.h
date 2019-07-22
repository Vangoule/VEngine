#pragma once
#include "Entity.h"
#include "Component.h"
#include <vector>
#include <functional>

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

			return ent;
		}

		void removeEntity(Entity* ent, bool immediate = false);

		bool cleanup();

		void reset();
		
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