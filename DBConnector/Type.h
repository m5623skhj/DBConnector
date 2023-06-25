#pragma once
#include <string>

class FString
{
public:
    FString() = default;

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

private:
    std::string thisString = "";
};

class FWString
{
public:
    FWString() = default;

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

private:
    std::wstring thisString = L"";
};

using ProcedureName = std::string;
using ProcedureTypeName = std::string;
