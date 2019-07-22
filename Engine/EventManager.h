#pragma once
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

		virtual void receive(Scene* scene, const T& event) = 0;
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
}