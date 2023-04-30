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
