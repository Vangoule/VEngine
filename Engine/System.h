#pragma once
#include "EventManager.h"

namespace VEngine {
	class System
	{
	public:
		virtual ~System() {}

		virtual void init() = 0;

		virtual void shutdown() = 0;

		virtual void tick() = 0;
	};
}