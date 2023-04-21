#include <iostream>
#include "DBConnector.h"

// 외부에서 다른 타입으로 캐스팅하지 말고, 내부에서 타입까지 정해주면 좋겠음

int main()
{
    DBConnector connect;

    connect.ConnectDB();
    connect.DBSendQuery("SELECT * FROM testtbl");

    auto row = connect.GetRow();
    do
    {
        if (row["id"] == nullptr || row["tablename"] == nullptr || row["no"] == nullptr)
        {
            return 0;
        }

        std::cout << row["id"] << " " << row["tablename"] << " " << row["no"] << std::endl;
        row.NextRow();
    } while (row.IsEndRow() == false);

    connect.FreeResult();
    connect.CloseDB();

    return 0;
}
