#pragma once

#include <map>
#include <string>
#include <typeindex>
#include "Component.h"
#include <functional>

namespace VEngine {
	class Entity {
	public:
		Entity() { m_id = 0; m_wasInit = false; m_isPendingDestroy = false; };
		~Entity();

		template <typename T>
		T* removeComponent() {
			auto it = m_components.find(std::type_index(typeid(T)));
			if (it != m_components.end()) {
				m_components.erase(it);
			}
			return nullptr;
		}

		template<typename T, typename... Args>
		ComponentHandle<T> addComponent(Args&& ... args)
		{
			auto found = m_components.find(std::type_index(typeid(T)));
			if (found != m_components.end())
			{
				T* component = new T(std::forward<Args>(args)...);
				auto handle = ComponentHandle<T>(component);

				return handle;
			}
			else
			{
				T* component = new T(std::forward<Args>(args)...);
				m_components.insert({ std::type_index(typeid(T)), component });

				auto handle = ComponentHandle<T>(component);
				return handle;
			}
		}

		template<typename T>
		ComponentHandle<T> get()
		{
			auto it = m_components.find(std::type_index(typeid(T)));
			if (it != m_components.end()) {
				return ComponentHandle<T>((T*)it->second);
			}
			return nullptr;
		}

		template<typename T>
		T getComponent()
		{
			auto it = m_components.find(std::type_index(typeid(T)));
			if (it != m_components.end()) {
				return ((T*)it->second);
			}
			return nullptr;
		}

		template <typename T>
		bool has() {
			auto it = m_components.find(std::type_index(typeid(T)));
			if (it != m_components.end()) {
				return true;
			}
			return false;
		}

		template<typename... Types>
		bool with(typename std::common_type<std::function<void(ComponentHandle<Types>...)>>::type view)
		{
			if (!has<Types...>())
				return false;

			view(get<Types>()...);
			return true;
		}

		bool isPendingDestroy() const
		{
			return m_isPendingDestroy;
		}

		void remove()
		{
			m_isPendingDestroy = true;
		}

		int getId() { return m_id; };

		void setId(int id) { m_id = id; };

		void init()
		{
			m_wasInit = true;
		}

		bool getWasInit()
		{
			return m_wasInit;
		}

	private:
		std::map<std::type_index, Component*> m_components;
		int m_id;
		bool m_isPendingDestroy;
		bool m_wasInit;
	};
}

