#pragma once
#include "Scene.h"
#include "EventManager.h"

namespace VEngine {
	class System
	{
	public:
		virtual ~System() {}

		virtual void init(Scene* scene) = 0;

		virtual void shutdown(Scene* scene) = 0;

		virtual void tick(Scene* scene) = 0;
	};
}