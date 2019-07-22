#include <sstream>
#include "Manager.h"
#include "InputManager.h"
#include "WindowManager.h"

//Systems


//Components

namespace VEngine {

	const static int NUM_KEYS = 512;
	const static int NUM_MOUSEBUTTONS = 256;

	static bool inputs[NUM_KEYS];
	static bool downKeys[NUM_KEYS];
	static bool upKeys[NUM_KEYS];

	static bool mouseInput[NUM_MOUSEBUTTONS];
	static bool downMouse[NUM_MOUSEBUTTONS];
	static bool upMouse[NUM_MOUSEBUTTONS];

	static float mouseX = 0;
	static float mouseY = 0;


	void InputManager::clean()
	{
		for (int i = 0; i < NUM_MOUSEBUTTONS; i++)
		{
			downMouse[i] = false;
			upMouse[i] = false;
		}

		for (int i = 0; i < NUM_KEYS; i++)
		{
			downKeys[i] = false;
			upKeys[i] = false;
		}

	}

	void InputManager::getInput(sf::Event event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			int value = event.key.code;

			if (value >= 0 && value <= 512)
			{
				inputs[value] = true;
				downKeys[value] = true;
			}
		}
		else if (event.type == sf::Event::KeyReleased)
		{
			int value = event.key.code;

			if (value >= 0 && value <= 512)
			{
				inputs[value] = false;
				upKeys[value] = true;
			}
		}
		else if (event.type == sf::Event::MouseButtonReleased)
		{
			int value = event.mouseButton.button;

			mouseInput[value] = false;
			upMouse[value] = true;
		}
		else if (event.type == sf::Event::MouseButtonPressed)
		{
			int value = event.mouseButton.button;

			mouseInput[value] = true;
			downMouse[value] = true;

		}
		else if (event.type == sf::Event::MouseMoved)
		{
			mouseX = (float)event.mouseMove.x;
			mouseY = (float)event.mouseMove.y;
		}

	}

	bool InputManager::getKey(int keyCode)
	{
		return inputs[keyCode];
	}

	bool InputManager::getKeyDown(int keyCode)
	{
		return downKeys[keyCode];
	}

	bool InputManager::getKeyUp(int keyCode)
	{
		return upKeys[keyCode];
	}

	bool InputManager::getMouse(int button)
	{
		return mouseInput[button];
	}

	bool InputManager::getMouseDown(int button)
	{
		return downMouse[button];
	}

	bool InputManager::getMouseUp(int button)
	{
		return upMouse[button];
	}

	glm::vec2 InputManager::getMousePosition()
	{
		glm::vec2 pos((float)mouseX, (float)mouseY);
		return pos;
	}

	void InputManager::setCursor(bool visible)
	{
		if (visible)
			WindowManager::get().getHandle()->setMouseCursorVisible(true);
		else
			WindowManager::get().getHandle()->setMouseCursorVisible(false);
	}

	void InputManager::setMousePosition(glm::vec2 pos)
	{
		WindowManager::get().setMousePos((int)pos.x, (int)pos.y);
	}

	void InputManager::update()
	{
		clean();

		sf::Event event;

		while (WindowManager::get().getHandle()->pollEvent(event))
		{
			getInput(event);

			if (event.type == sf::Event::Closed) {
				WindowManager::get().Close();
			}
			else if (event.type == sf::Event::Resized)
			{
				WindowManager::get().setSize(event.size.width, event.size.height);


			}

		}
	}


}