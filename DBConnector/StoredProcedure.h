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

	REGISTER_PROPERTY(id3);
	REGISTER_PROPERTY(teststring);

public:
	virtual ~test() {}

public:
	int id3 = 0;
	FWString teststring;

	using ResultType = DB_IgnoreType;
};

class update_test : public IStoredProcedure
{
	DEFINE_CLASS_INFO(update_test);

	REGISTER_PROPERTY(_id);

public:
	virtual ~update_test() {}

public:
	long long _id = 0;

	using ResultType = DB_IgnoreType;
};

class string_test_proc : public IStoredProcedure
{
	DEFINE_CLASS_INFO(string_test_proc);

	REGISTER_PROPERTY(test);

public:
	FWString test;

	using ResultType = DB_IgnoreType;
};

class input_test : public IStoredProcedure
{
	DEFINE_CLASS_INFO(input_test);

	REGISTER_PROPERTY(item);
	REGISTER_PROPERTY(item2);

public:
	int item;
	int item2;

	using ResultType = DB_IgnoreType;
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
		INPUT_TEST_PROCEDURE_MAP(TestProcedureMap, ResultPropertyMap, update_test)\
		INPUT_TEST_PROCEDURE_MAP(TestProcedureMap, ResultPropertyMap, string_test_proc)\
	}
#endif