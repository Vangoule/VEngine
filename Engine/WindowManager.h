#pragma once
#include <SFML/Window.hpp>

namespace VEngine
{
class WindowManager
{
public:
	bool createWindow();
	bool update();

private:
	sf::Window* m_window;

};
}
