#pragma once
#include <sqlext.h>

void PrintSQLErrorMessage(SQLHSTMT stmtHandle);
bool SQLIsSuccess(SQLRETURN returnValue);