#pragma once
#include "System.h"
#include "WindowManager.h"
#include "EventManager.h"
#include "glm\glm.hpp"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "GraphicsComponent.h"


namespace VEngine {

	class GraphicsSystem : public System,
		public EventSubscriber<Events::OnEntityCreated>,
		public EventSubscriber<Events::OnEntityDestroyed>,
		public EventSubscriber<Events::OnEntityInit>,
		public EventSubscriber<Events::OnComponentRemoved<GraphicsComponent>>
	{
	public:
		virtual ~GraphicsSystem() {}

		virtual void init(class Scene* scene);

		virtual void shutdown(class Scene* scene);

		virtual void tick(class Scene* scene);

		virtual void receive(class Scene* scene, const Events::OnEntityInit& event);

		virtual void receive(class Scene* scene, const Events::OnEntityCreated& event);

		virtual void receive(class Scene* scene, const Events::OnEntityDestroyed& event);

		virtual void receive(class Scene* scene, const Events::OnComponentRemoved<GraphicsComponent>& event);
	};

}
