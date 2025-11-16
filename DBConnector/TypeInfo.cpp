#include "PreCompile.h"

#include "Property.h"

#include <utility>

void TypeInfo::AddProperty(const Property& inProperty)
{
	std::string propertyName(inProperty.GetName());
	if (propertyMap.contains(propertyName))
	{
		return;
	}

	propertyMap.emplace(propertyName, inProperty);
	propertyTypeNameList.emplace_back(inProperty.GetTypeName());
}

void TypeInfo::GetAllProperties(OUT std::vector<std::pair<PropertyName, PropertyTypeName>>& propertyList) const
{
	for (const auto& [propertyName, propertyTypeName] : propertyMap)
	{
		propertyList.emplace_back(propertyName, propertyTypeName.GetTypeName());
	}
}

std::vector<PropertyTypeName> TypeInfo::GetAllPropertyTypeName() const
{
	return propertyTypeNameList;
}

size_t TypeInfo::GetNumOfProperty() const
{
	return propertyMap.size();
}
