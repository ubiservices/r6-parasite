#pragma once

#include "../../include.h"

#include "../Player/Entity.h"

namespace scimitar
{	
	// Temporary Caching solution (lusion make it better)
	// as we will kill performance when reading through
	// a list with 15k+ entities every frame!.

	class Cache
	{
	public:
		inline static ubiArray<Entity*> m_CachedEntities;
		inline static void RunUpdate();
		inline static void Clear();
	};
}