#include "SceneManager.h"

namespace VEngine {

	template<typename... Types>
	void SceneManager::each(typename std::common_type<std::function<void(Entity*, ComponentHandle<Types>...)>>::type viewFunc, bool includePendingDestroy)
	{
		for (auto* ent : each<Types...>(includePendingDestroy))
		{
			viewFunc(ent, ent->template get<Types>()...);
		}
	}

	EntityView SceneManager::all(bool bIncludePendingDestroy)
	{
		EntityIterator first(m_scene, 0, false, bIncludePendingDestroy);
		EntityIterator last(m_scene, m_scene->getEntityCount(), true, bIncludePendingDestroy);
		return EntityView(first, last);
	}

	void SceneManager::all(std::function<void(Entity*)> viewFunc, bool includePendingDestroy)
	{
		for (auto* ent : all(includePendingDestroy))
		{
			viewFunc(ent);
		}
	}


}
