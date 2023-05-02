#include "PreCompile.h"

#include "GoogleTest.h"
#include "ODBCConnector.h"
#include "StoredProcedure.h"
#include "ODBCMetaData.h"
#include "ODBCUtil.h"
#include <utility>
#include <vector>

#if UNIT_TEST
TEST(DBConnectorTest, DBInitializeTest)
{
	ODBCConnector connector;

	ASSERT_EQ(connector.ConnectDB(L"OptionFile/DBConnectFile.txt"), true);
	EXPECT_EQ(connector.InitDB(), true);

	connector.DisconnectDB();
}

TEST(DBConnectorTest, ProcedureParameterTest)
{
	std::map<std::string, std::shared_ptr<IStoredProcedure>> testProcedureMap;
	std::map<std::string, std::vector<std::pair<ProcedureName, ProcedureTypeName>>> resultPropertyMap;

	PROCEDURE_TEST_LIST(testProcedureMap, resultPropertyMap);

	ODBCConnector connector;

	ASSERT_EQ(connector.ConnectDB(L"OptionFile/DBConnectFile.txt"), true);
	EXPECT_EQ(connector.InitDB(), true);

	std::vector<ProcedureName> notMatchedProcedureList;
	auto columnMatch = [](
		const std::vector<std::pair<ProcedureName, ProcedureTypeName>>& cppProperties
		, const std::vector<std::pair<ProcedureName, ProcedureTypeName>>& dbProperties) -> bool
	{
		if (cppProperties.size() != dbProperties.size())
		{
			return false;
		}

		for (size_t i = 0; i < cppProperties.size(); ++i)
		{
			// 컬럼의 타입만 검사
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
	
		std::vector<std::pair<ProcedureName, ProcedureTypeName>> cppProperties;
		testProcedure.second->GetTypeInfo().GetAllProperties(cppProperties);

		auto matchedProcedureInfo = connector.GetProcedureInfo(testProcedure.first);
		ASSERT_NE(matchedProcedureInfo, nullptr);
		auto matchedProcedureResultColumnInfo = resultPropertyMap.find(testProcedure.first);
		ASSERT_NE(matchedProcedureResultColumnInfo, resultPropertyMap.end());

		std::vector<std::pair<ProcedureName, ProcedureTypeName>> dbInputProperties;
		std::vector<std::pair<ProcedureName, ProcedureTypeName>> dbResultProperties;
		char UTF8_name[256], UTF8_dataTypeName[256];
		for (const auto& inputColmun : matchedProcedureInfo->inputColumnInfoList)
		{
			UTF16ToUTF8(inputColmun.name.c_str(), UTF8_name);
			UTF16ToUTF8(inputColmun.dataTypeName.c_str(), UTF8_dataTypeName);
			dbInputProperties.emplace_back(std::make_pair(UTF8_name, UTF8_dataTypeName));
		}
		for (const auto& resultColmun : matchedProcedureInfo->resultColumnInfoList)
		{
			UTF16ToUTF8(resultColmun.name.c_str(), UTF8_name);
			UTF16ToUTF8(resultColmun.dataTypeName.c_str(), UTF8_dataTypeName);
			dbResultProperties.emplace_back(std::make_pair(UTF8_name, UTF8_dataTypeName));
		}

		bool isMatched = true;
		isMatched = columnMatch(cppProperties, dbInputProperties) && isMatched;
		isMatched = columnMatch(matchedProcedureResultColumnInfo->second, dbResultProperties) && isMatched;
		if (isMatched == false)
		{
			notMatchedProcedureList.emplace_back(testProcedure.first);
		}
		EXPECT_TRUE(isMatched);
	}

	if (notMatchedProcedureList.size() > 0u)
	{
		std::cout << std::endl;
		std::cout << "---------------------------------------" << std::endl;
		for (const auto& notMatchedProcedure : notMatchedProcedureList)
		{
			std::cout << "Procedure was not matched. ProcedureName[" << notMatchedProcedure << "]" << std::endl;
		}
		std::cout << "---------------------------------------" << std::endl << std::endl << std::endl;
	}
	connector.DisconnectDB();
}
#endif