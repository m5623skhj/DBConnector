#include "PreCompile.h"

#include "GoogleTest.h"
#include "googletest-main/googletest/include/gtest/gtest.h"

#include "ODBCConnector.h"

namespace GTestHelper
{
	bool StartTest()
	{
		testing::InitGoogleTest();
		return RUN_ALL_TESTS() == 0;
	}
}

TEST(DBConnectorTest, ProcedureTest)
{
	ODBCConnector connector;

	ASSERT_EQ(connector.ConnectDB(L"OptionFile/DBConnectFile.txt"), true);
	EXPECT_EQ(connector.InitDB(), true);

	connector.DisconnectDB();
}