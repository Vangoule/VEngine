#include <sstream>
#include "Manager.h"
#include "InputManager.h"
#include "WindowManager.h"
#include "SceneManager.h"
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

	static double mouseX = 0;
	static double mouseY = 0;


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

	void InputManager::getMouseInput(GLFWwindow* window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			if (button >= 0 && button <= 512)
			{
				mouseInput[button] = true;
				downMouse[button] = true;
			}
		}

		if (action == GLFW_RELEASE)
		{
			if (button >= 0 && button <= 512)
			{
				mouseInput[button] = false;
				upMouse[button] = true;
			}
		}
	}

	void InputManager::getMouseMoved(GLFWwindow* window, double xpos, double ypos)
	{
		mouseX = xpos;
		mouseY = ypos;
	}

	void InputManager::getKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		clean();

		if (action == GLFW_PRESS)
		{
			if (key >= 0 && key <= 512)
			{
				inputs[key] = true;
				downKeys[key] = true;
			}
		}

		if (action == GLFW_RELEASE)
		{
			if (key >= 0 && key <= 512)
			{
				inputs[key] = false;
				upKeys[key] = true;
			}
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

	//void InputManager::setCursor(bool visible)
	//{
	//	if (visible)
	//		WindowManager::get().getHandle()->setMouseCursorVisible(true);
	//	else
	//		WindowManager::get().getHandle()->setMouseCursorVisible(false);
	//}

	void InputManager::setMousePosition(glm::vec2 pos)
	{
		//WindowManager::get().setMousePos((int)pos.x, (int)pos.y);
	}
	void InputManager::init()
	{
		glfwSetKeyCallback(WindowManager::get().getHandle(), getKeyboardInput);
		glfwSetCursorPosCallback(WindowManager::get().getHandle(), getMouseMoved);
		glfwSetMouseButtonCallback(WindowManager::get().getHandle(), getMouseInput);
	}
	void InputManager::update()
	{
		clean();
		glfwPollEvents();
		

		//sf::Event event;

		//while (WindowManager::get().getHandle()->pollEvent(event))
		//{
		//	getInput(event);

		//	if (event.type == sf::Event::Closed) {
		//		WindowManager::get().Close();
		//	}
		//	else if (event.type == sf::Event::Resized)
		//	{
		//		WindowManager::get().setSize(event.size.width, event.size.height);


		//	}

		//}
	}


}