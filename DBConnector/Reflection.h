#pragma once

#include "PropertyMacro.h"
#include "TypeInfoMacro.h"
#include "TypeInfo.h"

// https://www.slideshare.net/xtozero/c20-251161090

template<typename To, typename From>
To* Cast(From* source)
{
	if (source == nullptr)
	{
		return nullptr;
	}

	if (source->GetTypeInfo().IsChildOf<To>() == false)
	{
		return nullptr;
	}

	return reinterpret_cast<To*>(source);
}
