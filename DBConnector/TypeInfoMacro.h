#pragma once
#include "TypeInfo.h"

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
private:\

