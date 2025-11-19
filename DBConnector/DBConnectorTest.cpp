#include "PreCompile.h"

#include "ODBCConnector.h"
#include "StoredProcedure.h"
#include "ODBCMetaData.h"
#include "ODBCUtil.h"
#include <utility>
#include <vector>
#include <gtest/gtest.h>

#if UNIT_TEST
TEST(DBConnectorTest, DBInitializeTest)
{
	ODBCConnector& connector = ODBCConnector::GetInst();

	ASSERT_EQ(connector.ConnectDB(L"OptionFile/DBConnectFile.txt"), true);
	EXPECT_EQ(connector.InitDB(), true);

	connector.DisconnectDB();
}

TEST(DBConnectorTest, ProcedureParameterTest)
{
	std::map<std::string, std::shared_ptr<SP::IStoredProcedure>> testProcedureMap;
	std::map<std::string, std::vector<std::pair<std::string, ProcedureTypeName>>> resultPropertyMap;

	PROCEDURE_TEST_LIST(testProcedureMap, resultPropertyMap)

	ODBCConnector& connector = ODBCConnector::GetInst();

	ASSERT_EQ(connector.ConnectDB(L"OptionFile/DBConnectFile.txt"), true);
	EXPECT_EQ(connector.InitDB(), true);

	std::vector<std::string> notMatchedProcedureList;
	auto columnMatch = [](
		const std::vector<std::pair<std::string, ProcedureTypeName>>& cppProperties
		, const std::vector<std::pair<std::string, ProcedureTypeName>>& dbProperties) -> bool
	{
		if (cppProperties.size() != dbProperties.size())
		{
			return false;
		}

		for (size_t i = 0; i < cppProperties.size(); ++i)
		{
			// 컬럼의 타입과 순서만 검사
			if (cppProperties[i].second != dbProperties[i].second)
			{
				if (ODBCUtil::IsSameType(cppProperties[i].second, dbProperties[i].second) == false)
				{
					return false;
				}
			}
		}

		return true;
	};

	for (const auto& testProcedure : testProcedureMap)
	{
		ASSERT_NE(testProcedure.second, nullptr);
	
		std::vector<std::pair<std::string, ProcedureTypeName>> cppProperties;
		testProcedure.second->GetTypeInfo().GetAllProperties(cppProperties);

		auto matchedProcedureInfo = connector.GetProcedureInfo(testProcedure.first);
		ASSERT_NE(matchedProcedureInfo, nullptr);
		if (matchedProcedureInfo == nullptr)
		{
			const std::string notMatchedInfo = "Procedure was not found in DBConnector. ProcedureName[" + testProcedure.first + "]\n";
			notMatchedProcedureList.emplace_back(notMatchedInfo);
			continue;
		}

		auto matchedProcedureResultColumnInfo = resultPropertyMap.find(testProcedure.first);
		ASSERT_NE(matchedProcedureResultColumnInfo, resultPropertyMap.end());
		if (matchedProcedureResultColumnInfo == resultPropertyMap.end())
		{
			const std::string notMatchedInfo = "Procedure result was not found in resultPropertyMap. ProcedureName[" + testProcedure.first + "]\n";
			notMatchedProcedureList.emplace_back(notMatchedInfo);
			continue;
		}

		std::vector<std::pair<std::string, ProcedureTypeName>> dbInputProperties;
		char utf8Name[256], utf8DataTypeName[256];
		for (const auto& inputColumn : matchedProcedureInfo->inputColumnInfoList)
		{
			UTF16ToUTF8(inputColumn.name.c_str(), utf8Name);
			UTF16ToUTF8(inputColumn.dataTypeName.c_str(), utf8DataTypeName);
			dbInputProperties.emplace_back(utf8Name, utf8DataTypeName);
		}
		
		std::vector<std::pair<std::string, ProcedureTypeName>> dbResultProperties;
		for (const auto& resultColumn : matchedProcedureInfo->resultColumnInfoList)
		{
			UTF16ToUTF8(resultColumn.name.c_str(), utf8Name);
			UTF16ToUTF8(resultColumn.dataTypeName.c_str(), utf8DataTypeName);
			dbResultProperties.emplace_back(utf8Name, utf8DataTypeName);
		}

		bool isMatched = true;
		isMatched = columnMatch(cppProperties, dbInputProperties) && isMatched;
		isMatched = columnMatch(matchedProcedureResultColumnInfo->second, dbResultProperties) && isMatched;
		if (isMatched == false)
		{
			const std::string notMatchedInfo = "Procedure parameter not matched. ProcedureName[" + testProcedure.first + "]\n";
			notMatchedProcedureList.emplace_back(notMatchedInfo);
		}
		EXPECT_TRUE(isMatched);
	}

	if (!notMatchedProcedureList.empty())
	{
		std::cout << '\n';
		std::cout << "---------------------------------------" << '\n';
		for (const auto& notMatchedProcedure : notMatchedProcedureList)
		{
			std::cout << notMatchedProcedure;
		}
		std::cout << "---------------------------------------" << '\n' << '\n' << '\n';
	}
	connector.DisconnectDB();
}
#endif