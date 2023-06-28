#include "PreCompile.h"

#include "TypeInfo.h"
#include "Property.h"

#include <utility>

void TypeInfo::AddProperty(const Property& inProperty)
{
	std::string propertyName(inProperty.GetName());
	if (propertyMap.empty() == false)
	{
		if (propertyMap.find(propertyName) != propertyMap.end())
		{
			return;
		}
	}

	propertyMap.emplace(propertyName, inProperty);
	propertyTypeNameList.emplace_back(inProperty.GetTypeName());
}

void TypeInfo::GetAllProperties(OUT std::vector<std::pair<PropertyName, PropertyTypeName>>& propertyList) const
{
	for (const auto& it : propertyMap)
	{
		propertyList.emplace_back(std::make_pair(it.first, it.second.GetTypeName()));
	}
}

std::vector<PropertyTypeName> TypeInfo::GetAllPropertyTypeName() const
{
	return propertyTypeNameList;
}

size_t TypeInfo::GetNumOfProperty()
{
	return propertyMap.size();
}