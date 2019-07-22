#pragma once
#include "Core.h"

#include "Entity.h"
#include "Scene.h"

#include "Component.h"
#include "GraphicsComponent.h"
#include "TransformComponent.h"

#include "SceneManager.h"
#include "SystemManager.h"

#include "GraphicsSystem.h"

namespace VEngine {

	void Engine::init()
	{
		std::cout << "Starting Engine" << std::endl;

		m_isRunning = true;

		WindowManager::get().Create(1920, 1080, "Game");

		Scene* scene = new Scene();

		Entity* e = scene->createEntity();
		e->addComponent<TransformComponent>(glm::vec3(0, 10, 10));
		e->addComponent<GraphicsComponent>();

		GraphicsSystem* gs = new GraphicsSystem();
		SystemManager::get().registerSystem(gs);

		SceneManager::get().setScene(scene);
				
		run();
	}

	void Engine::run()
	{
		update();
	}

	void Engine::update()
	{
		while (m_isRunning)
		{
			if (WindowManager::get().shouldClose())
				shutdown();

			InputManager::get().update();

			if (InputManager::get().getKeyDown(sf::Keyboard::Escape))
			{
				shutdown();
			}

			SystemManager::get().tick();

			WindowManager::get().getHandle()->display();
		}
	}

	void Engine::shutdown()
	{
		m_isRunning = false;
	}
}