#include "WindowManager.h"

namespace VEngine
{
	bool WindowManager::createWindow()
	{
		m_window = new sf::Window(sf::VideoMode(1920, 1080), "Main Window");

		if (m_window != NULL)
			return true;
		return false;

	}

	bool WindowManager::update()
	{
		if (m_window->isOpen())
		{
			sf::Event event;
			while (m_window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed) {
					m_window->close();
					return false;
				}
			}
			
			m_window->display();
		}

		return true;

	}
}