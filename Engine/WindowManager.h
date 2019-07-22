#pragma once
#include <iostream>

#include <SFML/Network.hpp>
#include <SFML/Window.hpp>

#include "Manager.h"

#define LOGGING_LEVEL_1
#include "Logger.h"
namespace VEngine {
	class WindowManager : public Manager<WindowManager>
	{

	public:
		WindowManager() : m_shouldClose(false), m_height(0), m_width(0) {};
		~WindowManager() {};

		void Create(int width, int height, std::string title);

		sf::Window* getHandle() {
			return &m_windowHndl;
		}

		void setMousePos(int posx, int posy);

		sf::Vector2i getMousePos();

		sf::Vector2u getSize();

		void setSize(int x, int y) {
			m_width = x;
			m_height = y;
		}

		void Close();
		bool shouldClose();

	private:
		sf::Window m_windowHndl;
		int m_width, m_height;
		std::string m_title;
		bool m_shouldClose;
	};
}