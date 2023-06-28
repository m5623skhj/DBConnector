#pragma once

#include "Type.h"

#define INPUT_REAL_POINTER(InputValue)\
{\
	realPointerList.emplace_back(&InputValue);\
}

struct IResultType
{
public:
	std::vector<void*> realPointerList;
};

struct DB_IgnoreType : public IResultType
{
	DEFINE_CLASS_INFO(DB_IgnoreType);
};

struct TestReulstType : public IResultType
{
	DEFINE_CLASS_INFO(TestReulstType);

	REGISTER_PROPERTY(id);
	REGISTER_PROPERTY(no);

public:
	TestReulstType()
	{
		INPUT_REAL_POINTER(id);
		INPUT_REAL_POINTER(no);
	}
	int id = 0;
	int no = 0;
};

struct FWStringResultType : public IResultType
{
	DEFINE_CLASS_INFO(FWStringResultType);

	REGISTER_PROPERTY(string);

public:
	FWStringResultType()
	{
		INPUT_REAL_POINTER(string);
	}
	FWString string;
};

struct SelectTest2ResultType : public IResultType
{
	DEFINE_CLASS_INFO(SelectTest2ResultType);

	REGISTER_PROPERTY(no);
	REGISTER_PROPERTY(tablename);

public:
	SelectTest2ResultType()
	{
		INPUT_REAL_POINTER(no);
		INPUT_REAL_POINTER(tablename);
	}
	int no = 0;
	FWString tablename;
};
