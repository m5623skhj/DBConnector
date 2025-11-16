#pragma once
#include <string>
#include "ODBCConst.h"

class FString
{
public:
    FString()
    {
        thisString.reserve(SQL_STRING_LENGTH);
    };

    FString(const char* inString)
    {
        thisString.assign(inString);
    }

    FString(std::string inString)
    {
        thisString.assign(inString);
    }

    void operator=(std::string inString)
    {
        thisString.assign(inString);
    }

    FString& operator+=(std::string inString)
    {
        thisString += inString;
        return *this;
    }

    bool operator==(const std::string& inCompareString) const
    {
        return thisString == inCompareString;
    }

    const char* GetCString() const
    {
        return thisString.c_str();
    }

    friend std::ostream& operator<<(std::ostream& stream, const FString& string)
    {
        stream << string.GetCString();
        return stream;
    }

    const std::string& GetOriginString() const
    {
        return thisString;
    }

private:
    std::string thisString = "";
};

class FWString
{
public:
    FWString()
    {
        thisString.reserve(SQL_STRING_LENGTH);
    };

    FWString(const WCHAR* inString)
    {
        thisString.assign(inString);
    }

    FWString(std::wstring inString)
    {
        thisString.assign(inString);
    }

    void operator=(const WCHAR* inString)
    {
        thisString.assign(inString);
    }

    void operator=(std::wstring inString)
    {
        thisString.assign(inString);
    }

    FWString& operator+=(std::wstring inString)
    {
        thisString += inString;
        return *this;
    }

    bool operator==(const std::wstring& inCompareString) const
    {
        return thisString == inCompareString;
    }

    const WCHAR* GetCString() const
    {
        return thisString.c_str();
    }

    friend std::wostream& operator<<(std::wostream& stream, const FWString& string)
    {
        stream << string.GetCString();
        return stream;
    }

    const std::wstring& GetOriginString() const
    {
        return thisString;
    }

    void Resize(const unsigned short size)
    {
		thisString.resize(size);
    }

private:
    std::wstring thisString = L"";
};

using ProcedureName = std::string;
using ProcedureTypeName = std::string;
