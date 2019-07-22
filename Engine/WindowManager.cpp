#include "WindowManager.h"
#include <sstream>

namespace VEngine {

	void WindowManager::Create(int width, int height, std::string title)
	{
		m_width = width;
		m_height = height;
		m_title = title;

		sf::ContextSettings settings;
		settings.depthBits = 24;
		settings.stencilBits = 8;
		settings.antialiasingLevel = 4;
		settings.majorVersion = 4;
		settings.minorVersion = 6;

		m_windowHndl.create(sf::VideoMode(1920, 1080), "Main Window", sf::Style::Default, settings);


		sf::ContextSettings settingsCheck = m_windowHndl.getSettings();
		LOG("Initiating Window");
		LOG("Your computer supports: ");
		LOG("Depth Bits:", settingsCheck.depthBits);
		LOG("Stencil Bits:", settingsCheck.stencilBits);
		LOG("Antialiasing Level:", settingsCheck.antialiasingLevel);
		LOG("Version:", settingsCheck.majorVersion, ".", settingsCheck.minorVersion);

		m_windowHndl.setFramerateLimit(144);
		m_windowHndl.setVerticalSyncEnabled(false);

	}


	sf::Vector2i WindowManager::getMousePos()
	{
		sf::Vector2i localPosition = sf::Mouse::getPosition(m_windowHndl);
		return localPosition;
	}

	void WindowManager::setMousePos(int posx, int posy)
	{
		sf::Mouse::setPosition(sf::Vector2i(posx, posy), m_windowHndl);
	}


	sf::Vector2u WindowManager::getSize()
	{
		return m_windowHndl.getSize();
	}

	void WindowManager::Close() {
		m_shouldClose = true;
	}

	bool WindowManager::shouldClose()
	{
		return m_shouldClose;
	}

}