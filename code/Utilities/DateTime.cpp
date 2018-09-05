// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"
#include <Windows.h>
#include <sstream>
#include <ostream>
#include <iomanip>
#include "DateTime.h"
#include "DMException.h"

using namespace std;

namespace Microsoft { namespace Windows { namespace Azure { namespace DeviceManagement { namespace Utils {

// ToDo: Provide Multi-byte versions.
wstring DateTime::GetCurrentDateTimeString()
{
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);

    return GetDateTimeString(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
}

wstring DateTime::GetDateTimeString(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute, unsigned int second)
{
    basic_ostringstream<wchar_t> formattedTime;
    formattedTime << setw(2) << setfill(L'0') << year
        << L'-' << setw(2) << setfill(L'0') << month
        << L'-' << setw(2) << setfill(L'0') << day
        << L'T' << setw(2) << setfill(L'0') << hour
        << L':' << setw(2) << setfill(L'0') << minute
        << L':' << setw(2) << setfill(L'0') << second;
    return formattedTime.str();
}

bool DateTime::ISO8601DateTimeFromString(const wstring& dateTimeString, ISO8601DateTime& dateTime)
{
    // Iso 8601 partial spec
    // http://www.w3.org/TR/NOTE-datetime

    // ToDo: review and support more formats.
    // ToDo: support milliseconds.
    memset(&dateTime, 0, sizeof(dateTime));

    // YYYY-MM-DDTHH:MM:SS[Z]
    // YYYY-MM-DDTHH:MM:SS[(-|+)hh:mm]
    vector<wstring> tokens;
    Utils::SplitString(dateTimeString, L'T', tokens);
    if (tokens.size() != 2)
    {
        wprintf(L"Warning: invalid system date/time format: %s", dateTimeString.c_str());
        return false;
    }

    vector<wstring> dateComponents;
    Utils::SplitString(tokens[0], L'-', dateComponents);
    if (dateComponents.size() != 3)
    {
        wprintf(L"Warning: invalid date format: %s", dateTimeString.c_str());
        return false;
    }
    dateTime.year = static_cast<WORD>(stoi(dateComponents[0]));
    dateTime.month = static_cast<WORD>(stoi(dateComponents[1]));
    dateTime.day = static_cast<WORD>(stoi(dateComponents[2]));

    wstring timeString = tokens[1];
    wstring zoneString;
    wchar_t zoneChar = '\0';
    vector<wstring> timeComponents;
    Utils::SplitString(timeString, L'-', timeComponents);
    if (timeComponents.size() == 2)
    {
        // 2016-10-10T09:00:01-008:00
        timeString = timeComponents[0];
        zoneChar = L'-';
        zoneString = timeComponents[1];
    }
    else
    {
        timeComponents.clear();
        Utils::SplitString(timeString, L'+', timeComponents);
        if (timeComponents.size() == 2)
        {
            // 2016-10-10T09:00:01+008:00
            timeString = timeComponents[0];
            zoneChar = L'+';
            zoneString = timeComponents[1];
        }
        else
        {
            if (timeString.size() > 0 &&
                (timeString[timeString.size() - 1] == L'Z' || timeString[timeString.size() - 1] == L'z'))
            {
                timeString = timeString.substr(0, timeString.size() - 1);
                zoneChar = L'Z';
            }
        }
        Utils::SplitString(tokens[1], L'-', timeComponents);
    }

    timeComponents.clear();
    Utils::SplitString(timeString, L':', timeComponents);
    if (timeComponents.size() != 3)
    {
        wprintf(L"Warning: invalid time format: %s", dateTimeString.c_str());
        return false;
    }
    dateTime.hour = static_cast<WORD>(stoi(timeComponents[0]));
    dateTime.minute = static_cast<WORD>(stoi(timeComponents[1]));
    dateTime.second = static_cast<WORD>(stoi(timeComponents[2]));
    dateTime.milliseconds = 0;

    timeComponents.clear();
    if (zoneChar == L'Z' || zoneChar == '\0')
    {
        dateTime.zoneHour = 0;
        dateTime.zoneMinute = 0;
    }
    else
    {
        Utils::SplitString(zoneString, L':', timeComponents);
        if (timeComponents.size() != 2)
        {
            wprintf(L"Warning: invalid time zone format: %s", dateTimeString.c_str());
            return false;
        }
        dateTime.zoneHour = static_cast<short>(stoi(timeComponents[0])) * (zoneChar == L'-' ? -1 : 1);
        dateTime.zoneMinute = static_cast<WORD>(stoi(timeComponents[1]));
    }

    return true;
}

void DateTime::ToUTC(const ISO8601DateTime& in, ISO8601DateTime& out)
{
    if (in.zoneHour == 0 && in.zoneMinute == 0)
    {
        memcpy(&out, &in, sizeof(out));
        return;
    }

    tm tsBase = { 0 };
    tsBase.tm_year = in.year - 1900;
    tsBase.tm_mon = in.month - 1; // 0 based
    tsBase.tm_mday = in.day;
    tsBase.tm_hour = in.hour;
    tsBase.tm_min = in.minute;
    tsBase.tm_sec = in.second;
    tsBase.tm_isdst = 0;

    time_t tBase = _mkgmtime32(&tsBase);

    time_t tDelta = 0;
    if (in.zoneHour > 0)
        tDelta = -1 * (abs(in.zoneHour) * 60 * 60 + in.zoneMinute * 60);
    else
        tDelta = (abs(in.zoneHour) * 60 * 60 + in.zoneMinute * 60);

    time_t tUtc = tBase + tDelta;

    tm result;
    gmtime_s(&result, &tUtc);

    out.year = static_cast<unsigned short>(result.tm_year + 1900);
    out.month = static_cast<unsigned short>(result.tm_mon + 1); // 0 based
    out.day = static_cast<unsigned short>(result.tm_mday);
    out.hour = static_cast<unsigned short>(result.tm_hour);
    out.minute = static_cast<unsigned short>(result.tm_min);
    out.second = static_cast<unsigned short>(result.tm_sec);
    out.milliseconds = 0;
    out.zoneHour = 0;
    out.zoneMinute = 0;
}

wstring DateTime::StringFromISO8601DateTime(const ISO8601DateTime& sourceDateTime, bool utc)
{
    ISO8601DateTime dateTime;
    if (utc)
    {
        ToUTC(sourceDateTime, dateTime);
    }
    else
    {
        dateTime = sourceDateTime;
    }

    basic_ostringstream<wchar_t> formattedTime;
    formattedTime
        << setw(4) << setfill(L'0') << dateTime.year
        << L"-" << setw(2) << setfill(L'0') << dateTime.month
        << L"-" << setw(2) << setfill(L'0') << dateTime.day
        << L"T"
        << setw(2) << setfill(L'0') << dateTime.hour
        << L':' << setw(2) << setfill(L'0') << dateTime.minute
        << L':' << setw(2) << setfill(L'0') << dateTime.second;
    // ToDo: support milliseconds.

    if (dateTime.zoneHour == 0)
    {
        formattedTime << L"Z";
    }
    else
    {
        if (dateTime.zoneHour > 0)
        {
            formattedTime << L"+";
        }
        else
        {
            formattedTime << L"-";
        }
        formattedTime
            << setw(2) << setfill(L'0') << abs(dateTime.zoneHour)
            << L':' << setw(2) << setfill(L'0') << dateTime.zoneMinute;
    }

    return formattedTime.str();
}

bool DateTime::SystemTimeFromISO8601(const wstring& dateTimeString, SYSTEMTIME& dateTime)
{
    ISO8601DateTime iso8601DateTime;
    if (!ISO8601DateTimeFromString(dateTimeString, iso8601DateTime))
    {
        return false;
    }

    dateTime.wYear = iso8601DateTime.year;
    dateTime.wMonth = iso8601DateTime.month;
    dateTime.wDay = iso8601DateTime.day;

    // ToDo: need to adjust the hour/minute to accommodate the time zone.
    dateTime.wHour = iso8601DateTime.hour;
    dateTime.wMinute = iso8601DateTime.minute;
    dateTime.wSecond = iso8601DateTime.second;
    dateTime.wMilliseconds = iso8601DateTime.milliseconds;
    dateTime.wDayOfWeek = 0;

    return true;
}

wstring DateTime::ISO8601FromSystemTime(const SYSTEMTIME& dateTime)
{
    // Iso 8601 partial spec
    // http://www.w3.org/TR/NOTE-datetime

    // ToDo: review and support more formats.

    basic_ostringstream<wchar_t> formattedTime;
    formattedTime
        << setw(4) << setfill(L'0') << dateTime.wYear
        << L"-" << setw(2) << setfill(L'0') << dateTime.wMonth
        << L"-" << setw(2) << setfill(L'0') << dateTime.wDay
        << L"T"
        << setw(2) << setfill(L'0') << dateTime.wHour
        << L':' << setw(2) << setfill(L'0') << dateTime.wMinute
        << L':' << setw(2) << setfill(L'0') << dateTime.wSecond
        << L'Z';

    return formattedTime.str();
}

wstring DateTime::CanonicalizeDateTime(const wstring& dateTimeString, bool utc)
{
    ISO8601DateTime dateTime;
    if (!ISO8601DateTimeFromString(dateTimeString, dateTime))
    {
        // ToDo: better message and error code
        throw DMException(-1, "Error: failed to parse date time! ");
    }
    return StringFromISO8601DateTime(dateTime, utc);
}

}}}}}