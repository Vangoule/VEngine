#pragma once
#include <unordered_map>
#include <algorithm>
#include "Manager.h"
#include "Entity.h"
namespace VEngine {
	class BaseEventSubscriber
	{
	public:
		virtual ~BaseEventSubscriber() {};
	};

	template<typename T>
	class EventSubscriber : public BaseEventSubscriber
	{
	public:
		virtual ~EventSubscriber() {}

		virtual void receive(const T& event) = 0;
	};

	namespace Events
	{
		struct OnEntityCreated
		{
			Entity* entity;
		};

		struct OnEntityInit
		{
			Entity* entity;
		};

		struct OnEntityDestroyed
		{
			Entity* entity;
		};

		template<typename T>
		struct OnComponentAssigned
		{
			Entity* entity;
			ComponentHandle<T> component;
		};

		template<typename T>
		struct OnComponentRemoved
		{
			Entity* entity;
			ComponentHandle<T> component;
		};
	}

	class EventManager : public Manager<EventManager>
	{
	public:
		EventManager(void) { };
		~EventManager(void) {};

		template<typename T>
		inline void subscribe(EventSubscriber<T>* subscriber)
		{
			auto index = std::type_index(typeid(T));
			auto found = m_subscribers.find(index);
			if (found == m_subscribers.end())
			{
				std::vector<BaseEventSubscriber*> subList;
				subList.push_back(subscriber);

				m_subscribers.insert({ index, subList });
			}
			else
			{
				found->second.push_back(subscriber);
			}
		}

		template<typename T>
		inline void unsubscribe(EventSubscriber<T>* subscriber)
		{
			auto index = std::type_index(typeid(T));
			auto found = m_subscribers.find(index);
			if (found != m_subscribers.end())
			{
				found->second.erase(std::remove(found->second.begin(), found->second.end(), subscriber), found->second.end());
				if (found->second.size() == 0)
				{
					m_subscribers.erase(found);
				}
			}
		}

		inline void unsubscribeAll(void* subscriber)
		{
			for (auto kv : m_subscribers)
			{
				kv.second.erase(std::remove(kv.second.begin(), kv.second.end(), subscriber), kv.second.end());
				if (kv.second.size() == 0)
				{
					m_subscribers.erase(kv.first);
				}
			}
		}

		template<typename T>
		inline void emit(const T& event)
		{
			auto found = m_subscribers.find(std::type_index(typeid(T)));
			if (found != m_subscribers.end())
			{
				for (auto* base : found->second)
				{
					auto* sub = reinterpret_cast<EventSubscriber<T>*>(base);
					sub->receive(event);
				}
			}
		}

	private:
		std::unordered_map<std::type_index,
			std::vector<BaseEventSubscriber*>,
			std::hash<std::type_index>,
			std::equal_to<std::type_index>> m_subscribers;
	};


}