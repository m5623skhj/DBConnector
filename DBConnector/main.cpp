#include <direct.h>

#include "PreCompile.h"
#include "BuildConfig.h"
#include <iostream>
#include <Windows.h>
#include <sql.h>
#include "Path.h"
#include "DBServer.h"

#include "ODBCConnector.h"
#include "StoredProcedure.h"

#include "GoogleTest.h"

using namespace std;

bool TryDBMigration()
{
    char cwd[512];
    if (_getcwd(cwd, sizeof(cwd)) == nullptr)
    {
        return false;
    }

    const string migratorPath = cwd + MIGRATOR_PATH;

    if (const int migratorResult = system(migratorPath.c_str()); migratorResult != 0)
    {
        return false;
    }

    return true;
}

int main() 
{
    if (TryDBMigration() == false)
    {
        cout << "---------------------" << '\n';
        cout << "Migration failed" << '\n';
        cout << "---------------------" << '\n' << '\n' << '\n';
        return 0;
    }
    cout << "---------------------" << '\n';
    cout << "Migration successes" << '\n';
    cout << "---------------------" << '\n' << '\n' << '\n';

#if UNIT_TEST
    if (GTestHelper::StartTest() == false)
    {
        cout << "---------------------" << '\n';
        cout << "GTest failed" << '\n';
        cout << "---------------------" << '\n' << '\n' << '\n';
        return 0;
    }
    cout << "---------------------" << '\n';
    cout << "GTest successes" << '\n';
    cout << "---------------------" << '\n' << '\n' << '\n';
#endif

    ODBCConnector& connector = ODBCConnector::GetInst();
    do
    {
        if (connector.ConnectDB(L"OptionFile/DBConnectFile.txt") == false)
        {
            cout << "---------------------" << '\n';
            cout << "ConnectDB() failed" << '\n';
            cout << "---------------------" << '\n' << '\n' << '\n';
            break;
        }

        if (connector.InitDB() == false)
        {
            cout << "---------------------" << '\n';
            cout << "InitDB() failed" << '\n';
            cout << "---------------------" << '\n' << '\n' << '\n';
            break;
        }

        cout << "---------------------" << '\n';
        cout << "InitDB() Success" << '\n';
        cout << "---------------------" << '\n' << '\n' << '\n';
    } while (false);

    DBServer dbServer(L"OptionFile/DBServerOptionFile.txt");

    while (true)
    {
        Sleep(1000);

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            dbServer.StopServer();
            break;
        }
    }

    connector.DisconnectDB();

    return 0;
}
