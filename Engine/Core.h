#pragma once
#include <Windows.h>
#include <iostream>

#include "WindowManager.h"

namespace VEngine {
	class Core
	{
	public:
		void init();
		void run();
		void update();
		void shutdown();
	private:

		bool isRunning;
		WindowManager* m_windowManager;
	};
}
