#pragma once
// https://www.ibm.com/docs/ko/i/7.3?topic=functions-sqlprocedurecolumns-get-inputoutput-parameter-information-procedure

#define SP_PARAMETER_LOCATION 1
#define SQL_STRING_LENGTH 128

enum COLUMN_NUMBER
{
	PROCEDURE_NAME = 3u,
	COLUMN_NAME = 4u,
	COLUMN_TYPE = 5u,
	DATA_TYPE = 6u,
	DATA_TYPE_NAME = 7u,
	COLUMN_SIZE = 8u,
};
