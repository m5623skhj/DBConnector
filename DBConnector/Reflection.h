#pragma once

// https://www.slideshare.net/xtozero/c20-251161090

template<typename To, typename From>
To* Cast(From* source)
{
	if (source == nullptr)
	{
		return nullptr;
	}

	if (source->GetTypeInfo().template IsChildOf<To>() == false)
	{
		return nullptr;
	}

	return reinterpret_cast<To*>(source);
}
