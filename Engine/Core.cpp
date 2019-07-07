#include "Core.h"

namespace VEngine {

	void Core::init()
	{
		std::cout << "Starting Engine" << std::endl;

		isRunning = true;

		m_windowManager = new WindowManager();	

		if(!m_windowManager->createWindow())
		{
			std::cout << "Window Failed to Create!" << std::endl;
			shutdown();
		}
		
		update();
		
	}

	void Core::run()
	{
		
	}

	void Core::update()
	{
		while (isRunning)
		{
			if (!m_windowManager->update())
			{
				std::cout << "Window Failed to Update!" << std::endl;
				shutdown();
			}
		}
	}

	void Core::shutdown()
	{
		isRunning = false;
	}
}