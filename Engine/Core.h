#pragma once
#include <Windows.h>
#include <iostream>

#include "WindowManager.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "SystemManager.h"

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
		SystemManager* systemManager;
		InputManager* inputManager;
		SceneManager* sceneManager;
		WindowManager* windowManager;

	};
}
