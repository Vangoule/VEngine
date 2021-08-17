#pragma once
#include "Core.h"

#include "Entity.h"
#include "Scene.h"

#include "Component.h"
#include "GraphicsComponent.h"
#include "TransformComponent.h"

#include "GraphicsSystem.h"

#include <GLFW/glfw3.h>

namespace VEngine {
	VulkanModelComponent* dragon;
	void Engine::init()
	{
		LOG("Starting Engine");

		m_isRunning = true;

		windowManager = &WindowManager::get();
		sceneManager = &SceneManager::get();
		inputManager = &InputManager::get();
		systemManager = &SystemManager::get();

		windowManager->Create(1920, 1080, "Game");
		inputManager->init();

		GraphicsSystem* gs = new GraphicsSystem();
		systemManager->registerSystem(gs);
		
		Scene* scene = new Scene();

		//Entity* e = scene->createEntity();
		//e->addComponent<TransformComponent>(glm::vec3(0, 1, 0));
		//e->addComponent<GraphicsComponent>("resources/models/cube.obj");

		Entity* e2 = scene->createEntity();
		e2->addComponent<TransformComponent>(glm::vec3(0, 2, 0));
		e2->addComponent<GraphicsComponent>("resources/models/dragon2.obj");
			   
		scene->initAll();

		ComponentHandle<GraphicsComponent> gc = e2->get<GraphicsComponent>();
		dragon = gc->model;
			   
		sceneManager->setScene(scene);
				
		run();
	}

	void Engine::run()
	{
		update();
	}

	int height = 3;
	std::vector<Entity*> entitiesAdded;
	void Engine::update()
	{
		double previousTime = glfwGetTime();
		double lastTime = glfwGetTime();
		int frameCount = 0;

		while (m_isRunning)
		{
			double currentTime = glfwGetTime();
			frameCount++;

			if (windowManager->shouldClose())
				shutdown();

			inputManager->update();

			if (inputManager->getKey(GLFW_KEY_F))
			{
				Entity* e = sceneManager->getScene()->createEntity();
				e->addComponent<TransformComponent>(glm::vec3(0, 0, height+=2));
				e->addComponent<GraphicsComponent>("resources/models/cube.obj");
				sceneManager->getScene()->init(e);
				entitiesAdded.push_back(e);
			}

			if (inputManager->getKey(GLFW_KEY_G))
			{
				if (entitiesAdded.size() > 0) {
					entitiesAdded.back()->remove();
					height-=2;
					entitiesAdded.pop_back();
				}
			}

			if (inputManager->getKeyDown(GLFW_KEY_ESCAPE) || inputManager->getKeyDown(GLFW_KEY_Q) || inputManager->getKeyDown(GLFW_KEY_SPACE))
			{
				shutdown();
			}

			systemManager->tick();

			
			//Limit FPS
			while (glfwGetTime() < lastTime + 1.0 / 144) {
				
			}
			lastTime += 1.0 / 144;

			//Display FPS
			if (currentTime - previousTime >= 1.0)
			{
				glfwSetWindowTitle(WindowManager::get().getHandle(), std::to_string(frameCount).c_str());

				frameCount = 0;
				previousTime = currentTime;
			}

		}

		sceneManager->cleanupScene();

		systemManager->shutdown();

		windowManager->destroy();
		
	}

	void Engine::shutdown()
	{
	
		windowManager->close();
		m_isRunning = false;
	}
}