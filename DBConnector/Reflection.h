#pragma once
#include <concepts>
#include <string>

// https://www.slideshare.net/xtozero/c20-251161090

class TypeInfo;

template <typename T>
concept HasSuper = requires
{
	T::Super;
} && std::same_as<typename T::Super, void> == false;

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
	using Type = T::ThisType;
};

template <typename T>
struct TypeInfoInitializer
{
	TypeInfoInitializer(const char* name)
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

	const TypeInfo* GetSuper() const
	{
		return super;
	}

	template <typename T> requires HasStaticTypeInfo<T>
	static const TypeInfo& GetStaticTypeInfo()
	{
		return T::StaticTypeInfo();
	}

	bool IsA(const TypeInfo& inItem) const
	{
		if (this == &inItem)
		{
			return true;
		}

		return hashCode == inItem.hashCode;
	}

	bool IsChildOf(const TypeInfo& inItem)
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
	bool IsA() const
	{
		return IsA(GetStaticTypeInfo<T>());
	}

	template <typename T>
	bool IsChildOf() const
	{
		return IsChildOf(GetStaticTypeInfo<T>());
	}

	const char* GetName() const
	{
		return name;
	}

private:
	size_t hashCode;
	const char* name = nullptr;
	std::string fullName;
	const TypeInfo* super = nullptr;
};

#define DEFINE_CLASS_INFO(TypeName)\
private:\
	friend SuperClassTypeDeduction;\
	friend TypeInfoInitializer;\
\
public:\
	using Super = typename SuperClassTypeDeduction<TypeName>::Type;\
	using ThisType = TypeName;\
\
	static TypeInfo& StaticTypeInfo()\
	{\
		static TypeInfo typeInfo{ TypeInfoInitializer<ThisType>(#TypeName) };\
		return typeInfo;\
	}\
\
	virtual const TypeInfo& GetTypeInfo() const\
	{\
		return typeInfo;\
	}\
\
private:\
	inline static TypeInfo& typeInfo = StaticTypeInfo();\
\
private:\
