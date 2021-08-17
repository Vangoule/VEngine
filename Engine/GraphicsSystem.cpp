#pragma once

#include "GraphicsSystem.h"

#include "SceneManager.h"
#include "EventManager.h"

#include "GraphicsComponent.h"
#include "TransformComponent.h"

namespace VEngine {

	void GraphicsSystem::init()
	{
		EventManager* eventManager = &EventManager::get();
		eventManager->subscribe<Events::OnEntityDestroyed>(this);
		eventManager->subscribe<Events::OnEntityInit>(this);
		eventManager->subscribe<Events::OnEntityCreated>(this);

		m_renderer = new VulkanRenderer();
		m_renderer->initVulkan((int)WindowManager::get().getSize().x, (int)WindowManager::get().getSize().y);
	}

	void GraphicsSystem::shutdown()
	{

		m_renderer->cleanup();
	}

	void GraphicsSystem::receive(const Events::OnEntityCreated& event)
	{
		GLOG( "An entity was created!");
	}

	void GraphicsSystem::receive(const Events::OnEntityInit& event)
	{
		GLOG("An entity was initialized!");
		Entity* ent = event.entity;

		if (ent->has<GraphicsComponent>())
		{
			ComponentHandle<GraphicsComponent> gc = ent->get<GraphicsComponent>();

			ModelCreateInfo modelCreateInfo;

			if (ent->has<TransformComponent>())
			{
				ComponentHandle<TransformComponent> tc = ent->get<TransformComponent>();
				modelCreateInfo = ModelCreateInfo(glm::vec3(1.0f), glm::vec3(1.0f), tc->getPosition());
			}
			else
			{
				modelCreateInfo = ModelCreateInfo(glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			}

			
			
			if (gc->filename.size() != 0)
			{
				gc->model = new VulkanModelComponent();
				gc->model->model = new Model();
				gc->model->model->loadFromFile(gc->filename, vertexLayout, modelCreateInfo, m_renderer->getDevice(), m_renderer->getDevice()->getGraphicsQueue());
			}
			else
			{
				gc->model->model->device = m_renderer->getDevice()->getDevice();
			}

			m_renderer->pushBackModel(ent->getId(), gc->model);

			

			//m_renderer->createCommandBuffers();
		}
	}
	bool refresh = false;
	void GraphicsSystem::receive(const Events::OnEntityDestroyed& event)
	{
		GLOG("An entity was destroyed!");
		Entity* ent = event.entity;

		if (ent->has<GraphicsComponent>())
		{
			ComponentHandle<GraphicsComponent> gc = ent->get<GraphicsComponent>();
			
			vkDeviceWaitIdle(m_renderer->getDevice()->getDevice());
			gc->model->model->destroy();
			delete gc->model;
			m_renderer->removeModel(ent->getId());
		}

		
	}

	void GraphicsSystem::receive(const Events::OnComponentRemoved<GraphicsComponent>& event)
	{

	}

	void GraphicsSystem::tick() {

		for (Entity* ent : SceneManager::get().each<GraphicsComponent>())
		{
			ComponentHandle<GraphicsComponent> gc = ent->get<GraphicsComponent>();
			

		}


		if (m_renderer->sceneChanged)
		{
			m_renderer->updateCommandBuffers();
			m_renderer->sceneChanged = false;
		}

		m_renderer->drawFrame();
	}

}