#pragma once
#include "BuildConfg.h"
#include "Reflection.h"
#include <map>
#include <typeinfo>
#include <typeindex>
#include "Type.h"
#include "ProcedureType.h"

#define INPUT_REAL_POINTER(InputValue)\
{\
	realPointerList.emplace_back(&InputValue);\
}

class IStoredProcedure
{
	DEFINE_CLASS_INFO(IStoredProcedure);

public:
	virtual ~IStoredProcedure() {}

public:
	std::vector<void*> realPointerList;
};

class test : public IStoredProcedure
{
	DEFINE_CLASS_INFO(test);

	REGISTER_PROPERTY(id3);
	REGISTER_PROPERTY(teststring);

public:
	test()
	{
		INPUT_REAL_POINTER(id3);
		INPUT_REAL_POINTER(teststring);
	}
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
	update_test()
	{
		INPUT_REAL_POINTER(_id);
	}
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
	string_test_proc()
	{
		INPUT_REAL_POINTER(test);
	}
	virtual ~string_test_proc() {}

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
	input_test()
	{
		INPUT_REAL_POINTER(item);
		INPUT_REAL_POINTER(item2);
	}
	virtual ~input_test() {}

public:
	int item;
	int item2;

	using ResultType = DB_IgnoreType;
};

class SELECT_TEST : public IStoredProcedure
{
	DEFINE_CLASS_INFO(SELECT_TEST);

	REGISTER_PROPERTY(id);

public:
	SELECT_TEST()
	{
		INPUT_REAL_POINTER(id);
	}
	long long id = 0;

	using ResultType = FWStringResultType;
};

class SELECT_TEST_2 : public IStoredProcedure
{
	DEFINE_CLASS_INFO(SELECT_TEST_2);

	REGISTER_PROPERTY(id);

public:
	SELECT_TEST_2()
	{
		INPUT_REAL_POINTER(id);
	}
	long long id = 0;

	using ResultType = SelectTest2ResultType;
};

class SELECT_TEST_3 : public IStoredProcedure
{
	DEFINE_CLASS_INFO(SELECT_TEST_3);


public:
	SELECT_TEST_3()
	{
	}

	using ResultType = SelectTest2ResultType;
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
		INPUT_TEST_PROCEDURE_MAP(TestProcedureMap, ResultPropertyMap, SELECT_TEST)\
		INPUT_TEST_PROCEDURE_MAP(TestProcedureMap, ResultPropertyMap, SELECT_TEST_2)\
		INPUT_TEST_PROCEDURE_MAP(TestProcedureMap, ResultPropertyMap, SELECT_TEST_3)\
	}
#endif

//INPUT_TEST_PROCEDURE_MAP(TestProcedureMap, ResultPropertyMap, SELECT_TEST)\
