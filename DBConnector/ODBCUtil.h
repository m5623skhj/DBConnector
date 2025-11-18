#pragma once
#include <sqlext.h>
#include <string>
#include "ODBCConst.h"
#include <unordered_map>
#include "Type.h"

namespace ODBCUtil
{
	namespace TypeTrait
	{
		template<typename T, typename Enable = void>
		struct ODBCTypeTraitor
		{
			static SQLSMALLINT GetCType() { return 0; }
			static SQLSMALLINT GetSQLType() { return 0; }
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

		template<>
		struct ODBCTypeTraitor<FString>
		{
			static SQLSMALLINT GetCType() { return SQL_CHAR; }
			static SQLSMALLINT GetSQLType() { return SQL_VARCHAR; }
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
				return static_cast<SQLPOINTER>(input);
			}
			else if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, FWString>)
			{
				return static_cast<SQLPOINTER>(input.c_str());
			}
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, FString>)
			{
				return static_cast<SQLPOINTER>(input.c_str());
			}
			else
			{
				return static_cast<SQLPOINTER>(&input);
			}
		}

		template<typename T>
		SQLULEN GetColumnSize(const T&)
		{
			if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, FWString>)
			{
				return SQL_STRING_LENGTH;
			}
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, FString>)
			{
				return SQL_STRING_LENGTH;
			}

			return sizeof(T);
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
				cTypeMap.insert({ "FString", SQL_CHAR });

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
				sqlTypeMap.insert({ "FString", SQL_VARCHAR });

				// columnSizeMap
				columnSizeMap.insert({ "bool", sizeof(bool) });
				columnSizeMap.insert({ "short", sizeof(short) });
				columnSizeMap.insert({ "int", sizeof(int) });
				columnSizeMap.insert({ "float", sizeof(float) });
				columnSizeMap.insert({ "double", sizeof(double) });
				columnSizeMap.insert({ "long long", sizeof(long long) });
				columnSizeMap.insert({ "__int64", sizeof(__int64) });
				columnSizeMap.insert({ "std::wstring", SQL_STRING_LENGTH });
				columnSizeMap.insert({ "WCHAR[]", SQL_STRING_LENGTH });
				columnSizeMap.insert({ "const WCHAR*", SQL_STRING_LENGTH });
				columnSizeMap.insert({ "FWString", SQL_STRING_LENGTH });
				columnSizeMap.insert({ "FString", SQL_STRING_LENGTH });
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
				const auto findItor = cTypeMap.find(typeString);
				if (findItor == cTypeMap.end())
				{
					return 0;
				}

				return findItor->second;
			}

			SQLSMALLINT GetSQLType(const std::string& typeString)
			{
				const auto findItor = sqlTypeMap.find(typeString);
				if (findItor == sqlTypeMap.end())
				{
					return 0;
				}

				return findItor->second;
			}

			SQLLEN GetBufferSize(const std::string& typeString)
			{
				const auto findItor = columnSizeMap.find(typeString);
				if (findItor == columnSizeMap.end())
				{
					return 0;
				}

				return findItor->second;
			}

			static SQLPOINTER GetPointerFromPointerTypeString(const std::string& typeString, const void* input)
			{
				if (typeString == "FWString")
				{
					return (SQLPOINTER)(((FWString*)input)->GetCString());
				}
				else if (typeString == "FString")
				{
					return (SQLPOINTER)(((FString*)input)->GetCString());
				}

				return (SQLPOINTER)input;
			}

		private:
			std::unordered_map<std::string, SQLSMALLINT> cTypeMap;
			std::unordered_map<std::string, SQLSMALLINT> sqlTypeMap;
			std::unordered_map<std::string, SQLLEN> columnSizeMap;
		};
	}

	void PrintSQLErrorMessage(const SQLHSTMT stmtHandle);
	bool SQLIsSuccess(const SQLRETURN returnValue);
	bool IsSameType(const std::string& lhs, const std::string& rhs);

	std::wstring GetDataTypeName(const SQLSMALLINT inDataType);

	template <typename T>
	bool SettingSPMaker(const SQLHSTMT stmtHandle, const int parameterLocation, SQLSMALLINT cTypeData, SQLSMALLINT sqlTypeData, const T& input)
	{
		if (const SQLPOINTER inputPointer = TypeTrait::GetPointerFromT(input); ODBCUtil::SQLIsSuccess(SQLBindParameter(
			stmtHandle
			, parameterLocation
			, SQL_PARAM_INPUT
			, TypeTrait::GetCType(input)
			, TypeTrait::GetSQLType(input)
			, 0
			, 0
			, inputPointer
			, 0
			, nullptr)) == false)
		{
			PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}

	bool DBSendQuery(const std::wstring& query, SQLHSTMT& stmtHandle);
	bool DBSendQueryDirect(const std::wstring& query, SQLHSTMT& stmtHandle);
	bool DBSendQueryWithPrepare(const std::wstring& query, SQLHSTMT& stmtHandle);
}
