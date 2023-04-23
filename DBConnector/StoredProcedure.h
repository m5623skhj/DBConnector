#pragma once

class IStoredProcedure
{
public:
	virtual ~IStoredProcedure() {}
	virtual std::wstring GetClassName() const = 0;
};

class test : IStoredProcedure
{
public:
	virtual ~test() {}
	virtual std::wstring GetClassName() const { return L"test"; }

public:
	int _id = 0;
	int _id2 = 0;
	int _id3 = 0;
};