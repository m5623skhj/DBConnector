#include "PreCompile.h"
#include "BuildConfg.h"
#include <iostream>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>

#include "ODBCConnector.h"
#include "StoredProcedure.h"

#include "GoogleTest.h"

using namespace std;

int main() 
{
#if UNIT_TEST
    if (GTestHelper::StartTest() == false)
    {
        cout << "GTest failed" << std::endl;
        return 0;
    }
    cout << "-------------" << endl;
    cout << "GTest Success" << endl;
    cout << "-------------" << endl << endl << endl;
#endif

    ODBCConnector connector;
    do
    {
        if (connector.ConnectDB(L"OptionFile/DBConnectFile.txt") == false)
        {
            cout << "ConnectDB() failed" << endl;
            break;
        }

        if (connector.InitDB() == false)
        {
            cout << "InitDB() failed" << endl;
            break;
        }

        cout << "InitDB() Success" << endl;
    } while (false);

    connector.DisconnectDB();

    return 0;
}
