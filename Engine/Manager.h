#pragma once
#include <iostream>

#define LOGGING_LEVEL_1
#include "Logger.h"

namespace VEngine {
	template <typename T>
	class Manager
	{
	public:

		static T& get() {
			static T m_instance;

			return m_instance;
		};

		Manager(Manager const&) = delete;
		Manager& operator=(Manager const&) = delete;

	protected:
		Manager() {}
		~Manager() {}

	};

}

