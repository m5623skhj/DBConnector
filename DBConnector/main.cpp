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

    auto conn = connector.GetConnection();
    if (conn == std::nullopt)
    {
        return 0;
    }

    if (connector.CallStoredProcedureDirect("update_test", conn.value().stmtHandle, 1) == false)
    {
        return 0;
    }

    auto procedure = connector.GetProcedureInfo("test");
    procedure->SettingSPMaker(conn.value().stmtHandle, SP_PARAMETER_LOCATION, 1, 2, 33, "testString");

    if (ODBCUtil::DBSendQueryDirect(procedure->sql, conn.value().stmtHandle) == false)
    {
        return 0;
    }
    
    DBServer dbServer(L"DBServerOptionFile.txt");

    connector.DisconnectDB();

    return 0;
}
