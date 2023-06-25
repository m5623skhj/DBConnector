#pragma once

#include "Type.h"

struct IResultType
{
	virtual void GetResultColumnProperty(OUT std::vector<std::pair<ProcedureName, ProcedureTypeName>>& propertyList) const = 0;
};

struct TestReulstType : public IResultType
{
	DEFINE_CLASS_INFO(TestReulstType);

	REGISTER_PROPERTY(id);
	REGISTER_PROPERTY(no);

public:
	int id = 0;
	int no = 0;

	virtual void GetResultColumnProperty(OUT std::vector<std::pair<ProcedureName, ProcedureTypeName>>& propertyList) const
	{
		GetTypeInfo().GetAllProperties(propertyList);
	}
};

struct DB_IgnoreType : public IResultType
{
	DEFINE_CLASS_INFO(DB_IgnoreType);

	virtual void GetResultColumnProperty(OUT std::vector<std::pair<ProcedureName, ProcedureTypeName>>& propertyList) const
	{
	}
};