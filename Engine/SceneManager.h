#pragma once
#include "Manager.h"
#include "Scene.h"
namespace VEngine {
	class EntityIterator
	{
	public:
		EntityIterator(Scene* Scene, size_t index, bool isEnd, bool includePendingDestroy);

		size_t getIndex() const
		{
			return index;
		}

		bool isEnd() const;

		bool includePendingDestroy() const
		{
			return m_includePendingDestroy;
		}

		Scene* getScene() const
		{
			return m_scene;
		}

		Entity* get() const;

		Entity* operator*() const
		{
			return get();
		}

		bool operator==(const EntityIterator& other) const
		{
			if (m_scene != other.getScene())
				return false;

			if (isEnd())
				return other.isEnd();

			return index == other.index;
		}

		bool operator!=(const EntityIterator& other) const
		{
			if (m_scene != other.getScene())
				return true;

			if (isEnd())
				return !other.isEnd();

			return index != other.index;
		}

		EntityIterator& operator++();

	private:
		bool m_isEnd = false;
		size_t index;
		class Scene* m_scene;
		bool m_includePendingDestroy;
	};

	class EntityView
	{
	public:
		EntityView(const EntityIterator& first, const EntityIterator& last)
			: firstItr(first), lastItr(last)
		{
			if (firstItr.get() == nullptr || (firstItr.get()->isPendingDestroy() && !firstItr.includePendingDestroy()))
			{
				++firstItr;
			}
		}

		const EntityIterator& begin() const
		{
			return firstItr;
		}

		const EntityIterator& end() const
		{
			return lastItr;
		}

	private:
		EntityIterator firstItr;
		EntityIterator lastItr;
	};

	template<typename... Types>
	class EntityComponentIterator
	{
	public:
		EntityComponentIterator(Scene* scene, size_t index, bool isEnd, bool includePendingDestroy);

		size_t getIndex() const
		{
			return index;
		}

		bool isEnd() const;

		bool includePendingDestroy() const
		{
			return m_includePendingDestroy;
		}

		Scene* getScene() const
		{
			return m_scene;
		}

		Entity* get() const;

		Entity* operator*() const
		{
			return get();
		}

		bool operator==(const EntityComponentIterator<Types...>& other) const
		{
			if (m_scene != other.getScene())
				return false;

			if (isEnd())
				return other.isEnd();

			return index == other.index;
		}

		bool operator!=(const EntityComponentIterator<Types...>& other) const
		{
			if (m_scene != other.getScene())
				return true;

			if (isEnd())
				return !other.isEnd();

			return index != other.index;
		}

		EntityComponentIterator<Types...>& operator++();

	private:
		bool m_isEnd = false;
		size_t index;
		class Scene* m_scene;
		bool m_includePendingDestroy;
	};

	template<typename... Types>
	class EntityComponentView
	{
	public:
		EntityComponentView(const EntityComponentIterator<Types...>& first, const EntityComponentIterator<Types...>& last);

		const EntityComponentIterator<Types...>& begin() const
		{
			return firstItr;
		}

		const EntityComponentIterator<Types...>& end() const
		{
			return lastItr;
		}

	private:
		EntityComponentIterator<Types...> firstItr;
		EntityComponentIterator<Types...> lastItr;
	}; 

	inline EntityIterator::EntityIterator(class Scene* scene, size_t index, bool isEnd, bool includePendingDestroy)
		: m_isEnd(isEnd), index(index), m_scene(scene), m_includePendingDestroy(includePendingDestroy)
	{
		if (index >= m_scene->getEntityCount())
			this->m_isEnd = true;
	}

	inline bool EntityIterator::isEnd() const
	{
		return m_isEnd || index >= m_scene->getEntityCount();
	}

	inline Entity* EntityIterator::get() const
	{
		if (isEnd())
			return nullptr;

		return m_scene->getByIndex(index);
	}

	inline EntityIterator& EntityIterator::operator++()
	{
		++index;
		while (index < m_scene->getEntityCount() && (get() == nullptr || (get()->isPendingDestroy() && !m_includePendingDestroy)))
		{
			++index;
		}

		if (index >= m_scene->getEntityCount())
			m_isEnd = true;

		return *this;
	}

	template<typename... Types>
	EntityComponentIterator<Types...>::EntityComponentIterator(Scene* scene, size_t index, bool isEnd, bool includePendingDestroy)
		: m_isEnd(isEnd), index(index), m_scene(scene), m_includePendingDestroy(includePendingDestroy)
	{
		if (index >= m_scene->getEntityCount())
			this->m_isEnd = true;
	}

	template<typename... Types>
	bool EntityComponentIterator<Types...>::isEnd() const
	{
		return m_isEnd || index >= m_scene->getEntityCount();
	}

	template<typename... Types>
	Entity* EntityComponentIterator<Types...>::get() const
	{
		if (isEnd())
			return nullptr;

		return m_scene->getByIndex(index);
	}

	template<typename... Types>
	EntityComponentIterator<Types...>& EntityComponentIterator<Types...>::operator++()
	{
		++index;
		while (index < m_scene->getEntityCount() && (get() == nullptr || !get()->template has<Types...>() || (get()->isPendingDestroy() && !m_includePendingDestroy)))
		{
			++index;
		}

		if (index >= m_scene->getEntityCount())
			m_isEnd = true;

		return *this;
	}

	template<typename... Types>
	EntityComponentView<Types...>::EntityComponentView(const EntityComponentIterator<Types...>& first, const EntityComponentIterator<Types...>& last)
		: firstItr(first), lastItr(last)
	{
		if (firstItr.get() == nullptr || (firstItr.get()->isPendingDestroy() && !firstItr.includePendingDestroy())
			|| !firstItr.get()->template has<Types...>())
		{
			++firstItr;
		}
	}
	   
	class SceneManager : public Manager<SceneManager>
	{
	public:
		SceneManager(void) { m_scene = nullptr; };
		~SceneManager(void) {
			m_scene->destroy();
		};

		void cleanupScene()
		{
			m_scene->destroy();
		}

		Scene* getScene() { return m_scene; };

		Scene* setScene(Scene* scene) { m_scene = scene; return m_scene; };

		template<typename... Types>
		void each(typename std::common_type<std::function<void(Entity*, ComponentHandle<Types>...)>>::type viewFunc, bool includePendingDestroy = false);
		void all(std::function<void(Entity*)> viewFunc, bool includePendingDestroy = false);
		EntityView all(bool bIncludePendingDestroy);

		template<typename... Types>
		EntityComponentView<Types...> each(bool includePendingDestroy = false)
		{
			EntityComponentIterator<Types...> first(m_scene, 0, false, includePendingDestroy);
			EntityComponentIterator<Types...> last(m_scene, m_scene->getEntityCount(), true, includePendingDestroy);
			return EntityComponentView<Types...>(first, last);
		}


	private:
		Scene* m_scene;

	};
}