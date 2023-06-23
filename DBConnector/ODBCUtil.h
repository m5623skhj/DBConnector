#pragma once
#include <sqlext.h>
#include "StoredProcedure.h"
#include <string>

namespace ODBCUtil
{
	namespace TypeTrait
	{
		template<typename T, typename Enable = void>
		struct ODBCTypeTraitor
		{
		};

		template<>
		struct ODBCTypeTraitor<bool>
		{
			static SQLSMALLINT GetCType() { return SQL_C_BIT; }
			static SQLSMALLINT GetSQLType() { return SQL_BIT; }
		};

		template<>
		struct ODBCTypeTraitor<short>
		{
			static SQLSMALLINT GetCType() { return SQL_C_SSHORT; }
			static SQLSMALLINT GetSQLType() { return SQL_IS_SMALLINT; }
		};

		template<>
		struct ODBCTypeTraitor<int>
		{
			static SQLSMALLINT GetCType() { return SQL_C_SLONG; }
			static SQLSMALLINT GetSQLType() { return SQL_INTEGER; }
		};

		template<>
		struct ODBCTypeTraitor<float>
		{
			static SQLSMALLINT GetCType() { return SQL_C_FLOAT; }
			static SQLSMALLINT GetSQLType() { return SQL_REAL; }
		};

		template<>
		struct ODBCTypeTraitor<double>
		{
			static SQLSMALLINT GetCType() { return SQL_C_DOUBLE; }
			static SQLSMALLINT GetSQLType() { return SQL_DOUBLE; }
		};

		template<>
		struct ODBCTypeTraitor<long long>
		{
			static SQLSMALLINT GetCType() { return SQL_C_SBIGINT; }
			static SQLSMALLINT GetSQLType() { return SQL_BIGINT; }
		};

		template<>
		struct ODBCTypeTraitor<std::wstring>
		{
			static SQLSMALLINT GetCType() { return SQL_WCHAR; }
			static SQLSMALLINT GetSQLType() { return SQL_WVARCHAR; }
		};

		template<>
		struct ODBCTypeTraitor<WCHAR[]>
		{
			static SQLSMALLINT GetCType() { return SQL_WCHAR; }
			static SQLSMALLINT GetSQLType() { return SQL_WVARCHAR; }
		};

		template<>
		struct ODBCTypeTraitor<const WCHAR*>
		{
			static SQLSMALLINT GetCType() { return SQL_WCHAR; }
			static SQLSMALLINT GetSQLType() { return SQL_WVARCHAR; }
		};

		template<typename T>
		SQLSMALLINT GetCType(const T&)
		{
			return ODBCTypeTraitor<T>::GetCType();
		}

		template<typename T>
		SQLSMALLINT GetSQLType(const T&)
		{
			return ODBCTypeTraitor<T>::GetSQLType();
		}

		template<typename T>
		static SQLPOINTER GetPointerFromT(const T& input)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				return (SQLPOINTER)input;
			}
			else if constexpr (std::is_same_v<T, std::wstring>)
			{
				return (SQLPOINTER)input.c_str();
			}
			else
			{
				return (SQLPOINTER)&input;
			}
		}
	}

	void PrintSQLErrorMessage(SQLHSTMT stmtHandle);
	bool SQLIsSuccess(SQLRETURN returnValue);
	bool IsSameType(const std::string& lhs, const std::string& rhs);

	std::wstring GetDataTypeName(SQLSMALLINT inDataType);

	template <typename T>
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, SQLSMALLINT cTypeData, SQLSMALLINT sqlTypeData, const T& input)
	{
		SQLPOINTER inputPointer = TypeTrait::GetPointerFromT(input);

		// string 타입이면 주소로 넣을 수 없는데
		if (ODBCUtil::SQLIsSuccess(SQLBindParameter(stmtHandle, parameterLocation, SQL_PARAM_INPUT, TypeTrait::GetCType(input), TypeTrait::GetSQLType(input)
			, 0, 0, inputPointer, 0, NULL)) == false)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}

	bool DBSendQuery(const std::wstring& query, SQLHSTMT& stmtHandle);
	bool DBSendQueryDirect(const std::wstring& query, SQLHSTMT& stmtHandle);
	bool DBSendQueryWithPrepare(const std::wstring& query, SQLHSTMT& stmtHandle);

	void GetDBResult(SQLHSTMT& stmtHandle);
	void GetDBResult(SQLHSTMT& stmtHandle, test::ResultType& t);
}
