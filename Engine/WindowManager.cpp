#include "WindowManager.h"
#include <sstream>

namespace VEngine {

	void WindowManager::Create(int width, int height, std::string title)
	{
		m_width = width;
		m_height = height;
		m_title = title;

		if (!glfwInit())
		{
			ELOG("GLFW not initialized.");
		}

		if (!glfwVulkanSupported())
		{
			ELOG("Vulkan not supported.");
		}

		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		bool fullScreen = false;

		if(fullScreen)
			m_window = glfwCreateWindow(m_width, m_height, "Main Window", glfwGetPrimaryMonitor(), NULL);
		else
			m_window = glfwCreateWindow(m_width, m_height, "Main Window", NULL, NULL);


		LOG("Initializing Window");
	}


	//sf::Vector2i WindowManager::getMousePos()
	//{
	//	sf::Vector2i localPosition = sf::Mouse::getPosition(m_windowHndl);
	//	return localPosition;
	//}

	//void WindowManager::setMousePos(int posx, int posy)
	//{
	//	sf::Mouse::setPosition(sf::Vector2i(posx, posy), m_windowHndl);
	//}


	//sf::Vector2u WindowManager::getSize()
	//{
	//	return m_windowHndl.getSize();
	//}

	void WindowManager::close() {
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
		m_shouldClose = true;
	}

	bool WindowManager::shouldClose()
	{
		return glfwWindowShouldClose(m_window);
	}

}