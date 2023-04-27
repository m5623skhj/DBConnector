#include "PreCompile.h"
#include <iostream>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>

#include "ODBCConnector.h"
#include "StoredProcedure.h"

#include "GoogleTest.h"

using namespace std;

struct Base 
{
    DEFINE_CLASS_INFO(Base)
};

struct Derived : Base
{
private: 
    friend SuperClassTypeDeduction; 
    friend TypeInfoInitializer; 

public: 
    using Super = typename SuperClassTypeDeduction<Derived>::Type; 
    using ThisType = Derived; 
    
    static TypeInfo& StaticTypeInfo() 
    {
        static TypeInfo typeInfo{ TypeInfoInitializer<ThisType>("Derived") }; 
        return typeInfo;
    } 
    
    virtual const TypeInfo& GetTypeInfo() const 
    {
        return typeInfo;
    }

private: 
    inline static TypeInfo& typeInfo = StaticTypeInfo();

private:
};

int main() 
{
    // test code
    //*/

    std::cout << std::boolalpha;
    std::cout << HasSuper<Base> << std::endl;    // false
    std::cout << HasSuper<Derived> << std::endl; // true
    //std::cout << HasSuper<int> << std::endl;     // false

    Derived d;
    auto super1 = d.GetTypeInfo().GetSuper();
    if (super1 != nullptr)
    {
        std::cout << super1->GetName() << std::endl;
    }

    test testProcedure;
    auto super = testProcedure.GetTypeInfo().GetSuper();
    if (super != nullptr)
    {
        cout << "has super" << endl;
    }

    //*/

    if (GTestHelper::StartTest() == false)
    {
        cout << "GTest failed" << std::endl;
        return 0;
    }

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
