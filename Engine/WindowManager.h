#pragma once
#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

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

		GLFWwindow* getHandle() {
			return m_window;
		}

		//void setMousePos(int posx, int posy);

		//glm::vec2 getMousePos() {};
		
		glm::vec2 getSize() { return glm::vec2(m_width, m_height); };

		void setSize(int x, int y) {
			m_width = x;
			m_height = y;
		}

		void close();
		bool shouldClose();

		void destroy()
		{
			glfwDestroyWindow(m_window);
			glfwTerminate();
		}


	private:
		GLFWwindow* m_window;
		int m_width, m_height;
		std::string m_title;
		bool m_shouldClose;
	};
}