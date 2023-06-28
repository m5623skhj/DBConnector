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

    string migratorPath = cwd + MIGRATOR_PATH;

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
        cout << "---------------------" << endl;
        cout << "GTest failed" << endl;
        cout << "---------------------" << endl << endl << endl;
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
            cout << "---------------------" << endl;
            cout << "ConnectDB() failed" << endl;
            cout << "---------------------" << endl << endl << endl;
            break;
        }

        if (connector.InitDB() == false)
        {
            cout << "---------------------" << endl;
            cout << "InitDB() failed" << endl;
            cout << "---------------------" << endl << endl << endl;
            break;
        }

        cout << "---------------------" << endl;
        cout << "InitDB() Success" << endl;
        cout << "---------------------" << endl << endl << endl;
    } while (false);

    auto conn = connector.GetConnection();
    if (conn == nullopt)
    {
        return 0;
    }

    auto procedure = connector.GetProcedureInfo("test");
    test testProcedure;
    testProcedure.id3 = 100;
    testProcedure.teststring = L"ohShit";
    connector.CallSPDirectWithSPObject(conn.value().stmtHandle, procedure, testProcedure);

    auto procedure2 = connector.GetProcedureInfo("string_test_proc");
    wstring testString2 = L"ttttteeeee";
    if (connector.CallSPDirect(conn.value().stmtHandle, procedure2, testString2) == false)
    {
        return 0;
    }

    auto selectTest2 = connector.GetProcedureInfo("SELECT_TEST_2");
    SELECT_TEST_2 selectProcedure2;
    selectProcedure2.id = 3;
    if (connector.CallSPDirectWithSPObject(conn.value().stmtHandle, selectTest2, selectProcedure2) == false)
    {
        return 0;
    }

    auto selectResult = connector.GetSPResult<SELECT_TEST_2::ResultType>(conn.value().stmtHandle);
    if (selectResult == nullopt)
    {
        return 0;
    }

    for (const auto& result : selectResult.value())
    {
        wcout << L"no : " << result.no << L", tablename : " << result.tablename << endl;
    }
    
    auto selectTest3 = connector.GetProcedureInfo("SELECT_TEST_3");
    SELECT_TEST_3 selectProcedure3;
    if (connector.CallSPDirectWithSPObject(conn.value().stmtHandle, selectTest3, selectProcedure3) == false)
    {
        return 0;
    }

    auto selectResult2 = connector.GetSPResult<SELECT_TEST_3::ResultType>(conn.value().stmtHandle);
    if (selectResult2 == nullopt)
    {
        return 0;
    }

    for (const auto& result : selectResult2.value())
    {
        wcout << L"no : " << result.no << L", tablename : " << result.tablename << endl;
    }

    //DBServer dbServer(L"DBServerOptionFile.txt");

    connector.DisconnectDB();

    return 0;
}
