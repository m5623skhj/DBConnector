#include "PreCompile.h"

#include "GoogleTest.h"
#include "ODBCConnector.h"

TEST(DBConnectorTest, ProcedureTest)
{
	ODBCConnector connector;

	ASSERT_EQ(connector.ConnectDB(L"OptionFile/DBConnectFile.txt"), true);
	EXPECT_EQ(connector.InitDB(), true);

	connector.DisconnectDB();
}
