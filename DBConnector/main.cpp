#include "PreCompile.h"
#include "BuildConfg.h"
#include <iostream>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>
#include "Path.h"

#include "ODBCConnector.h"
#include "StoredProcedure.h"

#include "GoogleTest.h"

using namespace std;

bool TryDBMigration()
{
    char cwd[512];
    if (_getcwd(cwd, sizeof(cwd)) == NULL)
    {
        return false;
    }

    std::string migratorPath = cwd + MIGRATOR_PATH;

    int migratorResult = system(migratorPath.c_str());
    if (migratorResult != 0)
    {
        return false;
    }

    return true;
}

int main() 
{
#if UNIT_TEST
    if (GTestHelper::StartTest() == false)
    {
        cout << "GTest failed" << std::endl;
        return 0;
    }
    cout << "---------------------" << endl;
    cout << "GTest successed" << endl;
    cout << "---------------------" << endl << endl << endl;
#endif

    if (TryDBMigration() == false)
    {
        cout << "---------------------" << endl;
        cout << "Migration failed" << endl;
        cout << "---------------------" << endl << endl << endl;
        return 0;
    }
    cout << "---------------------" << endl;
    cout << "Migration successed" << endl;
    cout << "---------------------" << endl << endl << endl;

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
