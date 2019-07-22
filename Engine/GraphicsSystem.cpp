#pragma once

#include "SceneManager.h"

#include "GraphicsSystem.h"

#include "GraphicsComponent.h"
#include "TransformComponent.h"

namespace VEngine {

	void GraphicsSystem::init(class Scene* scene)
	{

	}

	void GraphicsSystem::shutdown(class Scene* scene)
	{
		
	}

	void GraphicsSystem::receive(class Scene* scene, const Events::OnEntityCreated& event)
	{
		std::cout << "An entity was created!" << std::endl;
	}

	void GraphicsSystem::receive(class Scene* scene, const Events::OnEntityInit& event)
	{

	}

	void GraphicsSystem::receive(class Scene* scene, const Events::OnEntityDestroyed& event)
	{
		std::cout << "An entity was destroyed!" << std::endl;
	}

	void GraphicsSystem::receive(class Scene* scene, const Events::OnComponentRemoved<GraphicsComponent>& event)
	{

	}

	void GraphicsSystem::tick(class Scene* scene) {

		for (Entity* ent : SceneManager::get().each<GraphicsComponent>())
		{
			if (ent->get<TransformComponent>()->getPosition().x >= 10)
			{
				SceneManager::get().getScene()->removeEntity(ent);
			}
			else
			{
				glm::vec3 pos = ent->get<TransformComponent>()->getPosition();
				pos.x += 0.01f;
				
				ent->get<TransformComponent>()->setPosition(pos);
				printf("%.3f\n", ent->get<TransformComponent>()->getPosition().x);

			}
		}

	}

}