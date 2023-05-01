#pragma once

#include "TypeInfoMacro.h"
#include "TypeInfo.h"

#include <string>

class Property
{
public:
	const PropertyName& GetName() const
	{
		return name;
	}

	const PropertyTypeName& GetTypeName() const
	{
		return typeName;
	}

	template <typename T>
	struct ReturnValueWrapper
	{
	public:
		explicit ReturnValueWrapper(T& value) :
			value(&value) {}
		ReturnValueWrapper() = default;

		ReturnValueWrapper& operator=(const T& value)
		{
			*value = value;
			return *this;
		}

		operator T& ()
		{
			return *value;
		}

	private:
		T* value = nullptr;
	};

	const TypeInfo& GetTypeInfo() const
	{
		return type;
	}

	Property(const char* inName, const PropertyTypeName& inTypeName, TypeInfo& owner)
		: name(inName)
		, typeName(inTypeName)
		, type(owner)
	{
		owner.AddProperty(*this);
	}

private:
	const PropertyName name;
	const PropertyTypeName typeName;
	const TypeInfo& type;
};

template <typename T>
class PropertyRegister
{
public:
	PropertyRegister(const char* inName, const char* inTypeName, TypeInfo& inTypeInfo)
	{
		const char* classPrefix = "class ";

		std::string typeName(inTypeName);
		auto prefixPos = typeName.find(classPrefix);
		if (prefixPos != std::string::npos)
		{
			typeName.erase(prefixPos, strlen(classPrefix));
		}

		Property property(inName, typeName, inTypeInfo);
		GetProperties().emplace_back(std::move(property));
	}

private:
	static std::vector<Property>& GetProperties()
	{
		static std::vector<Property> properties;
		return properties;
	}
};
