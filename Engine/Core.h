#pragma once
#include <Windows.h>
#include <iostream>

#include "WindowManager.h"
#include "InputManager.h"

namespace VEngine {
	class Engine
	{
	public:
		void init();
		void run();
		void update();
		void shutdown();
	private:

		bool m_isRunning;
	};
}
