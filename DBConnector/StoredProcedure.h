#pragma once
#include "BuildConfg.h"
#include "Reflection.h"
#include <map>
#include <typeinfo>
#include <typeindex>
#include "Type.h"

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
};

#if UNIT_TEST
	#define INPUT_TEST_PROCEDURE_MAP(Procedure){\
		testProcedureMap.emplace(Procedure::StaticTypeInfo().GetName(), std::make_shared<Procedure>());\
	}
	
	#define PROCEDURE_TEST_LIST(){\
		INPUT_TEST_PROCEDURE_MAP(test)\
	}

	static std::map<std::string, std::shared_ptr<IStoredProcedure>> testProcedureMap;
#endif