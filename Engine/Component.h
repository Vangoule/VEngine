#pragma once
namespace VEngine {

	class Component
	{	
	public:
		virtual ~Component() {};
	};

	template<typename T>
	class ComponentHandle
	{
	public:
		ComponentHandle()
			: component(nullptr)
		{
		}

		ComponentHandle(T* component)
			: component(component)
		{
		}

		T* operator->() const
		{
			return component;
		}

		operator bool() const
		{
			return isValid();
		}

		T& get()
		{
			return *component;
		}

		bool isValid() const
		{
			return component != nullptr;
		}

	private:
		T* component;
	};


}