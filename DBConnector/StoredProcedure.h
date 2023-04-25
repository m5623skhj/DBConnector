#pragma once
#include "Reflection.h"

class IStoredProcedure
{
	DEFINE_CLASS_INFO(IStoredProcedure)

public:
	virtual ~IStoredProcedure() {}
};

class test : IStoredProcedure
{
	DEFINE_CLASS_INFO(test)

public:
	virtual ~test() {}

public:
	int _id = 0;
	int _id2 = 0;
	int _id3 = 0;
};