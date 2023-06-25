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

		template<>
		struct ODBCTypeTraitor<FWString>
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
		SQLPOINTER GetPointerFromT(const T& input)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				return (SQLPOINTER)input;
			}
			else if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, FWString>)
			{
				return (SQLPOINTER)input.c_str();
			}
			else
			{
				return (SQLPOINTER)&input;
			}
		}

		template<typename T>
		SQLULEN GetColumnSize(const T&)
		{
			if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, FWString>)
			{
				return 0;
			}

			return (SQLULEN)sizeof(T);
		}

		class SQLTypeGetterFromString
		{
		private:
			SQLTypeGetterFromString()
			{
				// cTypeMap
				cTypeMap.insert({ "bool", SQL_C_BIT });
				cTypeMap.insert({ "short", SQL_C_SSHORT });
				cTypeMap.insert({ "int", SQL_C_SLONG });
				cTypeMap.insert({ "float", SQL_C_FLOAT });
				cTypeMap.insert({ "double", SQL_C_DOUBLE });
				cTypeMap.insert({ "long long", SQL_C_SBIGINT });
				cTypeMap.insert({ "__int64", SQL_C_SBIGINT });
				cTypeMap.insert({ "std::wstring", SQL_WCHAR });
				cTypeMap.insert({ "WCHAR[]", SQL_WCHAR });
				cTypeMap.insert({ "const WCHAR*", SQL_WCHAR });
				cTypeMap.insert({ "FWString", SQL_WCHAR });

				// sqlTypeMap
				sqlTypeMap.insert({ "bool", SQL_BIT });
				sqlTypeMap.insert({ "short", SQL_IS_SMALLINT });
				sqlTypeMap.insert({ "int", SQL_INTEGER });
				sqlTypeMap.insert({ "float", SQL_REAL });
				sqlTypeMap.insert({ "double", SQL_DOUBLE });
				sqlTypeMap.insert({ "long long", SQL_BIGINT });
				sqlTypeMap.insert({ "__int64", SQL_BIGINT });
				sqlTypeMap.insert({ "std::wstring", SQL_WVARCHAR });
				sqlTypeMap.insert({ "WCHAR[]", SQL_WVARCHAR });
				sqlTypeMap.insert({ "const WCHAR*", SQL_WVARCHAR });
				sqlTypeMap.insert({ "FWString", SQL_WVARCHAR });
			}
			~SQLTypeGetterFromString() = default;

		public:
			static SQLTypeGetterFromString& GetInst()
			{
				static SQLTypeGetterFromString instance;
				return instance;
			}

		public:
			SQLSMALLINT GetCType(const std::string& typeString)
			{
				auto findIter = cTypeMap.find(typeString);
				if (findIter == cTypeMap.end())
				{
					return 0;
				}

				return findIter->second;
			}

			SQLSMALLINT GetSQLType(const std::string& typeString)
			{
				auto findIter = sqlTypeMap.find(typeString);
				if (findIter == sqlTypeMap.end())
				{
					return 0;
				}

				return findIter->second;
			}

			SQLPOINTER GetPointerFromPointerTypeString(const std::string& typeString, const void* input)
			{
				if (typeString == "FWString")
				{
					return (SQLPOINTER)(((FWString*)input)->GetCString());
				}

				return (SQLPOINTER)input;
			}

		private:
			std::map<std::string, SQLSMALLINT> cTypeMap;
			std::map<std::string, SQLSMALLINT> sqlTypeMap;
		};
	}

	void PrintSQLErrorMessage(SQLHSTMT stmtHandle);
	bool SQLIsSuccess(SQLRETURN returnValue);
	bool IsSameType(const std::string& lhs, const std::string& rhs);

	std::wstring GetDataTypeName(SQLSMALLINT inDataType);

	template <typename T>
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, SQLSMALLINT cTypeData, SQLSMALLINT sqlTypeData, const T& input)
	{
		SQLPOINTER inputPointer = TypeTrait::GetPointerFromT(input);

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
}
