#pragma once
#include "BuildConfg.h"
#include "Reflection.h"
#include <map>
#include <typeinfo>
#include <typeindex>
#include "Type.h"
#include "ProcedureType.h"

class IStoredProcedure
{
	DEFINE_CLASS_INFO(IStoredProcedure)

public:
	virtual ~IStoredProcedure() {}
};

class test : public IStoredProcedure
{
	DEFINE_CLASS_INFO(test);

	REGISTER_PROPERTY(_id);
	REGISTER_PROPERTY(_id2);
	REGISTER_PROPERTY(_id3);
	REGISTER_PROPERTY(test);

public:
	virtual ~test() {}

public:
	int _id = 0;
	int _id2 = 0;
	int _id3 = 0;
	FString test;

	using ResultType = TestReulstType;
};

#if UNIT_TEST
	#define INPUT_TEST_PROCEDURE_MAP(TestProcedureMap, ResultPropertyMap, Procedure)\
	{\
		TestProcedureMap.emplace(Procedure::StaticTypeInfo().GetName(), std::make_shared<Procedure>());\
		ResultPropertyMap.emplace(Procedure::StaticTypeInfo().GetName(), std::vector<std::pair<ProcedureName, ProcedureTypeName>>());\
		{\
			auto it = ResultPropertyMap.find(Procedure::StaticTypeInfo().GetName());\
			if (it != ResultPropertyMap.end())\
			{\
				Procedure::ResultType::StaticTypeInfo().GetAllProperties(it->second);\
			}\
		}\
	}
	
	#define PROCEDURE_TEST_LIST(TestProcedureMap, ResultPropertyMap){\
		INPUT_TEST_PROCEDURE_MAP(TestProcedureMap, ResultPropertyMap, test)\
	}
#endif