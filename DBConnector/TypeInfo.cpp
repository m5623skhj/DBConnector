#include "PreCompile.h"

#include "TypeInfo.h"
#include "Property.h"

#include <utility>

void TypeInfo::AddProperty(const Property& inProperty)
{
	std::string propertyName(inProperty.GetName());
	if (propertyMap.find(propertyName) != propertyMap.end())
	{
		return;
	}

	propertyMap.emplace(propertyName, inProperty);
}

void TypeInfo::GetAllProperties(OUT std::vector<std::pair<std::string, std::string>>& propertyList) const
{
	for (const auto& it : propertyMap)
	{
		propertyList.emplace_back(std::make_pair(it.first, it.second.GetTypeName()));
	}
}
