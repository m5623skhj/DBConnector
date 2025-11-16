#pragma once

#include <concepts>
#include <string>
#include <map>
#include <typeinfo>
#include <vector>

using PropertyName = std::string;
using PropertyTypeName = std::string;

class TypeInfo;
class Property;

template <typename T>
concept HasSuper = requires
{
	typename T::Super;
}&& std::derived_from<T, typename T::Super> == true;

template <typename T>
concept HasStaticTypeInfo = requires
{
	T::StaticTypeInfo();
};

template <typename T, typename U = void>
struct SuperClassTypeDeduction
{
	using Type = void;
};

template <typename T>
struct SuperClassTypeDeduction<T, std::void_t<typename T::ThisType>>
{
	using Type = typename T::ThisType;
};

template <typename T>
struct TypeInfoInitializer
{
	explicit TypeInfoInitializer(const char* name)
		: name(name)
	{
		if constexpr (HasSuper<T>)
		{
			super = &(typename T::Super::StaticTypeInfo());
		}
	}

	const char* name = nullptr;
	const TypeInfo* super = nullptr;
};

class TypeInfo
{
public:
	template <typename T>
	explicit TypeInfo(const TypeInfoInitializer<T>& initializer)
		: hashCode(typeid(T).hash_code())
		, name(initializer.name)
		, fullName(typeid(T).name())
		, super(initializer.super)
	{
	}

	[[nodiscard]]
	const TypeInfo* GetSuper() const
	{
		return super;
	}

	template <typename T> requires HasStaticTypeInfo<T>
	static const TypeInfo& GetStaticTypeInfo()
	{
		return T::StaticTypeInfo();
	}

	[[nodiscard]]
	bool IsA(const TypeInfo& inItem) const
	{
		if (this == &inItem)
		{
			return true;
		}

		return hashCode == inItem.hashCode;
	}

	[[nodiscard]]
	bool IsChildOf(const TypeInfo& inItem) const
	{
		if (IsA(inItem) == true)
		{
			return true;
		}

		for (const TypeInfo* nowSuper = super; nowSuper != nullptr; nowSuper = nowSuper->GetSuper())
		{
			if (nowSuper->IsA(inItem) == true)
			{
				return true;
			}
		}

		return false;
	}

	template <typename T>
	[[nodiscard]]
	bool IsA() const
	{
		return IsA(GetStaticTypeInfo<T>());
	}

	template <typename T>
	[[nodiscard]]
	bool IsChildOf() const
	{
		return IsChildOf(GetStaticTypeInfo<T>());
	}

	[[nodiscard]]
	const char* GetName() const
	{
		return name;
	}

	void AddProperty(const Property& inProperty);
	void GetAllProperties(OUT std::vector<std::pair<PropertyName, PropertyTypeName>>& propertyList) const;
	[[nodiscard]]
	std::vector<PropertyTypeName> GetAllPropertyTypeName() const;

	[[nodiscard]]
	size_t GetNumOfProperty() const;

private:
	using PropertyMap = std::map<PropertyName, Property>;

	size_t hashCode;
	const char* name = nullptr;
	std::string fullName;
	const TypeInfo* super = nullptr;

	PropertyMap propertyMap;
	std::vector<PropertyTypeName> propertyTypeNameList;
};

class Property
{
public:
	[[nodiscard]]
	const PropertyName& GetName() const
	{
		return name;
	}

	[[nodiscard]]
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

		ReturnValueWrapper& operator=(const T& value);

		explicit operator T& ()
		{
			return *value;
		}

	private:
		T* value = nullptr;
	};

	[[nodiscard]]
	const TypeInfo& GetTypeInfo() const
	{
		return type;
	}

	Property(const char* inName, PropertyTypeName inTypeName, TypeInfo& owner)
		: name(inName)
		, typeName(std::move(inTypeName))
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
Property::ReturnValueWrapper<T>& Property::ReturnValueWrapper<T>::operator=(const T& value)
{
	*value = value;
	return *this;
}

template <typename T>
class PropertyRegister
{
public:
	PropertyRegister(const char* inName, const char* inTypeName, TypeInfo& inTypeInfo)
	{
		const auto classPrefix = "class ";

		std::string typeName(inTypeName);
		if (const auto prefixPos = typeName.find(classPrefix); prefixPos != std::string::npos)
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
