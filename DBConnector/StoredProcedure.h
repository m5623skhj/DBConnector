#pragma once
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

public:
	virtual ~test() {}

public:
	int _id = 0;
	int _id2 = 0;
	int _id3 = 0;
};

class test2 : public IStoredProcedure
{
	DEFINE_CLASS_INFO(test2);

	REGISTER_PROPERTY(_id);
	REGISTER_PROPERTY(_testString);

public:
	virtual ~test2() {}

public:
	float _id = 0.f;
	FString _testString;
};
