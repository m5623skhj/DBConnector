#include "PreCompile.h"
#include "BuildConfg.h"
#include <iostream>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>
#include "Path.h"
#include "DBServer.h"

#include "ODBCConnector.h"
#include "StoredProcedure.h"
#include "ODBCConst.h"

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

    ODBCConnector& connector = ODBCConnector::GetInst();
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

    if (connector.CallStoredProcedure("update_test", 1) == false)
    {
        return 0;
    }

    auto procedure = connector.GetProcedureInfo("test");
    procedure->SettingSPMaker(connector.GetStmtHandle(), SP_PARAMETER_LOCATION, 1, 2, 33, "testString");

    if (connector.DBSendQuery(procedure->sql) == false)
    {
        return 0;
    }
    
    DBServer dbServer(L"DBServerOptionFile.txt");

    connector.DisconnectDB();

    return 0;
}
