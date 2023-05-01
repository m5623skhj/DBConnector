#include "PreCompile.h"

#include "GoogleTest.h"
#include "ODBCConnector.h"
#include "StoredProcedure.h"
#include "ODBCMetaData.h"
#include "ODBCUtil.h"
#include <utility>

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
	PROCEDURE_TEST_LIST();

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
			if (cppProperties[i].first != dbProperties[i].first)
			{
				return false;
			}
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

		std::vector<std::pair<ProcedureName, ProcedureTypeName>> dbProperties;
		for (const auto& inputColmun : matchedProcedureInfo->inputColumnInfoList)
		{
			char UTF8_inputName[256], UTF8_inputDataTypeName[256];
			UTF16ToUTF8(inputColmun.name.c_str(), UTF8_inputName);
			UTF16ToUTF8(inputColmun.dataTypeName.c_str(), UTF8_inputDataTypeName);
			dbProperties.emplace_back(std::make_pair(UTF8_inputName, UTF8_inputDataTypeName));
		}
		/*/
		// 아직 cpp에 result column 작업이 안돼서 주석처리
		for (const auto& resultColmun : matchedProcedureInfo->resultColumnInfoList)
		{
			dbProperties.emplace_back(std::make_pair(resultColmun.name, resultColmun.dataTypeName));
		}
		/*/

		bool isMatched = columnMatch(cppProperties, dbProperties);
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
			std::cout << "Procedure was not matched : " << notMatchedProcedure << std::endl;
		}
		std::cout << "---------------------------------------" << std::endl << std::endl << std::endl;
	}
	connector.DisconnectDB();
}
#endif