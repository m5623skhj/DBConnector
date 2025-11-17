#pragma once
#include <string>
#include "ODBCConst.h"

class FString
{
public:
    FString()
    {
        thisString.reserve(SQL_STRING_LENGTH);
    }

    explicit FString(const char* inString)
    {
        thisString.assign(inString);
    }

    explicit FString(const std::string& inString)
    {
        thisString.assign(inString);
    }

    void operator=(const std::string& inString)
    {
        thisString.assign(inString);
    }

    FString& operator+=(const std::string& inString)
    {
        thisString += inString;
        return *this;
    }

    bool operator==(const std::string& inCompareString) const
    {
        return thisString == inCompareString;
    }

	[[nodiscard]]
	const char* GetCString() const
    {
        return thisString.c_str();
    }

    friend std::ostream& operator<<(std::ostream& stream, const FString& string)
    {
        stream << string.GetCString();
        return stream;
    }

    [[nodiscard]]
	const std::string& GetOriginString() const
    {
        return thisString;
    }

private:
    std::string thisString;
};

class FWString
{
public:
    FWString()
    {
        thisString.reserve(SQL_STRING_LENGTH);
    }

    explicit FWString(const WCHAR* inString)
    {
        thisString.assign(inString);
    }

    explicit FWString(const std::wstring& inString)
    {
        thisString.assign(inString);
    }

    void operator=(const WCHAR* inString)
    {
        thisString.assign(inString);
    }

    void operator=(const std::wstring& inString)
    {
        thisString.assign(inString);
    }

    FWString& operator+=(const std::wstring& inString)
    {
        thisString += inString;
        return *this;
    }

    bool operator==(const std::wstring& inCompareString) const
    {
        return thisString == inCompareString;
    }

    [[nodiscard]]
    const WCHAR* GetCString() const
    {
        return thisString.c_str();
    }

    friend std::wostream& operator<<(std::wostream& stream, const FWString& string)
    {
        stream << string.GetCString();
        return stream;
    }

    [[nodiscard]]
    const std::wstring& GetOriginString() const
    {
        return thisString;
    }

    void Resize(const unsigned short size)
    {
		thisString.resize(size);
    }

private:
    std::wstring thisString;
};

using ProcedureName = std::string;
using ProcedureTypeName = std::string;
