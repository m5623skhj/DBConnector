#include "PreCompile.h"
#include <iostream>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>

#include "ODBCConnector.h"

using namespace std;

int main() {

    ODBCConnector connector;

    connector.ConnectDB(L"OptionFile/DBConnectFile.txt");
    connector.InitDB();

    if (connector.DBSendQuery(L"SELECT * FROM testtbl") == false)
    {
        connector.DisconnectDB();
        return 0;
    }

    connector.DisconnectDB();

    /*
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret;
    
    auto errorCheck = [](const SQLRETURN& error)
    {
        if (error != SQL_SUCCESS)
        {
            cout << "Error : " << error;
        }
    };

    // Allocate an environment handle
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

    // Set the ODBC version to use
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    // Allocate a connection handle
    SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

    // Connect to the data source
    auto error = SQLDriverConnect(dbc, NULL, (SQLWCHAR*)L"DSN=testDSN;UID=root;PWD=1234;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    errorCheck(error);

    // Allocate a statement handle
    error = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    errorCheck(error);

    // Prepare the SQL statement
    error = SQLPrepare(stmt, (SQLWCHAR*)L"SELECT * FROM testtbl", SQL_NTS);
    errorCheck(error);

    // Execute the statement
    error = SQLExecute(stmt);
    errorCheck(error); 

    // Bind the result columns
    //SQLCHAR col1[256];
    //int item[10];
    int item;
    SQLLEN col1len;
    error = SQLBindCol(stmt, 1, SQL_INTEGER, &item, sizeof(item), &col1len);
    errorCheck(error);

    SQLCHAR col2[256];
    SQLLEN col2len;
    error = SQLBindCol(stmt, 2, SQL_C_CHAR, col2, sizeof(col2), &col2len);
    errorCheck(error);

    // Fetch and print the results
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        cout << "col1: " << item << ", col2: " << col2 << std::endl;
    }

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    */

    return 0;
}
