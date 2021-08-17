#pragma once
#include "System.h"
#include "WindowManager.h"
#include "EventManager.h"
#include "glm\glm.hpp"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "GraphicsComponent.h"
#include "VulkanRenderer.h"

namespace VEngine {

	class GraphicsSystem : public System,
		public EventSubscriber<Events::OnEntityCreated>,
		public EventSubscriber<Events::OnEntityDestroyed>,
		public EventSubscriber<Events::OnEntityInit>,
		public EventSubscriber<Events::OnComponentRemoved<GraphicsComponent>>
	{
	public:
		virtual ~GraphicsSystem() {}

		virtual void init();

		virtual void shutdown();

		virtual void tick();

		virtual void receive(const Events::OnEntityInit& event);

		virtual void receive(const Events::OnEntityCreated& event);

		virtual void receive(const Events::OnEntityDestroyed& event);

		virtual void receive(const Events::OnComponentRemoved<GraphicsComponent>& event);
	private:
		VulkanRenderer* m_renderer;
	};

}
