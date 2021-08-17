#pragma once
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm\glm.hpp"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include "Manager.h"
#include "WindowManager.h"
#include <GLFW/glfw3.h>
namespace VEngine {



	class InputManager : public Manager<InputManager>
	{
		friend class Manager<InputManager>;

	public:
		InputManager(void) {};
		~InputManager(void) {};

		static bool getKey(int keyCode);
		static bool getKeyDown(int keyCode);
		static bool getKeyUp(int keyCode);

		static bool getMouse(int button);
		static bool getMouseDown(int button);
		static bool getMouseUp(int button);

		static glm::vec2 getMousePosition();

		static void setCursor(bool value);
		static void setMousePosition(glm::vec2 pos);
	
		static void init();

		static void update();

		static void clean();


		bool mouseLocked = false;
		bool firstMouse = false;

	private:
		static void getKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void getMouseInput(GLFWwindow* window, int button, int action, int mods);
		static void getMouseMoved(GLFWwindow* window, double xpos, double ypos);

	};
}