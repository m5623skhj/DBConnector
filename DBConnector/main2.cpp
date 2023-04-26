#include "PreCompile.h"
#include <iostream>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>

#include "ODBCConnector.h"
#include "StoredProcedure.h"

#include "GoogleTest.h"

using namespace std;

int main() {

    if (GTestHelper::StartTest() == false)
    {
        cout << "GTest failed" << std::endl;
        return 0;
    }

    // test code
    //*/

    test testProcedure;
    auto super = testProcedure.GetTypeInfo().GetSuper();
    if (super != nullptr)
    {
        cout << "has super" << endl;
    }

    //*/

    ODBCConnector connector;

    connector.ConnectDB(L"OptionFile/DBConnectFile.txt");
    connector.InitDB();



    if (connector.DBSendQuery(L"SELECT * FROM testtbl") == false)
    {
        connector.DisconnectDB();
        return 0;
    }

    connector.DisconnectDB();

    return 0;
}
